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

#ifndef RDSP_VOICESEEKERLIGHT_APP_UTILITIES_H
#define RDSP_VOICESEEKERLIGHT_APP_UTILITIES_H

#include "libVoiceSeekerLight.h"
#include "public/rdsp_voicespot.h"
#include <stdio.h>


#ifdef __cplusplus
extern "C" {
#endif

	/*
	 * VoiceSpot utilities
	 */

	void rdsp_import_voicespot_model(const char* Afilename, uint8_t** Amodel, uint32_t* Amodel_size);
	int32_t rdsp_set_voicespot_params(rdsp_voicespot_control* Avoicespot_control, int32_t Avoicespot_handle, const char* Avoicespot_params);

	/*
	 * Performance log file
	 */

	int32_t rdsp_write_performance_file_header(RETUNE_VOICESEEKERLIGHT_plugin_t* APluginInit, FILE* Afid);
	int32_t rdsp_write_performance_log(int32_t Atrigger, int32_t Atrigger_sample, int32_t Astart_sample, int32_t Astop_sample, int32_t Ascore, FILE* Afid);

	/*
	 * App utilities
	 */

	void csv_to_array(int32_t* Aarray, FILE* Afid);

#ifdef __cplusplus
}
#endif

#endif // RDSP_VOICESEEKERLIGHT_APP_UTILITIES_H
