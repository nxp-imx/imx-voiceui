/*
* Copyright 2020 NXP
*
* NXP Confidential. This software is owned or controlled by NXP and may only
* be used strictly in accordance with the applicable license terms found in
* file LICENSE.txt
*/

#ifndef PL_PLATFORM_TYPES_CORTEXA_
#define PL_PLATFORM_TYPES_CORTEXA_

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************************************/
/*                                                                                      */
/*  Includes                                                                            */
/*                                                                                      */
/****************************************************************************************/

#include <limits.h>
#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <stdint.h>




#define     PL_NULL                NULL                   ///< NULL pointer


typedef _Bool           PL_BOOL;
#define PL_TRUE         1
#define PL_FALSE        0

typedef unsigned char	PL_UINT8;
typedef signed char	        PL_CHAR;
typedef signed char	        PL_INT8;

typedef	short	        PL_INT16;
typedef unsigned short  PL_UINT16;

typedef int32_t     PL_INT32;
typedef uint32_t    PL_UINT32;

typedef int64_t    PL_INT64;
typedef uint64_t   PL_UINT64;

typedef float      PL_FLOAT;
typedef double     PL_DOUBLE;

typedef uintptr_t  PL_UINTPTR;


#define     PL_MAXENUM             2147483647          ///< Maximum value for enumerator


// Memory alignment
#define     PL_MEM_ALIGN(var, alignbytes)      var  __attribute__((aligned(alignbytes)))

//Alignment required by VIT model
#define     VIT_MODEL_ALIGN_BYTES      64


#include "PL_memoryRegion.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PL_PLATFORM_TYPES_CORTEXA_ */
