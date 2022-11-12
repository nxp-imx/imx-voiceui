/*
 * Copyright 2020-2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "VIT_helper.h"
#include <stdlib.h>
#include <alsa/asoundlib.h>

static PL_INT8    *pMemory[PL_NR_MEMORY_REGIONS];

void VIT_SetupMemoryRegion(PL_MemoryTable_st *memorytable)
{
  PL_UINT32               i;
  for (i = 0; i < PL_NR_MEMORY_REGIONS; i++)
  {
    /* Log the memory size */
    INFO("Memory region %d, size %d in Bytes, %.2f kBytes \n",
        (int)i,
        (int)memorytable->Region[i].Size,
        (float)memorytable->Region[i].Size/1024);

    if (memorytable->Region[i].Size != 0)
    {
      // reserve memory space for Size
      pMemory[i] = malloc(memorytable->Region[i].Size);
      memorytable->Region[i].pBaseAddress =
        (void *)(pMemory[i]);

      INFO(" Memory region address 0x%p\n",
          memorytable->Region[i].pBaseAddress);
    }
  }
}
snd_pcm_t *g_capture_handle;
snd_pcm_format_t g_format = SND_PCM_FORMAT_S16_LE;
int g_buffer_frames = VIT_SAMPLES_PER_30MS_FRAME;

int set_up_alsa (const char * device)
{
  int err;
  unsigned int rate = VIT_SAMPLE_RATE;
  snd_pcm_hw_params_t *hw_params;

  if ((err = snd_pcm_open(&g_capture_handle, device, SND_PCM_STREAM_CAPTURE, 0)) < 0)
  {
    ERROR("Error: opening audio device %s failed (%s)\n", device, snd_strerror (err));
    return -1;
  }
  INFO("Audio interface opened\n");

  if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
  {
    ERROR("Error: allocatinng hardware parameter failed: %s\n",
        snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Allocated\n");

  if ((err = snd_pcm_hw_params_any(g_capture_handle, hw_params)) < 0)
  {
    ERROR("Error: initializing hardware parameter structure failed: %s \n",
        snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Initialized\n");

  if ((err = snd_pcm_hw_params_set_access(g_capture_handle, hw_params,
          SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    ERROR("Error: setting access type failed %s\n", snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Access OK\n");

  if ((err = snd_pcm_hw_params_set_format(g_capture_handle, hw_params, g_format)) < 0)
  {
    ERROR("Error: setting sample format failed %s\n", snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Format OK\n");

  if ((err = snd_pcm_hw_params_set_rate(g_capture_handle, hw_params, rate, 0)) < 0)
  {
    ERROR("Error: setting sample rate failed %s\n", snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Rate OK\n");

  if ((err = snd_pcm_hw_params_set_channels(g_capture_handle, hw_params, 1)) < 0)
  {
    ERROR("Error: setting channel count failed %s\n", snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: Channels OK\n");

  if ((err = snd_pcm_hw_params(g_capture_handle, hw_params)) < 0)
  {
    ERROR("Error: setting parameters failed %s\n", snd_strerror (err));
    return -1;
  }
  INFO("Hardware parameter: OK\n");

  snd_pcm_hw_params_free(hw_params);
  INFO("Hardware parameter: Freed\n");

  if ((err = snd_pcm_prepare(g_capture_handle)) < 0)
  {
    ERROR("Error: preparing audio interface for use failed %s\n", snd_strerror (err));
    return -1;
  }

  INFO("Audio interface prepared\n");
  return 0;
}

int VIT_Print_Error(VIT_ReturnStatus_en  status, const char * f)
{
  switch (status)
  {
    case VIT_SUCCESS:
      return 0;
      break;
    case VIT_INVALID_BUFFER_MEMORY_ALIGNMENT:
      ERROR("%s: %s\n",f, "VIT_INVALID_BUFFER_MEMORY_ALIGNMENT");
      break;
    case VIT_INVALID_NULLADDRESS:
      ERROR("%s: %s\n",f, "VIT_INVALID_NULLADDRESS");
      break;
    case VIT_INVALID_ARGUMENT:
      ERROR("%s: %s\n",f, "VIT_INVALID_ARGUMENT");
      break;
    case VIT_INVALID_PARAMETER_OUTOFRANGE:
      ERROR("%s: %s\n",f, "VIT_INVALID_PARAMETER_OUTOFRANGE");
      break;
    case VIT_INVALID_SAMPLE_RATE:
      ERROR("%s: %s\n",f, "VIT_INVALID_SAMPLE_RATE");
      break;
    case VIT_INVALID_FRAME_SIZE:
      ERROR("%s: %s\n",f, "VIT_INVALID_FRAME_SIZE");
      break;
    case VIT_INVALID_MODEL:
      ERROR("%s: %s\n",f, "VIT_INVALID_MODEL");
      break;
    case VIT_INVALID_API_VERSION:
      ERROR("%s: %s\n",f, "VIT_INVALID_API_VERSION");
      break;
    case VIT_INVALID_STATE:
      ERROR("%s: %s\n",f, "VIT_INVALID_STATE");
      break;
    case VIT_INVALID_DEVICE:
      ERROR("%s: %s\n",f, "VIT_INVALID_DEVICE");
      break;
    case VIT_SYSTEM_ERROR:
      ERROR("%s: %s\n",f, "VIT_SYSTEM_ERROR");
      break;
    case VIT_ERROR_UNDEFINED:
      ERROR("%s: %s\n",f, "VIT_ERROR_UNDEFINED");
      break;
    case VIT_DUMMY_ERROR:
      ERROR("%s: %s\n",f, "VIT_DUMMY_ERROR");
      break;
  }
  return -1;
}
