/*
 * Copyright (c) 2021 by Retune DSP.
 * Copyright 2022-2024 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP
 * and may only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.
 * If you do not agree to be bound by the applicable license terms,
 * then you may not retain, install, activate or otherwise use the software.
 */

#include "RdspBuffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define RDSP_MIN(x,y) (x<y?x:y)

RDSP_STATIC int32_t RdspBuffer_Create(rdsp_buffer* buffer, int32_t num_outputs, int32_t block_size, int32_t num_blocks) {
    // Configure and allocate
    buffer->block_size = block_size;
    buffer->num_blocks = num_blocks;
    buffer->num_outputs = num_outputs;
    if (num_outputs > 1) {
        printf("Error: num_outputs is too large\n");
        return -1;
    }

    buffer->assume_full = 1;
    buffer->mem = malloc(buffer->block_size * buffer->num_blocks);
    if (buffer->mem == NULL ) {
        return -1;
    }
    RdspBuffer_Reset(buffer);
    return 0;
}

RDSP_STATIC void RdspBuffer_Destroy(rdsp_buffer* buffer) {
    // Deallocate memory
    free(buffer->mem);
}

RDSP_STATIC void RdspBuffer_Reset(rdsp_buffer* buffer) {
    // Reset buffer index states to zero
    buffer->index_input = 0;
    for (int32_t i = 0; i < buffer->num_outputs; i++)
        buffer->index_output[i] = 0;
}

RDSP_STATIC int32_t RdspBuffer_NumBlocksAvailable(rdsp_buffer* buffer, int32_t output_id) {
    // Return the number of blocks that are available in the buffer for outputting. If buffer->index_input == buffer->index_output, the buffer is assummed to be full if buffer->assume_full != 0
    int32_t num_blocks_in_buffer = buffer->index_input - buffer->index_output[output_id];
    if ((num_blocks_in_buffer < 0) || ((num_blocks_in_buffer == 0) && (buffer->assume_full != 0))) {
        num_blocks_in_buffer += buffer->num_blocks;
    }

    return num_blocks_in_buffer;
}

RDSP_STATIC void RdspBuffer_MakeRoomForNextInputBlock(rdsp_buffer* buffer, int32_t output_id) {
    // If needed, advance output index to make room for 1 input block. If output_id == -1, do this for all outputs

    int32_t start_output = output_id == -1 ? 0 : output_id;
    int32_t stop_output = output_id == -1 ? buffer->num_outputs : output_id + 1;
    for (int32_t i = start_output; i < stop_output; i++) {
        int32_t input_blocks_available = buffer->num_blocks - RdspBuffer_NumBlocksAvailable(buffer, i);
        if (input_blocks_available == 0) {
            RdspBuffer_NextOutputBlock(buffer, i, 0, 1); // Increment output
        }
    }
}

RDSP_STATIC uint8_t* RdspBuffer_NextInputBlock(rdsp_buffer* buffer) {
    // Returns pointer where to write next block. The buffer assumes the caller writes one block into the provided location.

    uint8_t* ptr = buffer->mem + buffer->index_input * buffer->block_size; // Pointer where to write

    // Increment index_input with wrap-around
    buffer->index_input++;
    if (buffer->index_input == buffer->num_blocks)
        buffer->index_input -= buffer->num_blocks;
    return ptr;
}

RDSP_STATIC void RdspBuffer_OffsetInputBlock(rdsp_buffer* buffer, int32_t index_offset) {
    // Adds index_offset to the input index

    int32_t index = buffer->index_input + index_offset;
    while (index < 0) {
        index += buffer->num_blocks;
    }

    while (index >= buffer->num_blocks) {
        index -= buffer->num_blocks;
    }

    buffer->index_input = index;
}

RDSP_STATIC uint8_t* RdspBuffer_NextOutputBlock(rdsp_buffer* buffer, int32_t output_id, int32_t index_offset, int32_t increment_index) {
    // Returns pointer where to read next block with a potential offset (index_offset). The buffer assumes the caller reads one block from the provided location.
    // If (increment_index != 0), index_output is incremented (can be disabled for filtering operations on the buffer)

    // Determine index to use
    int32_t index = buffer->index_output[output_id] + index_offset;
    while (index < 0) {
        index += buffer->num_blocks;
    }

    while (index >= buffer->num_blocks) {
        index -= buffer->num_blocks;
    }

    uint8_t* ptr = buffer->mem + index * buffer->block_size; // Pointer where to read
    if (increment_index) {
        buffer->index_output[output_id]++;
        if (buffer->index_output[output_id] == buffer->num_blocks)
            buffer->index_output[output_id] -= buffer->num_blocks;
    }
    return ptr;
}

RDSP_STATIC uint8_t* RdspBuffer_InputBlock(rdsp_buffer* buffer, int32_t index_offset) {
    // Returns pointer to input block with offset index_offset. No change to the internal state of the buffer is made.

    // Decrement index_input with wrap-around
    int32_t index_input_offset = buffer->index_input + index_offset;
    while (index_input_offset < 0) {
        index_input_offset += buffer->num_blocks;
    }

    while (index_input_offset >= buffer->num_blocks) {
        index_input_offset -= buffer->num_blocks;
    }

    uint8_t* ptr = buffer->mem + index_input_offset * buffer->block_size;
    return ptr;
}

RDSP_STATIC void RdspBuffer_WriteInputBlocks(rdsp_buffer* buffer, int32_t num_blocks_in, uint8_t* in_data) {
    // Writes num_blocks into the buffer reading from in_data and advances the index_input accordingly. This can be more efficient than writing one block at a time if the block size is small, e.g. for audio samples

    // The write operation may be split into two due to wrap around, here do the first write.
    int32_t num_blocks_1 = RDSP_MIN(buffer->num_blocks - buffer->index_input, num_blocks_in);
    memcpy(buffer->mem + buffer->index_input * buffer->block_size, in_data, num_blocks_1 * buffer->block_size);
    buffer->index_input += num_blocks_1;
    if (buffer->index_input >= buffer->num_blocks)
        buffer->index_input -= buffer->num_blocks;

    // If the write operation needs to be split into two due to wrap around, here do the second write.
    if (num_blocks_1 < num_blocks_in) {
        int32_t num_blocks_2 = num_blocks_in - num_blocks_1;
        memcpy(buffer->mem, in_data + num_blocks_1 * buffer->block_size, num_blocks_2 * buffer->block_size);
        buffer->index_input += num_blocks_2;
        if (buffer->index_input >= buffer->num_blocks)
            buffer->index_input -= buffer->num_blocks;
    }
}

RDSP_STATIC void RdspBuffer_ReadInputBlocks(rdsp_buffer* buffer, int32_t output_id, int32_t num_blocks_out, uint8_t* out_data) {
    // Reads num_blocks from the buffer writing to out_data and advances the index_output accordingly. This can be more efficient than reading one block at a time if the block size is small, e.g. for audio samples

    // The read operation may be split into two due to wrap around, here do the first read.
    int32_t num_blocks_1 = RDSP_MIN(buffer->num_blocks - buffer->index_output[output_id], num_blocks_out);
    memcpy(out_data, buffer->mem + buffer->index_output[output_id] * buffer->block_size, num_blocks_1 * buffer->block_size);
    buffer->index_output[output_id] += num_blocks_1;
    if (buffer->index_output[output_id] >= buffer->num_blocks)
        buffer->index_output[output_id] -= buffer->num_blocks;

    // If the read operation needs to be split into two due to wrap around, here do the second read.
    if (num_blocks_1 < num_blocks_out) {
        int32_t num_blocks_2 = num_blocks_out - num_blocks_1;
        memcpy(out_data + num_blocks_1 * buffer->block_size, buffer->mem, num_blocks_2 * buffer->block_size);
        buffer->index_output[output_id] += num_blocks_2;
        if (buffer->index_output[output_id] >= buffer->num_blocks)
            buffer->index_output[output_id] -= buffer->num_blocks;
    }
}
