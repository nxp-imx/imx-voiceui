/*Copyright 2021 Retune DSP 
* Copyright 2022 NXP 
*
* NXP Confidential. This software is owned or controlled by NXP 
* and may only be used strictly in accordance with the applicable license terms.  
* By expressly accepting such terms or by downloading, installing, 
* activating and/or otherwise using the software, you are agreeing that you have read, 
* and that you agree to comply with and are bound by, such license terms.  
* If you do not agree to be bound by the applicable license terms, 
* then you may not retain, install, activate or otherwise use the software.
*
*/
#ifndef RDSP_FILE_UTILS_PUBLIC
#define RDSP_FILE_UTILS_PUBLIC

//#if RDSP_CONVERSA_LIB_ENABLE_FILEIO

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#if USE_FATFS
#include "ff.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	FILE_OK = 0,
    FILE_GENERAL_ERROR = 1,
} RdspFileStatus;

extern RdspFileStatus rdsp__fopen(void* Afile, char* Afilename, const char* Arw);
extern RdspFileStatus rdsp__fwrite(void* Ain, int32_t Asize, int32_t Anum, void* Afile);

#ifdef __cplusplus
}
#endif

//#endif // RDSP_CONVERSA_LIB_ENABLE_FILEIO

#endif /* RDSP_MEMORY_UTILS_PUBLIC */


