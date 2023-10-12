/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#ifndef __SignalProcessor__SignalProcessorImplementation_h__
#define __SignalProcessor__SignalProcessorImplementation_h__

namespace SignalProcessor
{
    #define GET_MAJOR_VERSION(version)      (uint32_t)((0xFF000000u & version) >> 24u)
    #define GET_MINOR_VERSION(version)      (uint32_t)((0x00FF0000u & version) >> 16u)
    #define GET_PATCH_VERSION(version)      (uint32_t)((0x0000FF00u & version) >> 8u)

    class SignalProcessorImplementation
    {
        public:
            virtual
            ~SignalProcessorImplementation() {};
            /**
             * @brief This function should return the configured sample rate.
             * 
             * @retval Configured sample rate in Hz.
             * @retval -1 = any value is acceptable
             * 
             * @warning The function should be invoked after the signal processor has been
             * opened. Invoking the function prior signal processor opening returns -1.
             * 
             * @remark To be able to configure the audio HW in conjunction with a given signal processor implementation,
             * this function should be used so the SW can automatically configure the sample rate of given audio streams.
             */
            virtual int
            getSampleRate(void) const = 0;
            /**
             * @brief This function should return the configured sample format.
             * 
             * @retval Configured sample format.
             * @retval -1 = any value is acceptable
             * 
             * @remark The format numbering must correspond to ALSA format numbering.
             * 
             * \par To be able to configure the audio HW in conjunction with a given signal processor implementation,
             * this function should be used so the SW can automatically configure the sample format of given audio streams.
             */
            virtual const char *
            getSampleFormat(void) const = 0;
            /**
             * @brief This function should return configured buffer period.
             * 
             * @retval Configured buffer period.
             * @retval -1 = any value is acceptable
             * 
             * @warning The function should be invoked after the signal processor has been
             * opened. Invoking the function prior signal processor opening returns -1.
             * 
             * @remark To be able to configure the audio HW in conjunction with a given signal processor implementation,
             * this function should be used so the SW can automatically configure the buffer period of given audio streams.
             */
            virtual int
            getPeriodSize(void) const = 0;
            /**
             * @brief This function should return the count of configured input channels.
             *
             * @retval Configured input channels count.
             * @retval -1 = any value is acceptable
             *
             * @warning The function should be invoked after the signal processor has been
             * opened. Invoking the function prior signal processor opening returns -1.
             * 
             * @remark To be able to configure the audio HW in conjunction with a given signal processor implementation,
             * this function should be used so the SW can automatically configure the number of input channels of given stream.
             */
            virtual int
            getInputChannelsCount(void) const = 0;
            /**
             * @brief This function should return the count of configured reference channels.
             * 
             * @retval Configured reference channels count.
             * @retval -1 = any value is acceptable
             * 
             * @warning The function should be invoked after the signal processor has been
             * opened. Invoking the function prior signal processor opening returns -1.
             * 
             * @remark To be able to configure the audio HW in conjunction with a given signal processor implementation,
             * this function should be used so the SW can automatically configure the number of reference channels of given audio stream.
             */
            virtual int
            getReferenceChannelsCount(void) const = 0;
            /**
             * Returns the implementation version number
             * @details The version number should be stored in a 32 bit long variable.
             * Bits 0 - 7 are reserved
             * Bits 8 - 15 hold the patch version
             * Bits 16 - 23 hold the minor version
             * Bits 24 - 31 hold the major version
             */
            virtual uint32_t
            getVersionNumber(void) const = 0;
            /**
             * @details Responsibility of this function is to allocate all resources
             * required by the signal processor and initialize the signal processor
             * according to input settings.\n
             * The input settings may differ from vendor to vendor, thus an unordered map
             * is provided for flexibility.\n
             * Its also possible not to provide the settings. In such case default settings
             * are used as defined by the specialized implementing class.
             * 
             * @param[in] settings Signal processor configuration
             * 
             * @remark To have as generic as possible interface, we need to define the settings
             * as unorderd map. The configuration will be provided as a key:value pair. From
             * this perspective it may be unclear, what can be set by the user, thus it's
             * mandatory to implement the getSettingsDefinition function which should
             * print out the required key:value pairs.
             * 
             * @see setDefaultSettings, processSignal, closeProcessor, getJsonConfigurations
             */
            virtual int 
            openProcessor(const std::unordered_map<std::string, std::string> * settings = nullptr) = 0;
            /**
             *  @details This function should deallocate all resources initialized
             * during signal processor opening, revert settings back to default and close
             * the signal processor.\n
             * If there is nothing to be closed (processor has not been opened yet or so), 
             * silently assume closing was successful returning ok status.
             */
            virtual int
            closeProcessor(void) = 0;
            /**
             * @details This function should filter the mic buffer with the use of a reference
             * buffer (playback buffer) and provide the result in clean mic buffer. The
             * input signal format is dependent on the underlying signal processor implementation.\n
             * This function should return error in case the signal processor has not been
             * initialized yet.
             *
             * @param[in] nChannelMicBuffer Array of N-channels microphone signal. The sample format depends on the underlying implemnetation.
             * @param[in] micBufferSize Microphone array size in bytes
             * @param[in] nChannelRefBuffer Array of N-channels microphone signal. The sample format depends on the underlying implementation.
             * @param[in] refBufferSize Reference array size in bytes
             * @param[out] cleanMicBuffer Array of samples representing a filtered single microphone channel. The sample format depends on the underlying implementation.
             * @param[in] cleanMicBufferSize Microphone output array size in bytes.
             * 
             * @note The frame size of all buffers is the same. However, the total number of
             * samples (and thus bytes) will differ based on the number of channels per frame.
             * 
             * @warning Makre sure the number of microphone channels, reference channels and
             * clean microphone channels provided as arrays matches the signal processor configuration.
             * Otherwise the library may provide unexpected results or lead to segmentation fault.\n
             */
            virtual int 
            processSignal(
                const char* nChannelMicBuffer, size_t micBufferSize,
                const char* nChannelRefBuffer, size_t refBufferSize,
                char* cleanMicBuffer, size_t cleanMicBufferSize) = 0;

            /**
             * @brief Returns a string in JSON format describing the configurations and possible options.
             * @details The string should contain all required parameters with possible options. These
             * should be recognized by the openProcessor() function.
             * 
             * @see getSampleRate, getSampleFormat, getPeriodSize, getInputChannelsCount, getReferenceChannelsCount
             */
            virtual const std::string &
            getJsonConfigurations(void) const = 0;

        protected:
            /**
             *  @details A default settings should be set with this function.
             */
            virtual void setDefaultSettings(void) = 0;
    };
}
#endif
