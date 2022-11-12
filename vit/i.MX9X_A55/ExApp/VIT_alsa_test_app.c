/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// This test is exercising the NXP Voice UI solution: Voice Intelligent Technology.
// VIT is supporting wakeword detection and voice commands recognition features.
// Custom Wakeword and Voice commands can be defined with the VIT Model
// Generation online tool : vit.nxp.com
// More information on VIT technology can be found on the VIT web site :
// https://www.nxp.com/design/software/embedded-software/voice-intelligent-technology:VOICE-INTELLIGENT-TECHNOLOGY


#include "VIT_helper.h"
#include "VIT_action_executor.h"

#include "VIT_Model_en.h"
#include "VIT_Model_cn.h"
#ifdef MULTI_LANGUAGES
#include "VIT_Model_tr.h"
#include "VIT_Model_de.h"
#include "VIT_Model_es.h"
#include "VIT_Model_ja.h"
#include "VIT_Model_ko.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <asm/unistd.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/syscall.h>

#include <alsa/asoundlib.h>

#if defined  VERSION_CTRL
__attribute__((unused)) static char const ident_core_functions[] = "$Id: core_functions "CORE_FUNCTIONS_VER" $";
__attribute__((unused)) static char const ident_makefiles[] = "$Id: makefiles "MAKEFILES_VER" $";
__attribute__((unused)) static char const ident_vce[] = "$Id: vce "VCE_VER" $";
__attribute__((unused)) static char const ident_vit[] = "$Id: vit "VIT_VER" $";
__attribute__((unused)) static char const ident_vit_tests[] = "$Id: vit_tests "VIT_TESTS_VER" $";
#endif

#define RECOVER_RETRY_COUNT 5

int g_verbose = 0;
int g_time = 60; // one minute
int g_iterations;

extern snd_pcm_t *g_capture_handle;
extern snd_pcm_format_t g_format;
extern int g_buffer_frames;

#if defined(ARCH_AARCH64)
typedef struct {
  const char *name;
} imx_names_t;

static imx_names_t imx_names[] = {
  [VIT_IMX8MA53] = {"IMX8MA53"},
  [VIT_IMX9XA55] = {"IMX9XA55"},
};
#endif

static void display_usage() {
  LOG("vit-imx\n");
  LOG( "--device, -d: alsa_device, alsa device\n");
#if defined(ARCH_AARCH64)
  LOG( "--imx_device, -i: i.MX device: \n\t");
  for(int i = 0; i < VIT_NB_OF_DEVICES + 1; i++)
  {
    if (imx_names[i].name != NULL)
    {
      LOG("%s, ", imx_names[i].name);
    }
  }
  LOG( "\n");
#endif
  LOG( "--language, -l: language, ENGLISH, MANDARIN\n");
  LOG( "--show_commands, -s: language, ENGLISH, MANDARIN show available commands\n");
  LOG( "--verbose, -v: [0|1]: verbose\n");
  LOG( "--time, -t: [int]: detection duration in sec\n");
  LOG( "\n");
}

// Configure the detection period in second for each command
// VIT will return UNKNOWN if no command is recognized during this time span.
#define VIT_COMMAND_TIME_SPAN 3.0 // in second

/*!
 * @brief Main function
 */
int main(int argc, char *argv[])
{
  char *alsa_device = NULL;
  char *language = NULL;
  int show_commands = 0;
#if defined(ARCH_AARCH64)
  char *imx_device = NULL;
  int device_id = VIT_IMX8MA53;
#else
  int device_id = -1;
#endif

  g_iterations = g_time * 100;
  int c;
  int retry = RECOVER_RETRY_COUNT;
  const int silent = 0;

  while (1) {
    static struct option long_options[] = {
      {"device", required_argument, NULL, 'd'},
      {"language", required_argument, NULL, 'l'},
      {"verbose", required_argument, NULL, 'v'},
      {"show_commands", required_argument, NULL, 's'},
      {"time", required_argument, NULL, 't'},
#if defined(ARCH_AARCH64)
      {"imx_device", optional_argument, NULL, 'i'},
#endif
      {NULL, 0, NULL, 0}};

    /* getopt_long stores the option index here. */
    int option_index = 0;

#if defined(ARCH_AARCH64)
    const char *optstring = "d:l:v:s:t:i:";
#else
    const char *optstring = "d:l:v:s:t:";
#endif
    c = getopt_long(argc, argv,
                    optstring, long_options,
                    &option_index);

    /* Detect the end of the options. */
    if (c == -1) break;

    switch (c) {
      case 'd':
        alsa_device = optarg;
        break;
      case 'l':
        language = optarg;
        break;
      case 'v':
        g_verbose = strtol(optarg, NULL, 10);
        break;
      case 's':
        language = optarg;
        show_commands = 1;
        break;
      case 't':
        g_time = strtol(optarg, NULL, 10);
        g_iterations = g_time * 100;
        break;
#if defined(ARCH_AARCH64)
      case 'i':
        imx_device = optarg;
        break;
#endif
      case 'h':
      case '?':
        display_usage();
        exit(-1);
      default:
        exit(-1);
    }
  }

  if (argc < 3) {
    display_usage();
    exit(-1);
  }

#if defined(ARCH_AARCH64)
  if (imx_device != NULL) {
    int i;
    for(i = 0; i < VIT_NB_OF_DEVICES + 1; i++)
    {
      if (imx_names[i].name && strncmp(imx_device, imx_names[i].name, strlen(imx_names[i].name)) == 0)
      {
        device_id = i;
        break;
      }
    }
    if (i > VIT_NB_OF_DEVICES)
    {
      ERROR("\nWrong i.MX device! (%d)\n", i);
      display_usage();
      return -1;
    }
  } else {
      LOG("\ni.MX device (--i) not specified, defaulting to %s\n\n", imx_names[device_id].name);
  }
#endif

  if ((show_commands == 0) && (set_up_alsa(alsa_device) != 0)) {
    ERROR("Alsa setup failed\n");
    return -1;
  }

#ifdef PERF_MEASURE
  int perf_fd = set_up_perf();
#endif

  char *buffer;
  int err;

  PL_MemoryTable_st       memorytable;
  PL_INT16                *inputbuffer;
  VIT_Handle_t            handle = PL_NULL;
  VIT_ControlParams_st    controlparams;
  VIT_ReturnStatus_en     status = 0;
  VIT_VoiceCommand_st     voiceCommand;
  VIT_WakeWord_st         wakeWord;

  VIT_InstanceParams_st   instparams = {
    .SampleRate_Hz   = VIT_SAMPLE_RATE,
    .SamplesPerFrame = g_buffer_frames,
    .NumberOfChannel = _1CHAN,
    .APIVersion      = VIT_API_VERSION,
    .DeviceId        = device_id
  };
  const PL_UINT8            *VIT_Model = VIT_Model_en;
  size_t model_size = sizeof(VIT_Model_en);
  VIT_DetectionStatus_en    VIT_DetectionResults = VIT_NO_DETECTION;

  if (language != NULL) {
    if (strcasecmp(language, "ENGLISH") == 0)
    {
      VIT_Model = VIT_Model_en;
      model_size = sizeof(VIT_Model_en);
    }
    else if (strcasecmp(language, "MANDARIN") == 0)
    {
      VIT_Model = VIT_Model_cn;
      model_size = sizeof(VIT_Model_cn);
    }
#ifdef MULTI_LANGUAGES
    else if (strcasecmp(language, "TURKISH") == 0)
    {
      VIT_Model = VIT_Model_tr;
      model_size = sizeof(VIT_Model_tr);
    }
    else if (strcasecmp(language, "GERMAN") == 0)
    {
      VIT_Model = VIT_Model_de;
      model_size = sizeof(VIT_Model_de);
    }
    else if (strcasecmp(language, "SPANISH") == 0)
    {
      VIT_Model = VIT_Model_es;
      model_size = sizeof(VIT_Model_es);
    }
    else if (strcasecmp(language, "JAPANESE") == 0)
    {
      VIT_Model = VIT_Model_ja;
      model_size = sizeof(VIT_Model_ja);
    }
    else if (strcasecmp(language, "KOREAN") == 0)
    {
      VIT_Model = VIT_Model_ko;
      model_size = sizeof(VIT_Model_ko);
    }
#endif
    else {
      ERROR("Language '%s' not supported\n", language);
      return -1;
    }
  }
  pid_t tid = syscall(SYS_gettid);
  LOG("Main tid %d\n", tid);


  VIT_run_thread();

  /*
   * VIT Set Model
   */
  status = VIT_SetModel(VIT_Model, VIT_MODEL_IN_ROM);
  CHECK(status, "VIT_SetModel");

  INFO("Model size %zu Bytes\n", model_size);

  /*
   * VIT Get Model Info
   */
  VIT_ModelInfo_st Model_Info;
  status = VIT_GetModelInfo(&Model_Info);
  CHECK(status, "VIT_GetModelInfo");

  INFO("VIT Model info \n");
  INFO("  VIT Model Release = 0x%04x\n", Model_Info.VIT_Model_Release);
  if (Model_Info.pLanguage != PL_NULL)
  {
    INFO("  Language supported : %s \n", Model_Info.pLanguage);
  }

  INFO("  Number of WakeWords supported : %d \n", Model_Info.NbOfWakeWords);
  INFO("  Number of Commands  supported : %d \n", Model_Info.NbOfVoiceCmds);
  if (!Model_Info.WW_VoiceCmds_Strings)
  {
    LOG("VIT_Model does not integrate WakeWord and Voice Commands strings\n");
  }
  else
  {
    const char* ptr;

    INFO("\nVIT_Model integrates WakeWord and Voice Commands strings\n");
    LOG("WakeWord supported : \n");
    ptr = Model_Info.pWakeWord_List;
    if (ptr != PL_NULL)
    {
      for (PL_UINT16 i = 0; i < Model_Info.NbOfWakeWords; i++)
      {
        LOG("   '%s' \n", ptr);
        ptr += strlen(ptr) + 1;  // to consider NULL char
      }
    }
    LOG("\nVoice commands supported : \n");
    ptr = Model_Info.pVoiceCmds_List;
    if (ptr != PL_NULL)
    {
      for (PL_UINT16 i = 0; i < Model_Info.NbOfVoiceCmds; i++)
      {
        LOG("   '%s' \n", ptr);
        ptr += strlen(ptr) + 1;  // to consider NULL char
      }
    }
  }
  if (show_commands) {
    return 0;
  }

  // Get Lib Info
  VIT_LibInfo_st Lib_Info;
  status = VIT_GetLibInfo(&Lib_Info);
  CHECK(status, "VIT_GetLibInfo");

  // VIT get memory table
  status = VIT_GetMemoryTable(PL_NULL, &memorytable, &instparams);
  CHECK(status, "VIT_GetMemoryTable");

  // Set up memory
  VIT_SetupMemoryRegion(&memorytable);

  // Get handle
  status = VIT_GetInstanceHandle(&handle, &memorytable, &instparams);
  CHECK(status, "VIT_GetInstanceHandle");

  // Reset Instance
  status = VIT_ResetInstance(handle);
  CHECK(status, "VIT_ResetInstance");

  // Set VIT control parameters
  controlparams.OperatingMode = VIT_LPVAD_ENABLE | VIT_WAKEWORD_ENABLE | VIT_VOICECMD_ENABLE;
  controlparams.Feature_LowRes = PL_FALSE;
  controlparams.Command_Time_Span  = VIT_COMMAND_TIME_SPAN; // in second


  // Apply VIT Control Parameters
  status = VIT_SetControlParameters(handle, &controlparams);
  CHECK(status, "VIT_SetControlParameters");

  PL_INT32 frame_cnt = 0;

  LOG("VIT example started!\n");

  buffer = malloc(g_buffer_frames * snd_pcm_format_width(g_format) / 8 );
  LOG("Buffer allocated %d \n", (g_buffer_frames * snd_pcm_format_width(g_format) / 8 ));

  for (int i = 0; i < g_iterations; ++i)
  {
    if ((err = snd_pcm_readi(g_capture_handle, buffer, g_buffer_frames)) != g_buffer_frames)
    {
      ERROR("read from audio interface failed (%d %s), trying to recover\n",
          err, snd_strerror(err));
      if ((err = snd_pcm_recover(g_capture_handle, err, silent)))
      {
        ERROR("fail to recover pcm read (%d, %s), retry (%d)\n", err, snd_strerror(err), --retry);
        if (!retry)
        {
          break;
        }
      }
      else
      {
        LOG("pcm read recover success!\n");
        retry = RECOVER_RETRY_COUNT;
      }

    }

    inputbuffer = (PL_INT16 *)buffer;
    status = VIT_Process(handle, (void*)inputbuffer, &VIT_DetectionResults);
    CHECK(status, "VIT_Process");

    VIT_StatusParams_st        VIT_StatusParams_Buffer;
    VIT_GetStatusParameters(handle, &VIT_StatusParams_Buffer,
        sizeof(VIT_StatusParams_Buffer));


    static PL_BOOL VAD_DETECTION_ON = PL_FALSE;
    if (VIT_StatusParams_Buffer.LPVAD_EventDetected)
    {
      if (!VAD_DETECTION_ON)
      {
        LOG("%d VAD DETECTION \n", frame_cnt);
      }
      VAD_DETECTION_ON = PL_TRUE;
    }
    else {
      if (VAD_DETECTION_ON)
      {
        LOG(" VAD NO DETECTION \n");
      }
      VAD_DETECTION_ON = PL_FALSE;
    }

    if (VIT_DetectionResults == VIT_WW_DETECTED)
    {
      // Retrieve id of the Wakeword detected
      // String of the WakeWord can also be retrieved (when WW and CMDs strings
      // are integrated in Model)
      status = VIT_GetWakeWordFound(handle, &wakeWord);
      if (status != VIT_SUCCESS)
      {
        ERROR("VIT_GetWakeWordFound error: %d\r\n", status);
        break;
      }
      else
      {
        LOG(" - Wakeword detected %d", wakeWord.Id);

        // Retrieve WW Name: OPTIONAL
        // Check first if WW string is present
        if (wakeWord.pName != PL_NULL)
        {
          LOG(" %s\n", wakeWord.pName);
        }
        else
        {
          LOG("\n");
        }
      }
    }
    else if (VIT_DetectionResults == VIT_VC_DETECTED)
    {
      // Retrieve id of the Voice Command detected
      // String of the Command can also be retrieved (when WW and CMDs strings
      // are integrated in Model)
      status = VIT_GetVoiceCommandFound(handle, &voiceCommand);
      if (status != VIT_SUCCESS)
      {
        ERROR("VIT_GetVoiceCommandFound error: %d\r\n", status);
        break;
      }
      else
      {
        LOG(" - Voice Command detected %d", voiceCommand.Id);

        VIT_send_cmd(voiceCommand.Id);

        // Retrieve CMD Name: OPTIONAL
        // Check first if CMD string is present
        if (voiceCommand.pName != PL_NULL)
        {
          LOG(" %s\n", voiceCommand.pName);
        }
        else
        {
          LOG("\n");
        }
      }
    }

    frame_cnt++;
  }

  free(buffer);

  INFO("buffer freed\n");
  snd_pcm_close(g_capture_handle);
  INFO("audio interface closed\n");

  VIT_stop_thread();
  LOG("VIT example finished!\n");

error:
  return status;
}
