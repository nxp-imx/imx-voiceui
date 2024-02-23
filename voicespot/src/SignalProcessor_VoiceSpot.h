/*
 * Copyright (c) 2021 by Retune DSP.
 * Copyright 2022-2024 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP
 * and may only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.
 * If you do not agree to be bound by the applicable license terms,
 * then you may not retain, install, activate or otherwise use the software.
 */

#include <public/rdsp_voicespot.h>
#include <public/rdsp_voicespot_utils.h>
#include <RdspVslAppUtilities.h>
#include <RdspCycleCounter.h>
#include <iostream>
#include <mqueue.h>

#ifndef __SignalProcessor_VoiceSpot_h__
#define __SignalProcessor_VoiceSpot_h__

#define VOICESEEKER_OUT_NHOP 200
#define VSLOUTBUFFERSIZE (VOICESEEKER_OUT_NHOP * sizeof(float))

#define CHECK(x) \
        do { \
                if (!(x)) { \
                        fprintf(stderr, "%s:%d: ", __func__, __LINE__); \
                        perror(#x); \
                        exit(-1); \
                } \
        } while (0) \

namespace SignalProcessor {

	class SignalProcessor_VoiceSpot {

		int32_t voicespot_status;
		rdsp_voicespot_control* voicespot_control;	// Pointer to VoiceSpot control struct
		int32_t data_type;							// Input is float, floating-point mode computations
		int32_t voicespot_handle;					// VoiceSpot handle
		int32_t enable_highpass_filter;
		int32_t generate_output;
		uint8_t* model_blob;
		uint32_t model_blob_size;

		/*
		 * ADAPTIVE THRESHOLDS MODES:
		 * 0: Fixed threshold (uses the array event_thresholds as thresholds)
		 * 1: Adaptive threshold
		 * 2: Adaptive sensitivity
		 * 3: Adaptive threshold + adaptive sensitivity
		 */
		int32_t adapt_threshold_mode;				//Option 3 is selected

		//VoiceSpot Configuration
		rdsp_voicespot_version voicespot_version;
		char* voicespot_model_string;
		char** voicespot_class_string;
		int32_t num_samples_per_frame;
		int32_t num_outputs;

		int32_t last_notification;
		int32_t framecount_in;
		int32_t framecount_out;
		int32_t vad_timeout_frames;
		int32_t disable_trigger_frame_counter;
		int32_t num_triggers;
		int32_t voiceseeker_mcps_count;

		float voiceseeker_mcps;

		struct mq_attr attr;
		mqd_t mq_vslout;
		mqd_t mq_iter;
		mqd_t mq_trigg;
		mqd_t mq_offset;

	public:
		//Constructor
		SignalProcessor_VoiceSpot();

		int32_t voiceSpot_process(void* vsl_out, bool notify, int32_t iteration, int32_t enable_triggering);
		mqd_t get_mqVslout();
		mqd_t get_mqIter();
		mqd_t get_mqTrigg();
		mqd_t get_mqOffset();
	};

}
#endif

