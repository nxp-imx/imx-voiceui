/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2022 NXP
 */

#ifndef __SignalProcessor_VIT_h__
#define __SignalProcessor_VIT_h__

#include <iostream>

#include "PL_platformTypes_CortexA.h"
#include "VIT.h"
#include "VIT_Model_en.h"

#define MODEL_LOCATION              VIT_MODEL_IN_ROM
#define DEVICE_ID                   VIT_IMX8MA53
#define VIT_OPERATING_MODE          VIT_VOICECMD_ENABLE	// Enabling Voice Commands only
#define VIT_COMMAND_TIME_SPAN 		3.0 // in second
#define NUMBER_OF_CHANNELS          _1CHAN
#define MEMORY_ALIGNMENT            8     // in bytes

namespace SignalProcessor {

	class SignalProcessor_VIT {

	private:
		bool VoiceSpotEnable;
		bool VITWakeWordEnable;
	public:
		//Constructor
		SignalProcessor_VIT();
		VIT_Handle_t VIT_Handle;
		VIT_Handle_t VIT_open_model();
		void VIT_close_model(VIT_Handle_t VITHandle);
		bool VIT_Process_Phase(VIT_Handle_t VITHandle, int16_t* frame_data, int16_t* pCmdId);
		bool isVoiceSpotEnable();
		bool isVITWakeWordEnable();
	};

}
#endif

