/*
Copyright (c) 2021 by Retune DSP. This code is the confidential and
proprietary property of Retune DSP and the possession or use of this
file requires a written license from Retune DSP.

Contact information: info@retune-dsp.com
                     www.retune-dsp.com
*/

#ifndef RDSP_BUFFER_H
#define RDSP_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RDSP_STATIC
#define RDSP_STATIC
#endif


#include <stdint.h>


// Circular block-buffer implementation

typedef struct rdsp_buffer_struct {
	int32_t block_size;							// Size of blocks (in bytes)
	int32_t num_blocks;							// Size of buffer (in blocks)
	int32_t index_input;						// Index of next input data (in blocks)
	int32_t num_outputs;                        // Number of output indexes
	int32_t index_output[1];					// Index of next output data (in blocks)
	int32_t assume_full;						// Flag indicating if the buffer is assumed full if index_input == index_output
	uint8_t* mem;								// Pointer to the buffer memory
} rdsp_buffer;

RDSP_STATIC int32_t RdspBuffer_Create(rdsp_buffer* buffer, int32_t num_outputs, int32_t block_size, int32_t num_blocks);
RDSP_STATIC void RdspBuffer_Destroy(rdsp_buffer* buffer);
RDSP_STATIC void RdspBuffer_Reset(rdsp_buffer* buffer);
RDSP_STATIC int32_t RdspBuffer_NumBlocksAvailable(rdsp_buffer* buffer, int32_t output_id);
RDSP_STATIC void RdspBuffer_MakeRoomForNextInputBlock(rdsp_buffer* buffer, int32_t output_id);
RDSP_STATIC uint8_t* RdspBuffer_NextInputBlock(rdsp_buffer* buffer);
RDSP_STATIC void RdspBuffer_OffsetInputBlock(rdsp_buffer* buffer, int32_t index_offset);
RDSP_STATIC uint8_t* RdspBuffer_NextOutputBlock(rdsp_buffer* buffer, int32_t output_id, int32_t index_offset, int32_t increment_index);
RDSP_STATIC uint8_t* RdspBuffer_InputBlock(rdsp_buffer* buffer, int32_t index_offset);
RDSP_STATIC void RdspBuffer_WriteInputBlocks(rdsp_buffer* buffer, int32_t num_blocks_in, uint8_t* in_data);
RDSP_STATIC void RdspBuffer_ReadInputBlocks(rdsp_buffer* buffer, int32_t output_id, int32_t num_blocks_out, uint8_t* out_data);

#ifdef __cplusplus
}
#endif

#endif // RDSP_BUFFER_H
