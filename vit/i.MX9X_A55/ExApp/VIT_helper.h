/*
* Copyright 2020-2022 NXP
*
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef VIT_HELPER_H
#define VIT_HELPER_H
#ifdef ARCH_AARCH64
#include "PL_platformTypes_CortexA.h"
#else
#include "PL_platformTypes_linux.h"
#endif
#include "VIT.h"

extern int g_verbose;

#define INFO(...) \
  if (g_verbose)\
    fprintf(stdout, __VA_ARGS__)

#define LOG(...) \
  fprintf(stdout, __VA_ARGS__)

#define ERROR(...) \
  fprintf(stderr, __VA_ARGS__)

void VIT_SetupMemoryRegion(PL_MemoryTable_st *memorytable);

int VIT_Print_Error(VIT_ReturnStatus_en  status, const char * f);

#define CHECK(STATUS, F) \
    do { if (VIT_Print_Error((STATUS), (F)) == -1) \
        goto error;\
    } while(0)

int set_up_alsa (const char * device);

#endif
