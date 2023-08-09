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
#ifndef RDSP_STRING_UTILS_PUBLIC
#define RDSP_STRING_UTILS_PUBLIC

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif


#if SDK_DEBUGCONSOLE
// Use FSL driver for MCUXpresso builds
extern int DbgConsole_Printf(const char *formatString, ...);
#define rdsp_printf(...) DbgConsole_Printf(__VA_ARGS__)
#else
// Use stdio
#define rdsp_printf(...) printf(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif /* RDSP_STRING_UTILS_PUBLIC */


