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

#include "RdspVslAppUtilities.h"
#include <string.h>
#include <stdlib.h>

 /*
  * VoiceSpot Utilities
  */

void rdsp_import_voicespot_model(const char* Afilename, uint8_t** Amodel, uint32_t* Amodel_size) {
	//Open model file
	if (Afilename == NULL) {
		printf("Error: a model file name must be provided\n");
		return;
	}

	FILE* file = fopen(Afilename, "rb");
	if (file == NULL) {
		printf("Error: cannot find model file %s\n", Afilename);
		return;
	}

	//Get file length
	unsigned long fileLen;
	fseek(file, 0, SEEK_END);
	fileLen = ftell(file);
	fseek(file, 0, SEEK_SET);

	//Allocate memory
	*Amodel = (uint8_t*)rdsp_malloc(fileLen + 1 + 16);
	if (!*Amodel) {
		fprintf(stderr, "Memory error!");
		fclose(file);
		return;
	}

	//Read file contents into buffer
	Amodel[0] += ((uintptr_t) * (Amodel)) % 16;
	size_t rt = fread(*(Amodel), fileLen, 1, file);
	fclose(file);

	*Amodel_size = fileLen;
}

int32_t rdsp_set_voicespot_params(rdsp_voicespot_control* Avoicespot_control, int32_t Avoicespot_handle, const char* Avoicespot_params) {
	// Set up parameters using a parameter blob
	if (Avoicespot_params != NULL) {
		FILE* fileptr = fopen(Avoicespot_params, "rb"); // Open the file in binary mode
		if (fileptr == NULL) {
			printf("Parameter file not be found: %s\n", Avoicespot_params);
			return -1;
		}
		fseek(fileptr, 0, SEEK_END); // Jump to the end of the file
		uint32_t param_blob_size = ftell(fileptr); // Get the current byte offset in the file
		rewind(fileptr); // Jump back to the beginning of the file
		uint8_t* param_blob = (uint8_t*)rdsp_malloc(param_blob_size);
		size_t ret = fread(param_blob, param_blob_size, 1, fileptr); // Read in the entire file
		fclose(fileptr); // Close the file

		int32_t voicespot_status = rdspVoiceSpot_SetParametersFromBlob(Avoicespot_control, Avoicespot_handle, param_blob);
		printf("rdspVoiceSpot_SetParametersFromBlob: voicespot_status = %d\n", (int)voicespot_status);
		rdsp_free(param_blob);
		return voicespot_status;
	}
	return -1;
}

/*
 * Performance log file
 */

int32_t rdsp_write_performance_file_header(RETUNE_VOICESEEKERLIGHT_plugin_t* APluginInit, FILE* Afid) {
	if (Afid) {
		// Create header
		// Add information concerning the setup
		fprintf(Afid, "libVoiceSeekerLight v%d.%d.%d\n", APluginInit->version.major, APluginInit->version.minor, APluginInit->version.patch);
		fprintf(Afid, "samplerate,num_mics,num_spks,framesize_in,framesize_out\n");
		fprintf(Afid, "%.0f,%d,%d,%d,%d\n\n",
			APluginInit->constants.samplerate,
			APluginInit->config.num_mics,
			APluginInit->config.num_spks,
			APluginInit->constants.framesize_in,
			APluginInit->config.framesize_out);

		// Add captions for trigger info
		fprintf(Afid, "trigger,trigger_sample,start_sample,stop_sample,score,threshold\n");
	}
	return 0;
}

int32_t rdsp_write_performance_log(int32_t Atrigger, int32_t Atrigger_sample, int32_t Astart_sample, int32_t Astop_sample, int32_t Ascore, FILE* Afid) {
	if (Afid) {
		// Log trigger
		fprintf(Afid, "%i,%i,%i,%i,%i\n", Atrigger, Atrigger_sample, Astart_sample, Astop_sample, Ascore);
	}
	return 0;
}

void csv_to_array(int32_t* Aarray, FILE* Afid) {
	if (Afid != NULL) {
		int32_t idx = 0;
		char line[1024];
		const char delim[2] = ",";
		while (fgets(line, sizeof(line), Afid)) {
			char* tok = strtok(line, delim);
			while (tok != NULL) {
				Aarray[idx++] = atoi(tok);
				tok = strtok(NULL, delim);
			}
		}
	}
}
