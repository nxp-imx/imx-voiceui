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
#ifndef RDSP_MEMORY_UTILS_PUBLIC
#define RDSP_MEMORY_UTILS_PUBLIC

#include <stddef.h>
#include <stdint.h>
#include "memcheck.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
    MEM_ALIGN_1 = 1,	/*!< Align 1 byte */
    MEM_ALIGN_2 = 2,	/*!< Align 2 byte */
    MEM_ALIGN_4 = 4,	/*!< Align 4 byte */
    MEM_ALIGN_8 = 8,	/*!< Align 8 byte */
    MEM_ALIGN_16 = 16,	/*!< Align 16 byte */
    //MEM_ALIGN_32 = 32,	/*!< Align 32 byte */
} MemAlign_t;


extern uint32_t rdsp_plugin_get_heapmem_analysis_flag();
extern void rdsp_plugin_set_heapmem_analysis_flag(int32_t Amode);

extern void rdsp_plugin_malloc_init(void* Aextmem_baseptr, void* Aextmem_nextptr, uint32_t AextmemSize);
extern uint32_t rdsp_plugin_malloc_GetAllocatedBytes();
extern void* rdsp_plugin_malloc(size_t Asize, int32_t align);
extern void* rdsp_plugin_malloc_func(size_t size, int32_t align);

extern void rdsp_plugin_scratch_init(void* Aext_scratch_mem_baseptr, void* Aext_scratch_mem_nextptr, uint32_t Aext_scratch_memSize);
extern uint32_t rdsp_plugin_scratch_GetAllocatedBytes();
extern void rdsp_plugin_scratch_reset();
extern void* rdsp_plugin_scratch_malloc(size_t Asize, int32_t Aalign);

extern void* rdsp_plugin_memset(void* Aptr, uint8_t Aval, uint32_t Asize);
extern void* rdsp_plugin_memcpy(void* Adest, void* Asrc, uint32_t Asize);
extern void* rdsp_plugin_memmove(void* Adest, const void* Asrc, uint32_t Asize);
extern uint32_t rdsp_plugin_memcompare(void* Ax, void* Ay, uint32_t Asize);
extern void rdsp_plugin_free(void* Aptr);

#ifdef _WIN32
#define RDSP_MEM_ALIGN
#else
#if defined(RDSP_128BIT_PLATFORM) || defined(__ARM_NEON)
#define RDSP_MEM_ALIGN __attribute__((aligned(16)))
#else
#define RDSP_MEM_ALIGN __attribute__((aligned(8)))
#endif
#endif

#if defined(RDSP_128BIT_PLATFORM) || defined(__ARM_NEON)
#define RDSP_PLUGIN_MALLOC_ALIGN(type, n) (type*)rdsp_plugin_malloc(sizeof(type)*n, MEM_ALIGN_16)
#define RDSP_PLUGIN_SCRATCH_MALLOC_ALIGN(type, n) (type*)rdsp_plugin_scratch_malloc(sizeof(type)*n, MEM_ALIGN_16)
#else
#define RDSP_PLUGIN_MALLOC_ALIGN(type, n) (type*)rdsp_plugin_malloc(sizeof(type)*n, MEM_ALIGN_8)
#define RDSP_PLUGIN_SCRATCH_MALLOC_ALIGN(type, n) (type*)rdsp_plugin_scratch_malloc(sizeof(type)*n, MEM_ALIGN_8)
#endif

#ifdef __cplusplus
}
#endif

#endif /* RDSP_MEMORY_UTILS_PUBLIC */
