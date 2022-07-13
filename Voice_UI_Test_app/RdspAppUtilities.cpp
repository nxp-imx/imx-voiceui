/*
 * Copyright (c) 2020 by Retune DSP. This code is the confidential and
 * proprietary property of Retune DSP and the possession or use of this
 * file requires a written license from Retune DSP.
 *
 * Contact information: info@retune-dsp.com
 *                      www.retune-dsp.com
 */

#include "RdspAppUtilities.h"

#include <cstring>
#include <errno.h>
#include <stdexcept>

int read_ccount_ext() {
	unsigned int ccount = 0;
	return ccount;
}

float int_to_float_ext(int32_t Ax)
{
	return (float)Ax;
}

float float_to_float_ext(float Ax)
{
	return (float)Ax;
}

int32_t float_to_int32_ext(float Ax)
{
	return (int32_t)Ax;
}

int16_t float_to_int16_ext(float Ax)
{
	return (int16_t)Ax;
}


void rdsp_float_to_pcm(char* tmp_buf_int, float** Abuffer, uint32_t Anum_samples, int32_t num_channels, int32_t sample_size) {
	if (sample_size == 2) {
		const float scale = float_to_float_ext(32768.0f); // buffer is 32 bit floating-point
		int16_t* ptr = (int16_t*)tmp_buf_int;
		for (uint32_t isample = 0; isample < Anum_samples; isample++) {
			for (uint16_t ich = 0; ich < num_channels; ich++, ptr++) {
				*ptr = float_to_int16_ext(Abuffer[ich][isample] * scale);
			}
		}
	}
	else if (sample_size == 4) {
		const float scale = float_to_float_ext(32768.0f * 65536.0f);
		int32_t* ptr = (int32_t*)tmp_buf_int;
		for (uint32_t isample = 0; isample < Anum_samples; isample++) {
			for (uint16_t ich = 0; ich < num_channels; ich++, ptr++) {
				*ptr = float_to_int32_ext(Abuffer[ich][isample] * scale);
			}
		}

	} else {
		printf(" NO 16/32 bits on rdsp_float_to_pcm\n");
	}
}

void rdsp_pcm_to_float(const char* tmp_buf_int, float** Abuffer, uint32_t Anum_samples, int32_t num_channels, int32_t sample_size) {

	// 16 bit integer wav file
	if (sample_size == 2) {
		const float scale = float_to_float_ext(3.0517578125e-05f); // Convert to float
		for (int32_t ich = 0; ich <num_channels; ich++) {
			int16_t* ptr = (int16_t*)tmp_buf_int + ich;
			for (uint32_t isample = 0; isample < Anum_samples; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = int_to_float_ext(*ptr) * scale;
			}
		}
	}
	else if (sample_size == 4) {
		const float scale = float_to_float_ext(4.656612873077393e-10f);
		for (int32_t ich = 0; ich <num_channels; ich++) {
			int32_t* ptr = (int32_t*)tmp_buf_int + ich;
			for (uint32_t isample = 0; isample < Anum_samples; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = int_to_float_ext(*ptr) * scale;
			}
		}
	} else {
		printf(" NO 16/32 bits on rdsp_pcm_to_float\n");
	}
}


