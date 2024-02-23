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

#ifndef RDSP_WAVFILE_H
#define RDSP_WAVFILE_H

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RDSP_STATIC
#define RDSP_STATIC
#endif

// WAVE PCM soundfile format, see e.g. http://soundfile.sapp.org/doc/WaveFormat/ and http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/WAVE/WAVE.html

enum WaveFormat {
	WAVE_FORMAT_PCM = 0x0001,
	WAVE_FORMAT_IEEE_FLOAT = 0x0003,
	WAVE_FORMAT_ALAW = 0x0006,
	WAVE_FORMAT_MULAW = 0x0007,
	WAVE_FORMAT_EXTENSIBLE = 0xFFFE
};

typedef struct chunk_header_s {
	char chunk_id[4];
	uint32_t chunk_size;
} chunk_header_t;

typedef struct wave_fmt_pcm_s {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
} wave_fmt_pcm_t;

typedef struct wave_fmt_non_pcm_s {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint16_t size;
} wave_fmt_non_pcm_t;

typedef struct wave_fmt_ext_s {
	uint16_t audio_format;
	uint16_t num_channels;
	uint32_t sample_rate;
	uint32_t byte_rate;
	uint16_t block_align;
	uint16_t bits_per_sample;
	uint16_t size;
	uint16_t valid_bits;
	uint32_t channel_mask;
	uint8_t subformat[16]; // 16 bytes
} wave_fmt_ext_t;

typedef union wave_fmt_u {
	wave_fmt_pcm_s pcm;
	wave_fmt_non_pcm_s non_pcm;
	wave_fmt_ext_s ext;
} wave_fmt_t;

typedef struct rdsp_wav_file_s {
	FILE* fid;
	chunk_header_t riff_header;
	char riff_type[4];
	chunk_header_t fmt_header;
	wave_fmt_t fmt;
	char data_id[4];
	uint32_t data_size; // data_size denotes the number of samples
	size_t num_read;
} rdsp_wav_file_t;

RDSP_STATIC void read_bytes(FILE* fid, void* buf, uint32_t bufsize);
RDSP_STATIC void skip_bytes(FILE* fid, uint32_t bufsize);
RDSP_STATIC chunk_header_t read_chunk_header(FILE* fid, char* Achunk_id);

RDSP_STATIC rdsp_wav_file_t rdsp_wav_read_open(const char* Afilename);
RDSP_STATIC size_t rdsp_wav_read_int32(int32_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file);
RDSP_STATIC size_t rdsp_wav_read_float(float** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file);

RDSP_STATIC rdsp_wav_file_t rdsp_wav_write_open(const char* Afilename, uint32_t Asample_rate, uint16_t Anum_channels, uint16_t Abit_depth, WaveFormat Aformat);
RDSP_STATIC size_t rdsp_wav_write_int16(int16_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file);
RDSP_STATIC size_t rdsp_wav_write_int32(int32_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file);
RDSP_STATIC size_t rdsp_wav_write_interleaved_int32(
    int32_t* interleaved_buffer,
    uint32_t Anum_samples,
    rdsp_wav_file_t* Awav_file);
RDSP_STATIC size_t rdsp_wav_write_float(float** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file);

RDSP_STATIC void rdsp_wav_close(rdsp_wav_file_t* Awav_file);

#ifdef __cplusplus
}
#endif

#endif /* RDSP_WAVFILE_H */
