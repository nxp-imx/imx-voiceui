/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#include <AudioStreamBase.h>
#include <alsa/asoundlib.h>
#include <string>
#include <memory>

#ifndef AUDIO_STREAM_GUARD_
#define AUDIO_STREAM_GUARD_

namespace AudioStreamWrapper
{

/**
 * Available stream directions
 * The stream can be an input (capture) stream, which acquires samples
 * from microphone input or output (playback) stream, which feeds the
 * samples into sound speakers.\n
 * In case of virtual devices like a loopback device, this can be a bit
 * confusing. However, to simulate a virtual input, the loopback stream
 * should be opened as output - writing (playing back) into the loopback
 * we achieve samples on loopback to be read.
 */
enum class StreamDirection
{
    /** Output (playback) stream */
    eOutput  = 0,
    /** Input (caputure) stream */
    eInput   = 1
};

/**
 * Available stream types
 * The stream type represents, whether the samples are arranged in a frame
 * as interleaved or noninterleaved and whether the acquired samples
 * are accessed directly or indirectly.
 * 
 * @remark Direct access in ring buffer is not suppored at the moment. This
 * means, that the samples are copied out from the ring buffer into users
 * buffer.
 */
enum class StreamType
{
    /** Interleaved samples ordering, indirect access */
    eInterleaved = 3,
    /** Non-interleaved samples ordering, indirect access */
    eNonInterleaved = 4
};

struct streamSettings
{
    /**
     * Stream name as ALSA understands them (hw:1,0, or names on asound.conf)
     */
    std::string streamName;
    /**
     * Sample formats
     * 
     * @remark Maybe we should abstract away any references to ALSA, however
     * there are plenty of formats. Need to decide, whether it would make sense
     * to define just some of them or redefine the whole set?
     */
    snd_pcm_format_t format;
    /**
     * Defines the sample ordering in stream (non-/interleaved)
     */
    StreamType accessType;
    /**
     * Defines whether the stream is an input (capture) or output (playback)
     */
    StreamDirection direction;
    /**
     * Number of stream channels (1 - mono, 2 - stereo, 3...)
     */
    int channels;
    /**
     * Sample rate in Hz
     */
    int rate;
    /**
     * Total number of frames to hold
     * This parameter should be an int multiple of periodSizeFrames
     */
    int bufferSizeFrames;
    /**
     * Minimal number of frames to acquire before the user can use them
     */
    int periodSizeFrames;
};

#define FAILURE -1
#define SUCCESS 0
#define INSUFFICIENT_FRAMES -1

class AudioStream : public AudioStreamBase
{
public:
    AudioStream(void);
    ~AudioStream(void);
    void open(const struct streamSettings & settings);
    void start(void) override;
    void stop(bool force) override;
    void close(void) override;
    int recover(int err);
    int readFrames(void * buffer, size_t size);
    int writeFrames(const void * buffer, size_t size);

    void printConfig(void);

protected:
    snd_pcm_t * _handle;
    snd_pcm_hw_params_t * _hwParams;
    snd_pcm_sw_params_t * _swParams;

    /* Stream configuration */
    std::string _streamName;
    snd_pcm_stream_t _streamType;
    snd_pcm_format_t _format;
    snd_pcm_access_t _accessType;
    int _channels;
    int _rate;
    int _bufferSizeFrames;
    int _periodSizeFrames;

    void setHwParams(void);
    void setSwParams(void);
};

} // namespace AudioStreamWrapper

#endif /* AUDIO_STREAM_GUARD_ */