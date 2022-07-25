/*
Copyright 2021 by Retune DSP
Copyright 2022 NXP

NXP Confidential. This software is owned or controlled by NXP and may only be used strictly in accordance with the applicable license terms.  By expressly accepting such terms or by downloading, installing, activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef RDSP_VOICESEEKERLIGHT_PLUGIN_TYPES_H_
#define RDSP_VOICESEEKERLIGHT_PLUGIN_TYPES_H_

#include <stdint.h>

#if defined(FUSIONDSP)
#include <xtensa/tie/xt_fusion.h>
#endif
#if defined(HIFI3)
#include <xtensa/tie/xt_hifi3.h>
#endif
#if defined(HIFI4)
#include <xtensa/tie/xt_hifi4.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(HIFI3) || defined(HIFI4) || defined(FUSIONDSP)
	typedef xtfloat rdsp_float;
	typedef xtfloatx2 rdsp_floatx2;
	typedef xtfloatx2 rdsp_complex;
#else
	typedef float rdsp_float;
	typedef float rdsp_floatx2[2];
	typedef float rdsp_complex[2];
#endif

	typedef rdsp_float rdsp_coordinate_xyz_t[3];

#ifdef __cplusplus
}
#endif

#endif // RDSP_VOICESEEKERLIGHT_PLUGIN_TYPES_H_
