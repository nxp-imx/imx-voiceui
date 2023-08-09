/*
 * Copyright (c) 2021 by Retune DSP. This code is the confidential and
 * proprietary property of Retune DSP and the possession or use of this
 * file requires a written license from Retune DSP.
 *
 * Contact information: info@retune-dsp.com
 *                      www.retune-dsp.com
 */

#ifndef __SignalProcessor_VoiceSeekerLight_h__
#define __SignalProcessor_VoiceSeekerLight_h__

#include "SignalProcessorImplementation.h"

#include <iostream>
#include <libVoiceSeekerLight.h>
#include <string>
#include <alsa/asoundlib.h>
#include <vector>
#include <exception>
#include <mqueue.h>

#include <RdspAppUtilities.h>
#include <RdspWavfile.h>
#include <RdspCycleCounter.h>
#include <AFEConfigState.h>

#define MAXSTR 1023

#define	RDSP_VOICESEEKER_LIGHT_APP_VERSION_MAJOR	0 // Major version the app
#define	RDSP_VOICESEEKER_LIGHT_APP_VERSION_MINOR	2 // Minor version the app
#define	RDSP_VOICESEEKER_LIGHT_APP_VERSION_PATCH	8 // Patch version the app

#define RDSP_ENABLE_VAD 0
#define RDSP_ENABLE_AEC 1
#define RDSP_ENABLE_VOICESPOT 1
#define RDSP_ENABLE_PAYLOAD_BUFFER 1
#define RDSP_ENABLE_AGC 0
#define RDSP_BUFFER_LENGTH_SEC 1.5f
#define RDSP_AEC_FILTER_LENGTH_MS 150 // TODO: can AEC filter length be reduced?
#define RDSP_DISABLE_TRIGGER_TIMEOUT_SEC 1

#define VOICESEEKER_OUT_NHOP 200

#define MINUTE_INTERVAL_WAV_FILE 3		//Interval of minutes for saving audio files for delay analysis (Saves files every 3 minutes)

#define CHECK(x) \
        do { \
                if (!(x)) { \
                        fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
                        perror(#x); \
                        exit(-1); \
                } \
        } while (0) \

namespace SignalProcessor {
	typedef enum {
		MACHINE_UNKNOWN = -1,
		MACHINE_IMX8M,
		MACHINE_IMX93EVK11,
		MACHINE_IMX93QSB,
	} MachineInfo;

	class SignalProcessor_VoiceSeekerLight : public SignalProcessor::SignalProcessorImplementation {

		RETUNE_VOICESEEKERLIGHT_plugin_t vsl;
		rdsp_voiceseekerlight_config_t vsl_config;
		rdsp_voiceseekerlight_constants_t vsl_constants;

		enum class VoiceSeekerLightSignalProcessorState {
			closed,
			opened,
			filtering
		};

		//Define structure for circular buffer
		typedef struct {
			char* samples;
			int32_t head, tail;
			size_t size, num_entries;
		} queue;

		//Signal processor customization, we "need" these for the implementation
		static const std::string _jsonConfigDescription; //JSON configuration
		VoiceSeekerLightSignalProcessorState _state; //State identifier
		size_t _sampleSize; //Size of samples in bytes. Parameter derived out of _sampleFormat variable
		int32_t _sampleRate; //Selected sample rate.
		snd_pcm_format_t _sampleFormat = SND_PCM_FORMAT_S32_LE; //Selected sample format. The number should correspond to ALSA formats
		int32_t _periodSize; //Selected period size
		int32_t _inputChannelsCount; //Selected input channels count
		int32_t _referenceChannelsCount; //Selected reference channels count
		int32_t _channel2output; //Input channel selected as output. Must be in range of _inputChannelsCount indexed from 0 to (_inputChannelsCount - 1)
		bool _WWDetection;
		int32_t framesize_in;									//Read only variable from vsl_config.framesize_in
		int32_t framesize_out;

		//These variables are needed if resampling is enabled
		int32_t framesize_in_mic;
		int32_t framesize_in_ref;

		uint32_t heap_size;
		void* heap_memory;
		uint32_t scratch_size;
		void* scratch_memory;

		int32_t sizeBuffDelay; //Number of bytes should be changed carefully depending on delay samples
		int32_t delaySamples;  //Delay in number of samples
		queue circularBuffDelay; //Circular buffer for delay
		bool debugEnable;

		float** ref_in; //Input reference buffer
		float** mic_in; //Input mic buffer

		int32_t iteration;
		int32_t disable_trigger_frame_counter;

		//Create audio files for delay debuggin
		rdsp_wav_file_t fid_mic_delay;
		rdsp_wav_file_t fid_ref_delay;
		rdsp_wav_file_t fid_mic_out;
		bool fid_delay_files_open;
		int32_t num_delay_files;
		// Specify mic geometry
		AFEConfig::mic_xyz mic[4];
		MachineInfo machine_info;

		void setDefaultSettings(); //Sets the signal processors settings to default values.

		//Functions for delay buffer
		void initQueue(queue* q, size_t samples_delay, size_t maxSize);
		void queueDestroy(queue* q);
		void enqueue(queue* q, const char* samples_ref, size_t sizeBuff);
		void dequeue(queue* q, char* samples, size_t sizeBuff);

		int32_t sendBufferToWakeWordEngine(void* buffer, int32_t length, int32_t iteration, int32_t enable_triggering);
		int32_t getKeyWordOffsetFromWakeWordEngine();

	public:
		//Construtor, initializes internal resources
		SignalProcessor_VoiceSeekerLight();

		//This set of functions come from the base class and we need to implement them all
		int32_t openProcessor(const std::unordered_map<std::string, std::string>* settings = nullptr);
		int32_t closeProcessor();
		int32_t processSignal(const char* nChannelMicBuffer, size_t micBufferSize,
			const char* nChannelRefBuffer, size_t refBufferSize,
			char* cleanMicBuffer, size_t cleanMicBufferSize);

		//Returns the complete configuration space in JSON fromat
		const std::string& getJsonConfigurations() const override;

		//Set of functions returning the basic features supported by the signal processor.
		int32_t getSampleRate() const override;
		int32_t getPeriodSize() const override;
		int32_t getInputChannelsCount() const override;
		int32_t getReferenceChannelsCount() const override;
		const char* getSampleFormat() const override;

		uint32_t getVersionNumber() const override;
		MachineInfo getMachineInfo();
	};

}
#endif
