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
#include "RdspMemoryUtilsPublic.h"

#define RDSP_MEMORY_UTILS_USES_STDLIB 0
#if RDSP_MEMORY_UTILS_USES_STDLIB==1
#include <stdlib.h>
#endif

#define RDSP_MEMORY_UTILS_USES_STRING 1
#if RDSP_MEMORY_UTILS_USES_STRING==1
#include <string.h>
#endif

void* rdsp_plugin_malloc(size_t Asize, int32_t align) {
#ifdef RDSP_ENABLE_MEMCHECK
	void* p = memcheck_malloc_align(Asize, align, NULL, 0, NULL);
	return p;
#else
	return rdsp_plugin_malloc_func(Asize, align);
#endif
}

static void* extmem_baseptr;
static void* extmem_nextptr;
static uint32_t extmemSize;
static uint32_t extmem_count_bytes;
static uint32_t extmem_analysis_mode_flag = 0;

void rdsp_plugin_set_heapmem_analysis_flag(int32_t Aanalysis_mode_flag) {
	extmem_analysis_mode_flag = Aanalysis_mode_flag;
}

uint32_t rdsp_plugin_get_heapmem_analysis_flag() {
	return extmem_analysis_mode_flag;
}

void rdsp_plugin_malloc_init(void* Aextmem_baseptr, void* Aextmem_nextptr, uint32_t AextmemSize) {
	extmem_baseptr = Aextmem_baseptr;
	extmem_nextptr = Aextmem_nextptr;
	extmemSize = AextmemSize;
	extmem_count_bytes = 0;
}

uint32_t rdsp_plugin_malloc_GetAllocatedBytes() {
	return extmem_count_bytes;
}

void* rdsp_plugin_malloc_func(size_t size, int32_t align) {
	void* pRet = NULL;
	uint32_t aligned_size = (size + align - 1) & ~(align - 1);

	/* For heap memory analysis */
	if (extmem_analysis_mode_flag == 1)	{
		extmem_count_bytes += aligned_size;
		return (void*) 0xFFFF;
	}

#if RDSP_MEMORY_UTILS_USES_STDLIB==1
	pRet = malloc(aligned_size);
#else
	uintptr_t tmp;
	int32_t failed = 0;
	if ((size <= extmemSize) && (align <= MEM_ALIGN_16)) {
		/* Do not allocate if we do not have enough memory */
		if (extmemSize >= aligned_size) {
			tmp = (uintptr_t)extmem_nextptr;
			pRet = (void*)((tmp + (uintptr_t)(align - 1))
				& ~(uintptr_t)(align - 1));

			extmem_nextptr = (void*)((uintptr_t)pRet + aligned_size);
			extmemSize -= aligned_size;
		}
		else
			failed = 1;
	}
	else
		failed = 1;
	
	extmem_count_bytes += aligned_size;

#ifdef RDSP_ENABLE_PRINTF
	if (failed == 1)
		printf("Not enough memory available or requested alignment not supported!\n");

	printf("Allocated %i bytes => count_malloc_bytes = %i\n", aligned_size, extmem_count_bytes);

	// Check for alignment
	if ((uint32_t) pRet % align)
		printf("Alignment not correct!\n");
#endif
#endif // RDSP_MEMORY_UTILS_USES_STDLIB==1

	return pRet;
}

static void* ext_base_scratch_mem_baseptr;
static void* ext_base_scratch_mem_nextptr;
static uint32_t ext_base_scratch_memSize;

static void* ext_scratch_mem_baseptr;
static void* ext_scratch_mem_nextptr;
static uint32_t ext_scratch_memSize;
static uint32_t ext_scratch_mem_count_bytes;
static uint32_t ext_scratch_max_mem_count_bytes;

void rdsp_plugin_scratch_init(void* Aext_scratch_mem_baseptr,
		void* Aext_scratch_mem_nextptr, uint32_t Aext_scratch_memSize) {

	ext_base_scratch_mem_baseptr = Aext_scratch_mem_baseptr;
	ext_base_scratch_mem_nextptr = Aext_scratch_mem_nextptr;
	ext_base_scratch_memSize = Aext_scratch_memSize;

	ext_scratch_mem_baseptr = ext_base_scratch_mem_baseptr;
	ext_scratch_mem_nextptr = ext_base_scratch_mem_nextptr;
	ext_scratch_memSize = ext_base_scratch_memSize;

	ext_scratch_mem_count_bytes = 0;
	ext_scratch_max_mem_count_bytes = 0;
}

uint32_t rdsp_plugin_scratch_GetAllocatedBytes() {
	return ext_scratch_mem_count_bytes;
}

void rdsp_plugin_scratch_reset() {

	/* For heap memory analysis */
	if (extmem_analysis_mode_flag == 1)
		return;

	if (ext_scratch_mem_count_bytes > ext_scratch_max_mem_count_bytes)
		ext_scratch_max_mem_count_bytes = ext_scratch_mem_count_bytes;
	ext_scratch_mem_count_bytes = 0;

	ext_scratch_mem_baseptr = ext_base_scratch_mem_baseptr;
	ext_scratch_mem_nextptr = ext_base_scratch_mem_nextptr;
	ext_scratch_memSize = ext_base_scratch_memSize;

}

void* rdsp_plugin_scratch_malloc(size_t Asize, int32_t Aalign) {

	void *pRet = NULL;
	uint32_t aligned_size = (Asize + Aalign - 1) & ~(Aalign - 1);

	/* For heap memory analysis */
	if (extmem_analysis_mode_flag == 1) {
		return (void*) 0xFFFF;
	}

#if RDSP_MEMORY_UTILS_USES_STDLIB==1
	pRet = malloc(aligned_size);
#else
	uint64_t tmp;
	if ((Asize <= ext_scratch_memSize) && (Aalign <= MEM_ALIGN_16)) {
		/* Do not allocate if we do not have enough memory */
		if (ext_scratch_memSize >= aligned_size) {
			tmp = (uint64_t) (intptr_t) ext_scratch_mem_nextptr;
			pRet = (void *) (intptr_t) ((tmp + Aalign - 1)
					& ~(uint64_t) (Aalign - 1));

			ext_scratch_mem_nextptr =
					(void*) (intptr_t) ((uint64_t) ((intptr_t) pRet)
							+ aligned_size);
			ext_scratch_memSize -= aligned_size;
		}

		ext_scratch_mem_count_bytes += aligned_size;

#ifdef RDSP_SIM
//		printf("scratch_malloc(%i) \t address = %x, mem_avail = %i\n", Asize, (uint32_t) pRet, ext_scratch_mem_count_bytes );
#endif
	} else {
#ifdef RDSP_ENABLE_PRINTF
		printf("Not enough memory available! Requested %i scratch bytes but only %i scratch bytes allocated.\n", Asize, ext_base_scratch_memSize);
#endif
	}
#endif // RDSP_MEMORY_UTILS_USES_STDLIB==1
	return pRet;
}

void* rdsp_plugin_memset(void* Aptr, uint8_t Aval, uint32_t Asize) {
#if RDSP_MEMORY_UTILS_USES_STRING==1
	return memset(Aptr, Aval, Asize);
#else
    // Replicate val from 8b to 32b
    uint32_t val32;
    uint8_t* val8 = (uint8_t*)&val32;
    uint32_t i;
    for (i = 0; i < 4; i++)
        val8[i] = Aval;

    // Set 32b chunks
    i = 0;
    uint32_t* ptr32 = (uint32_t*)Aptr;
    for (; i < Asize >> 2; i++)
        *ptr32++ = val32;

    // Do the rest
    uint8_t* ptr8 = (uint8_t*)ptr32;
    int32_t rem = Asize - (i << 2);
    while (rem-- > 0)
        *ptr8++ = Aval;

    return Aptr;
#endif
}

void* rdsp_plugin_memcpy(void* Adest, void* Asrc, uint32_t Asize) {
#if RDSP_MEMORY_UTILS_USES_STRING==1
	return memcpy(Adest, Asrc, Asize);
#else
	uint32_t *src32 = (uint32_t *) Asrc;
	uint32_t *dst32 = (uint32_t *) Adest;
    uint32_t i = 0;
	for(; i < Asize >> 2; i++)
		*dst32++ = *src32++;

	// Do the rest
	uint8_t *src8 = (uint8_t *) src32;
	uint8_t *dst8 = (uint8_t *) dst32;
	int32_t rem = Asize - (i << 2);
	while (rem-- > 0)
		*dst8++ = *src8++;

    return Adest;
#endif
}

void* rdsp_plugin_memmove(void* Adest, const void* Asrc, uint32_t Asize) {
#if RDSP_MEMORY_UTILS_USES_STRING==1
	return memmove(Adest, Asrc, Asize);
#else
	//#warning rdsp_plugin_memmove requires RDSP_MEMORY_UTILS_USES_STRING==1
	return memmove(Adest, Asrc, Asize);
//	return NULL;
#endif
}

uint32_t rdsp_plugin_memcompare(void* Ax, void* Ay, uint32_t Asize) {
#if RDSP_MEMORY_UTILS_USES_STRING==1
	return memcmp(Ax, Ay, Asize);
#else
	const uint32_t *x32 = (const uint32_t *) Ax;
	const uint32_t *y32 = (const uint32_t *) Ay;
	uint32_t i = 0;
	for (; i < Asize >> 2; i++) {
		if (*x32++ != *y32++)
			return 1;
	}

	// Do the rest
	int32_t rem = Asize - (i << 2);
	const uint8_t *x8 = (const uint8_t *) x32;
	const uint8_t *y8 = (const uint8_t *) y32;
	while (rem-- > 0) {
		if (*x8++ != *y8++)
			return 1;
	}
	return 0;
#endif
}

void rdsp_plugin_free(void* Aptr) {
}

