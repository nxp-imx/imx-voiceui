/*
 * Copyright (c) 2020 by Retune DSP. This code is the confidential and
 * proprietary property of Retune DSP and the possession or use of this
 * file requires a written license from Retune DSP.
 *
 * Contact information: info@retune-dsp.com
 *                      www.retune-dsp.com
 */

#ifndef RDSP_VOICESEEKER_APP_UTILITIES_H
#define RDSP_VOICESEEKER_APP_UTILITIES_H

#include <stdint.h>


	typedef struct RETUNE_VOICESEEKER_plugin_s RETUNE_VOICESEEKER_plugin_t;

	int read_ccount_ext();

	/*
	 * Conversion
	 */
	float int_to_float_ext(int32_t Ax);
	float float_to_float_ext(float Ax);
	int32_t float_to_int32_ext(float Ax);
	int16_t float_to_int16_ext(float Ax);

	/*
	 * Math
	 */
#ifndef M_PI
#define M_PI 3.14159265358979323846f   // pi
#endif

	void rdsp_pcm_to_float(const char* tmp_buf_int, float** Abuffer, uint32_t Anum_samples, int32_t num_channels, int32_t sample_size);

	void rdsp_float_to_pcm(char* tmp_buf_int, float** Abuffer, uint32_t Anum_samples, int32_t num_channels, int32_t sample_size);

#endif /* RDSP_VOICESEEKER_APP_UTILITIES_H */
