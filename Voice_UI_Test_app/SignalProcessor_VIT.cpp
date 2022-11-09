/*
 * SPDX-License-Identifier: BSD-3-Clause
 * Copyright 2022 NXP
 */

#include <string.h>
#include "SignalProcessor_VIT.h"
#include "AFEConfigState.h"

namespace SignalProcessor {

	//Constructor
	SignalProcessor_VIT::SignalProcessor_VIT() {

		/* Using VoiceSpot to detect wakeword as default */
		this->VoiceSpotEnable = true;
		this->VITWakeWordEnable = false;
	}

	VIT_Handle_t SignalProcessor_VIT::VIT_open_model() {
		VIT_ReturnStatus_en       Status;                                   // Status of the function
		VIT_Handle_t              VITHandle = PL_NULL;                      // VIT handle pointer

		// General
		PL_UINT32                 i;
		PL_BOOL                   InitPhase_Error = PL_FALSE;
		VIT_InstanceParams_st     VITInstParams;                            // VIT instance parameters structure
		VIT_ControlParams_st      VITControlParams;                         // VIT control parameters structure
		PL_MemoryTable_st         VITMemoryTable;                           // VIT memory table descriptor
		PL_INT8*		pMemory[PL_NR_MEMORY_REGIONS];		// Pointers to dynamically allocated memory
		const PL_UINT8            *VIT_Model = VIT_Model_en;

		AFEConfig::AFEConfigState configState;
		this->VoiceSpotEnable = (configState.isConfigurationEnable("VoiceSpotDisable", 0) == 1)? false : true;
		this->VITWakeWordEnable = (configState.isConfigurationEnable("VITWakeWordEnable", 0) == 1)? true : false;
		std::string VIT_Model_Setting = configState.isConfigurationEnable("VITLanguage", "English");
		if (this->VoiceSpotEnable && this->VITWakeWordEnable) {
			printf("VIT Configuration error: VoiceSpot and VIT WakeWord detection can't work together!\n");
			exit(-1);
		}

		/*
		 *   VIT Set Model : register the Model in VIT
		 */
		if (VIT_Model_Setting == "Mandarin") {
			VIT_Model = VIT_Model_cn;
		}
		else if (VIT_Model_Setting == "English") {
			VIT_Model = VIT_Model_en;
		}
		else {
			printf("Warning: Unknown VIT model! Using English by default!\n");
			VIT_Model = VIT_Model_en;
		}
		Status = VIT_SetModel(VIT_Model, MODEL_LOCATION);
		if (Status != VIT_SUCCESS)
		{
			printf("VIT_SetModel error : %d\n", Status);
			exit(-1);                                        // We can exit from here since memory is not allocated yet
		}


		/*
		*   VIT Get Model Info (OPTIONAL)
		*       To retrieve information on the VIT_Model registered in VIT:
		*               - Model Release Number, number of commands supported
		*               - WakeWord supported (when info is present)
		*               - list of commands (when info is present)
		*
		*/
		VIT_ModelInfo_st Model_Info;
		Status = VIT_GetModelInfo(&Model_Info);
		if (Status != VIT_SUCCESS)
		{
			printf("VIT_GetModelInfo error : %d\n", Status);
			exit(-1);                                        // We can exit from here since memory is not allocated yet
		}

		printf("VIT Model info \n");
		printf("  VIT Model Release = 0x%04x\n", Model_Info.VIT_Model_Release);
		if (Model_Info.pLanguage != PL_NULL)
			printf("  Language supported : %s \n", Model_Info.pLanguage);

		printf("  Number of WakeWords supported : %d \n", Model_Info.NbOfWakeWords);
		printf("  Number of Commands supported : %d \n", Model_Info.NbOfVoiceCmds);
		if (!Model_Info.WW_VoiceCmds_Strings)               // Check here if Model is containing WW and CMDs strings
		{
			printf("  VIT_Model integrating WakeWord and Voice Commands strings : NO\n");
		}
		else
		{
			const char* ptr;
			printf("  WakeWord supported : \n");
			ptr = Model_Info.pWakeWord_List;
			if (ptr != PL_NULL)
			{
				for (PL_UINT16 i = 0; i < Model_Info.NbOfWakeWords; i++)
				{
					printf("   '%s' \n", ptr);
					ptr += strlen(ptr) + 1;  // to consider NULL char
				}
			}
			printf("  Voice commands supported : \n");
			ptr = Model_Info.pVoiceCmds_List;
			if (ptr != PL_NULL)
			{
				for (PL_UINT16 i = 0; i < Model_Info.NbOfVoiceCmds; i++)
				{
					printf("   '%s' \n", ptr);
					ptr += strlen(ptr) + 1;                 // to consider NULL char
				}
			}
		}


		/*
		 *   VIT Get Library information
		 */
		VIT_LibInfo_st Lib_Info;
		Status = VIT_GetLibInfo(&Lib_Info);
		if (Status != VIT_SUCCESS)
		{
			printf("VIT_GetLibInfo error : %d\n", Status);
			exit(-1);                                        // We can exit from here since memory is not allocated yet
		}

		/*
		 *   Configure VIT Instance Parameters
		 */
		 // Check that NUMBER_OF_CHANNELS is supported by VIT
		 // Retrieve from VIT_GetLibInfo API the number of channel supported by the VIT lib
		PL_UINT16 max_nb_of_Channels = Lib_Info.NumberOfChannels_Supported;
		if (NUMBER_OF_CHANNELS > max_nb_of_Channels)
		{
			printf("VIT lib is supporting only : %d channels\n", max_nb_of_Channels);
			exit(-1);                                        // We can exit from here since memory is not allocated yet
		}
		VITInstParams.SampleRate_Hz = VIT_SAMPLE_RATE;
		VITInstParams.SamplesPerFrame = VIT_SAMPLES_PER_10MS_FRAME;
		VITInstParams.NumberOfChannel = VIT_MAX_NUMBER_OF_CHANNEL;
		VITInstParams.APIVersion = VIT_API_VERSION;
#if defined CortexA55
		VITInstParams.DeviceId = VIT_IMX9XA55;
#elif defined CortexA53
		VITInstParams.DeviceId = VIT_IMX8MA53;
#else
		VITInstParams.DeviceId = VIT_IMX8MA53;
#endif
		/*
		 *   VIT get memory table : Get size info per memory type
		 */
		Status = VIT_GetMemoryTable(PL_NULL,                // VITHandle param should be NULL
					    &VITMemoryTable,
					    &VITInstParams);
		if (Status != VIT_SUCCESS)
		{
			printf("VIT_GetMemoryTable error : %d\n", Status);
			exit(-1);                                        // We can exit from here since memory is not allocated yet
		}


		/*
		 *   Reserve memory space : Malloc for each memory type
		 */
		for (i = 0; i < PL_NR_MEMORY_REGIONS; i++)
		{
			/* Log the memory size */
			if (VITMemoryTable.Region[i].Size != 0)
			{
				// reserve memory space
				// NB : VITMemoryTable.Region[PL_MEMREGION_PERSISTENT_FAST_DATA] should be allocated
				//      in the fatest memory of the platform (when possible) - this is not the case in this example.
				pMemory[i] = (PL_INT8 *)malloc(VITMemoryTable.Region[i].Size + MEMORY_ALIGNMENT);
				VITMemoryTable.Region[i].pBaseAddress = (void*)pMemory[i];
			}
		}

		/*
		*    Create VIT Instance
		*/
		VITHandle = PL_NULL;                            // force to null address for correct memory initialization
		Status = VIT_GetInstanceHandle(&VITHandle,
					       &VITMemoryTable,
					       &VITInstParams);
		if (Status != VIT_SUCCESS)
		{
			InitPhase_Error = PL_TRUE;
			printf("VIT_GetInstanceHandle error : %d\n", Status);
		}

		/*
		*    Test the reset (OPTIONAL)
		*/
		if (!InitPhase_Error)
		{
			Status = VIT_ResetInstance(VITHandle);
			if (Status != VIT_SUCCESS)
			{
				InitPhase_Error = PL_TRUE;
				printf("VIT_ResetInstance error : %d\n", Status);
			}
		}


		/*
		*   Set and Apply VIT control parameters
		*/
		VITControlParams.OperatingMode = VIT_OPERATING_MODE;
		if (this->VITWakeWordEnable && !this->VoiceSpotEnable) {
			printf("Using VIT for wakeword detection.\n");
			VITControlParams.OperatingMode = VIT_ALL_MODULE_ENABLE;
		}

		VITControlParams.Feature_LowRes = PL_FALSE;
		VITControlParams.Command_Time_Span = VIT_COMMAND_TIME_SPAN;
		if (!InitPhase_Error)
		{
			Status = VIT_SetControlParameters(VITHandle,
							  &VITControlParams);
			if (Status != VIT_SUCCESS)
			{
				InitPhase_Error = PL_TRUE;
				printf("VIT_SetControlParameters error : %d\n", Status);
			}
		}

		return VITHandle;
	}

	void SignalProcessor_VIT::VIT_close_model(VIT_Handle_t VITHandle) {
		VIT_InstanceParams_st     VITInstParams;                            // VIT instance parameters structure
		PL_MemoryTable_st         VITMemoryTable;                           // VIT memory table descriptor
		VIT_ReturnStatus_en       Status;                                   // Status of the function

		// retrieve size and address of the different MEM tables allocated
		// Should provide VIT_Handle to retrieve the size of the different MemTabs
		Status = VIT_GetMemoryTable(VITHandle,
					    &VITMemoryTable,
					    &VITInstParams);
		if (Status != VIT_SUCCESS)
		{
			printf("VIT_GetMemoryTable error : %d\n", Status);
			exit(-1);
		}

		// Free the VIT MEM tables
		for (int i = (PL_NR_MEMORY_REGIONS-1); i >= 0; i--)
		{
			if ((VITMemoryTable.Region[i].Size != 0) &&
			    (VITMemoryTable.Region[i].pBaseAddress != PL_NULL))
			{
				free((PL_INT8*)VITMemoryTable.Region[i].pBaseAddress);
			}
		}
	}

	bool SignalProcessor_VIT::VIT_Process_Phase(VIT_Handle_t VITHandle, int16_t* frame_data, int16_t* pCmdId) {
		VIT_ReturnStatus_en       Status;                                   // Status of the function
		VIT_DetectionStatus_en    VIT_DetectionResults = VIT_NO_DETECTION;  // VIT detection result

		VIT_VoiceCommand_st       VoiceCommand;                             // Voice Command id
		VIT_WakeWord_st         wakeWord;

		Status = VIT_Process(VITHandle,
				     (void *)frame_data,
				     &VIT_DetectionResults);
		if (Status == VIT_INVALID_DEVICE)
			static int ret = printf("Invalid Device : %d\n", Status);
		else if (Status != VIT_SUCCESS)
			printf("VIT_Process error : %d\n", Status);

		if (VIT_DetectionResults == VIT_WW_DETECTED)
		{
			// Retrieve id of the Wakeword detected
			// String of the WakeWord can also be retrieved (when WW and CMDs strings
			// are integrated in Model)
			Status = VIT_GetWakeWordFound(VITHandle, &wakeWord);
			if (Status != VIT_SUCCESS)
			{
				printf("VIT_GetWakeWordFound error: %d\r\n", Status);
				return false;
			}
			else
			{
				printf(" - Wakeword detected %d", wakeWord.Id);
				// Retrieve WW Name: OPTIONAL
				// Check first if WW string is present
				if (wakeWord.pName != PL_NULL)
				{
					printf(" %s\n", wakeWord.pName);
				}
				return true;
			}
		}
		else if (VIT_DetectionResults == VIT_VC_DETECTED)
		{
			// Retrieve id of the Voice Command detected
			// String of the Command can also be retrieved (when WW and CMDs strings are integrated in Model)
			Status = VIT_GetVoiceCommandFound(VITHandle, &VoiceCommand);
			if (Status != VIT_SUCCESS)
			{
				printf("VIT_GetVoiceCommandFound error : %d\n", Status);
			}
			else
			{
				printf(" - Voice Command detected %d", VoiceCommand.Id);
				*pCmdId = VoiceCommand.Id;

				// Retrieve CMD Name : OPTIONAL
				// Check first if CMD string is present
				if (VoiceCommand.pName != PL_NULL)
					printf(" %s\n\n", VoiceCommand.pName);
				return true;
			}
		}

		return false;
	}

	bool SignalProcessor_VIT::isVoiceSpotEnable() {
		return this->VoiceSpotEnable;
	}

	bool SignalProcessor_VIT::isVITWakeWordEnable() {
		return this->VITWakeWordEnable;
	}
}
