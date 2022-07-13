// SPDX-License-Identifier: BSD-3-Clause
#include "AudioStream.h"
#include "AudioStreamException.h"
#include <sys/stat.h>
#include <iostream>
#include <alsa/asoundlib.h>

namespace AudioStreamWrapper
{
    /**
     * @addtogroup pcm_dir PCM direction macros
     * @{
     */
    #define PCM_VALUE_LESS      ((int) -1)
    #define PCM_VALUE_EQUAL     ((int) 0)
    #define PCM_VALUE_GREATER   ((int) 1)
    /** @} */

    #define BITS_PER_BYTE       8u

    AudioStream::AudioStream(void) : _handle(nullptr), _hwParams(nullptr), _swParams(nullptr)
    {
    }

    AudioStream::~AudioStream(void)
    {
        close();
    }

    void
    AudioStream::open(const struct streamSettings & settings)
    {
        int err = FAILURE;

        if (0u != (settings.bufferSizeFrames % settings.periodSizeFrames))
        {
            throw AudioStreamException("Buffer size not an int multiple of period size", settings.streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if (nullptr != this->_handle)
        {
            throw AudioStreamException("Stream already opened", settings.streamName.c_str(), __FILE__, __LINE__, -1);
        }

        this->_streamName       = settings.streamName;
        this->_streamType       = (settings.direction == StreamDirection::eInput) ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK;
        this->_accessType       = static_cast<snd_pcm_access_t>(settings.accessType);
        this->_channels         = settings.channels;
        this->_format           = settings.format;
        this->_rate             = settings.rate;
        this->_bufferSizeFrames = settings.bufferSizeFrames;
        this->_periodSizeFrames = settings.periodSizeFrames;

        err = snd_pcm_hw_params_malloc(&(this->_hwParams));
        if (err < 0) throw AudioStreamException("Allocating memory for HW params failed", this->_streamName.c_str(), __FILE__, __LINE__, -1);

        err = snd_pcm_sw_params_malloc(&(this->_swParams));

        err = snd_pcm_open(&_handle, _streamName.c_str(), this->_streamType, SND_PCM_NONBLOCK);
        if (err < 0) throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);

        setHwParams();

        setSwParams();
    }

    void
    AudioStream::setHwParams(void)
    {
        snd_pcm_uframes_t val;
        int err = 0;
        /* Get the stream configuration space
        This configuration space contains all possible configurations
        for sample rate, channels, buffer sizes etc.
        Once a user sets a given parameter of some value, the rest
        of parameters gets narrowed down. So at certain moment the
        API may result in an error reporting unsupported configuration.*/
        if ((err = snd_pcm_hw_params_any(_handle, _hwParams)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_hw_params_set_access(_handle, _hwParams, this->_accessType)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_hw_params_set_format(_handle, _hwParams, this->_format)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_hw_params_set_channels(_handle, _hwParams, this->_channels)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_hw_params_set_rate(_handle, _hwParams, this->_rate, PCM_VALUE_EQUAL)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_hw_params_set_period_size(_handle, _hwParams, this->_periodSizeFrames, PCM_VALUE_EQUAL)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        val = this->_bufferSizeFrames;
        if ((err = snd_pcm_hw_params_set_buffer_size_near(_handle, _hwParams, &val)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }
        this->_bufferSizeFrames = val;

        if ((err = snd_pcm_hw_params(_handle, _hwParams)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }
    }

    void
    AudioStream::setSwParams(void)
    {
        /* TODO if needed, we can define this function and introduce new parameters. For now, we don't
        configure SW parameters. */
        int err;
        if ((err = snd_pcm_sw_params_current(_handle, _swParams)) < 0)
        {
            printf("[AudioStream]: Unable to get stream software parameters; %s\n", snd_strerror(err));
            close();
            exit(EXIT_FAILURE);
        }
        /* playback pcm will start automatically if samples in ring buffer is >= start threshold */
        snd_pcm_uframes_t start_threshold = this->_streamType == SND_PCM_STREAM_PLAYBACK? _bufferSizeFrames: 1;
        if ((err = snd_pcm_sw_params_set_start_threshold(_handle, _swParams, start_threshold)) < 0)
        {
            printf("[AudioStream]: Unable to set start treshold; %s\n", snd_strerror(err));
            close();
            exit(EXIT_FAILURE);
        }
        if ((err = snd_pcm_sw_params_set_stop_threshold(_handle, _swParams, _bufferSizeFrames)) < 0)
        {
            printf("[AudioStream]: Unable to set stop treshold; %s\n", snd_strerror(err));
            close();
            exit(EXIT_FAILURE);
        }
        if ((err = snd_pcm_sw_params_set_avail_min(_handle, _swParams, _periodSizeFrames)) < 0)
        {
            printf("[AudioStream]: Unable to set avail min; %s\n", snd_strerror(err));
            close();
            exit(EXIT_FAILURE);
        }
        if((err = snd_pcm_sw_params(_handle, _swParams)) < 0)
        {
            printf("[AudioStream]: Unable to set SW parameters; %s\n", snd_strerror(err));
            close();
            exit(EXIT_FAILURE);
        }
    }

    void
    AudioStream::start(void)
    {
        int err;
        if (nullptr == this->_handle)
        {
            throw AudioStreamException("Stream not opened", this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }

        if ((err = snd_pcm_start(this->_handle)) < 0)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }
    }

    void
    AudioStream::stop(bool force)
    {
        int err = 0;
        snd_pcm_state_t state = snd_pcm_state(this->_handle);

        if ((true == force) || (SND_PCM_STATE_SUSPENDED == state))
        {
            err = snd_pcm_drop(this->_handle);
        }
        else
        {
            err = snd_pcm_drain(this->_handle);
        }

        if (0 > err)
        {
            throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
        }
    }

    void
    AudioStream::close(void)
    {
        /* From ALSA github source code it appears, that the snd_pcm_close() function drops the stream,
        frees allocated memory of hw params, closes the stream and frees the pcm memory. */
        int err;
        if (nullptr != this->_handle)
        {
            err = snd_pcm_close(this->_handle);
            if (0 > err)
            {
                throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
            }

            this->_handle = nullptr;
            this->_hwParams = nullptr;
        }

        if (nullptr != this->_swParams)
        {
            snd_pcm_sw_params_free(this->_swParams);
            this->_swParams = nullptr;
        }
    }

    int
    AudioStream::recover(int err)
    {
        return snd_pcm_recover(this->_handle, err, 1);
    }

    int
    AudioStream::readFrames(void * buffer, size_t byte_count)
    {
        /* Make sure the user provided buffer of appropriate size by computing the required bytes count */
        size_t expected_bytes = static_cast<size_t>(snd_pcm_format_size(this->_format, this->_channels * this->_periodSizeFrames));

        /* We compare the required byte count with provided buffer size */
        if (expected_bytes != byte_count) throw AudioStreamException("Buffer for storing captured samples is to small", this->_streamName.c_str(), __FILE__, __LINE__, -1);

        /* Check that the stream is input, otherwise we can't read out of it */
        /* TODO it appears, that writing to a capture stream and reading out of playback stream is allowed and doesn't return any errors...
        maybe we can skip this check... needs more investigation and make a decision, how to handle writing/reading into/from capture/playback stream. */
        //if (SND_PCM_STREAM_CAPTURE != this->_streamType) throw AudioStreamException("Invalid use of readFrames(), stream opened as output/playback!", this->_streamName.c_str(), __FILE__, __LINE__, -1);

        uint8_t buffer_offset = 0;
        size_t frames_count = this->_periodSizeFrames;
        size_t result = 0;
        while (frames_count > 0) {
            int err = snd_pcm_readi(this->_handle, (uint8_t *)buffer + buffer_offset, frames_count);
            if (err == -EAGAIN || (err > 0 && (size_t)err < frames_count)) {
                snd_pcm_wait(this->_handle, 100);
            }
            else if (err < 0) {
                if (this->recover(err) < 0)
                    throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
            }

            if (err > 0) {
                frames_count -= err;
                buffer_offset += err * (byte_count / this->_periodSizeFrames);
                result += err;
            }
        }
        return result;
    }

    int
    AudioStream::writeFrames(const void * buffer, size_t byte_count)
    {
        /* Make sure the user provided buffer of appropriate size by computing the required bytes count */
        size_t expected_bytes = static_cast<size_t>(snd_pcm_format_size(this->_format, this->_channels * this->_periodSizeFrames));

        /* We compare the required byte count with provided buffer size */
        if (expected_bytes != byte_count) throw AudioStreamException("Buffer being played doesn't match configured period size", this->_streamName.c_str(), __FILE__, __LINE__, -1);

        /* Check that the stream is input, otherwise we can't read out of it */
        //if (SND_PCM_STREAM_PLAYBACK != this->_streamType) throw AudioStreamException("Invalid use of writeFrames(), stream opened as input/capture!", this->_streamName.c_str(), __FILE__, __LINE__, -1);

        void *data = const_cast<void *>(buffer);
        uint8_t buffer_offset = 0;
        size_t frames_count = this->_periodSizeFrames;
        size_t result = 0;
        while (frames_count > 0) {
            int err = snd_pcm_writei(this->_handle, (uint8_t *)data + buffer_offset, frames_count);
            if (err == -EAGAIN) {
                snd_pcm_wait(this->_handle, 100);
                continue;
            }
            if (err < 0) {
                if (this->recover(err) < 0)
                    throw AudioStreamException(snd_strerror(err), this->_streamName.c_str(), __FILE__, __LINE__, -1);
            }

            if (err > 0) {
                frames_count -= err;
                buffer_offset += err * (byte_count / this->_periodSizeFrames);
                result += err;
            }
        }
        return result;
    }

    void
    AudioStream::printConfig(void)
    {
        std::cout << this->_streamName << " configuration:" << std::endl;
        snd_pcm_format_t format;
        snd_pcm_hw_params_get_format(_hwParams, &format);
        std::cout << "Format is: " << snd_pcm_format_name(format) << std::endl;

        snd_pcm_access_t accessType;
        snd_pcm_hw_params_get_access(_hwParams, &accessType);
        std::cout << "Access type is: " << snd_pcm_access_name(accessType) << std::endl;

        unsigned int channels;
        snd_pcm_hw_params_get_channels(_hwParams, &channels);
        std::cout << "Channels count is: " << channels << std::endl;

        unsigned int rate;
        int dir;
        snd_pcm_hw_params_get_rate(_hwParams, &rate, &dir);
        std::cout << "Rate is: " << rate << "[Hz]\n";

        snd_pcm_uframes_t buffer_size;
        snd_pcm_hw_params_get_buffer_size(_hwParams, &buffer_size);
        std::cout << "Buffer size is: " << buffer_size << "frames\n";

        snd_pcm_uframes_t period_size;
        snd_pcm_hw_params_get_period_size(_hwParams, &period_size, &dir);
        std::cout << "Period size is: " << period_size << "frames\n";
        std::cout << std::endl;
    }

}   /* namespace AudioStream */
