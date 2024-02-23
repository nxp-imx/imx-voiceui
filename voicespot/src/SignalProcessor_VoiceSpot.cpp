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

#include "SignalProcessor_VoiceSpot.h"
#include "SignalProcessor_NotifyTrigger.h"
#include "AFEConfigState.h"

namespace SignalProcessor {

	//Constructor
	SignalProcessor_VoiceSpot::SignalProcessor_VoiceSpot()
		: voicespot_status{ 0 }, voicespot_control{ nullptr }, data_type{ RDSP_DATA_TYPE__FLOAT32 }, voicespot_handle{ 0 },
		enable_highpass_filter{ 1 }, generate_output{ 0 }, model_blob{ nullptr }, model_blob_size{ 0 }, adapt_threshold_mode{ 3 },
		voicespot_version{ 0 }, voicespot_model_string{ nullptr }, voicespot_class_string{ nullptr }, num_samples_per_frame{ 0 },
		num_outputs{ 0 }, last_notification{ 0 }, framecount_in{ 0 }, framecount_out{ 0 }, vad_timeout_frames{ 0 },
		disable_trigger_frame_counter{ 0 },	num_triggers{ 0 }, voiceseeker_mcps_count{ 0 }, voiceseeker_mcps{ 0.0 }{

		AFEConfig::AFEConfigState configState;
		std::string voicespot_model = configState.isConfigurationEnable("VoiceSpotModel", "HeyNXP_en-US_1.bin");
		std::string voicespot_params = configState.isConfigurationEnable("VoiceSpotParams", "HeyNXP_1_params.bin");
		std::string voicespot_model_path = "/unit_tests/nxp-afe/" + voicespot_model;
		std::string voicespot_params_path = "/unit_tests/nxp-afe/" + voicespot_params;
		//initialize the queue attributes
		attr.mq_flags = 0;
		attr.mq_maxmsg = 10;
		attr.mq_msgsize = VSLOUTBUFFERSIZE;
		attr.mq_curmsgs = 0;

		//create the message queue
		mq_vslout = mq_open("/voicespot_vslout", O_CREAT | O_RDONLY, 0644, &attr);
		attr.mq_msgsize = sizeof(int32_t);
		mq_iter = mq_open("/voiceseeker_iterations", O_CREAT | O_RDONLY, 0644, &attr);
		mq_trigg = mq_open("/voiceseeker_trigger", O_CREAT | O_RDONLY, 0644, &attr);
		mq_offset = mq_open("/voicespot_offset", O_CREAT | O_WRONLY, 0644, &attr);

		//Create VoiceSpot control structure
#if defined (CortexA55)
        RDSP_DeviceId_en device_id = Device_IMX9_CA55;
#elif defined (CortexA53)
        RDSP_DeviceId_en device_id = Device_IMX8M_CA53;
#else
        RDSP_DeviceId_en device_id = Device_IMX8M_CA53;
#endif
		voicespot_status = rdspVoiceSpot_CreateControl(&voicespot_control, data_type, device_id);
		printf("rdspVoiceSpot_CreateControl: voicespot_status = %d\n", (int32_t)voicespot_status);

		//Create VoiceSpot instance
		voicespot_status = rdspVoiceSpot_CreateInstance(voicespot_control, &voicespot_handle, enable_highpass_filter, generate_output);
		printf("rdspVoiceSpot_CreateInstance: voicespot_status = %d\n", (int32_t)voicespot_status);

		//Load VoiceSpot keyword model
		rdsp_import_voicespot_model(voicespot_model_path.c_str(), &model_blob, &model_blob_size);
		printf("VoiceSpot model: %s\r\n", voicespot_model.c_str());

		//Check the integrity of the model
		if (rdspVoiceSpot_CheckModelIntegrity(model_blob_size, model_blob) != RDSP_VOICESPOT_OK) {
			printf("rdspVoiceSpot_CheckModelIntegrity: Model integrity check failed\n");
			return;
		}

		//Open the VoiceSpot instance
		voicespot_status = rdspVoiceSpot_OpenInstance(voicespot_control, voicespot_handle, model_blob_size, model_blob, 0, 0);
		printf("rdspVoiceSpot_OpenInstance: voicespot_status = %d\n", (int32_t)voicespot_status);

		//Enable use of the Adaptive Threshold mechanism
		voicespot_status = rdspVoiceSpot_EnableAdaptiveThreshold(voicespot_control, voicespot_handle, adapt_threshold_mode);
		printf("rdspVoiceSpot_EnableAdaptiveThreshold: voicespot_status = %d\n", voicespot_status);

		//Set VoiceSpot parameters
		voicespot_status = rdsp_set_voicespot_params(voicespot_control, voicespot_handle, voicespot_params_path.c_str());

		//Retrieve VoiceSpot configuration
		rdspVoiceSpot_GetLibVersion(voicespot_control, &voicespot_version);
		printf("VoiceSpot library version: %d.%d.%d.%u\n", voicespot_version.major, voicespot_version.minor, voicespot_version.patch, voicespot_version.build);
		rdspVoiceSpot_GetModelInfo(voicespot_control, voicespot_handle, &voicespot_version, &voicespot_model_string, &voicespot_class_string, &num_samples_per_frame, &num_outputs);
		printf("VoiceSpot model version: %d.%d.%d\n\n", voicespot_version.major, voicespot_version.minor, voicespot_version.patch);
	}

	int32_t SignalProcessor_VoiceSpot::voiceSpot_process(void* vsl_out, bool notify, int32_t iteration, int32_t enable_triggering) {

		/* event_thresholds is an array of manually set minimum thresholds for a trigger event per class.
		 * NULL means automatic, i.e., no manually set minimum thresholds.*/
		int32_t* event_thresholds = NULL;

		 //VoiceSpot Process
		int32_t num_scores = 0;
		int32_t scores[4]{ 0 };
		int32_t** sfb_output = NULL;
		int32_t voicespot_status = rdspVoiceSpot_Process(voicespot_control, voicespot_handle, RDSP_PROCESSING_LEVEL__FULL, (uint8_t*)vsl_out, &num_scores, scores, (uint8_t**)sfb_output);

		if (voicespot_status != RDSP_VOICESPOT_OK) {
			printf("rdspVoiceSpot_Process: voicespot_status = %d\n", (int32_t)voicespot_status);
			return -1;
		}

		bool notified = false;
		int32_t framesize_out = VOICESEEKER_OUT_NHOP;

		//Check for trigger
		int32_t score_index_trigger = rdspVoiceSpot_CheckIfTriggered(voicespot_control, voicespot_handle, scores, enable_triggering, event_thresholds, RDSP_PROCESSING_LEVEL__FULL);

		if (score_index_trigger >= 0) {
			num_triggers++;
			int32_t keyword_start_offset_samples = -1;
			int32_t keyword_stop_offset_samples = 0;
			int32_t timing_accuracy = 4; // Accuracy of the timing estimate, in frames
			voicespot_status = rdspVoiceSpot_EstimateStartAndStop(voicespot_control, voicespot_handle, score_index_trigger, -1, timing_accuracy, &keyword_start_offset_samples, &keyword_stop_offset_samples); // Comment out this line if timing estimation is not needed

			if (voicespot_status != RDSP_VOICESPOT_OK)
				printf("rdspVoiceSpot_EstimateStartAndStop: voicespot_status = %d\n", (int32_t)voicespot_status);

			//Log trigger
			int32_t trigger_sample = framecount_out * framesize_out;
			int32_t start_sample = trigger_sample - keyword_start_offset_samples;
			int32_t stop_sample = trigger_sample - keyword_stop_offset_samples;
			printf("trigger = %i, trigger_sample = %i, start_sample = %i, stop_sample = %i, score = %i\n", num_triggers, trigger_sample, start_sample, stop_sample, scores[0]);
			printf("keyword_start_offset_samples = %i\n", keyword_start_offset_samples);
			printf("ITER = %d\n", iteration);

			//Inform VoiceSeekerLight upon a trigger event
			if (notify)
				SignalProcessor_notifyTrigger(notified, "WakeWordNotify", iteration, last_notification);

			return  keyword_start_offset_samples;
		}
		return  0;
	}

	mqd_t SignalProcessor_VoiceSpot::get_mqVslout() {
		return mq_vslout;
	}

	mqd_t SignalProcessor_VoiceSpot::get_mqIter() {
		return mq_iter;
	}

	mqd_t SignalProcessor_VoiceSpot::get_mqTrigg() {
		return mq_trigg;
	}

	mqd_t SignalProcessor_VoiceSpot::get_mqOffset() {
		return mq_offset;
	}
}
