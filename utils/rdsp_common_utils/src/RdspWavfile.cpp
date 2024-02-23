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

#include "RdspWavfile.h"
#include <string.h>

RDSP_STATIC void read_bytes(FILE* fid, void* buf, uint32_t bufsize)
{
	if (!fread(buf, 1, bufsize, fid)) {
		printf("Not enough bytes in file\n");
		return;
	}
	fflush(fid);
}

RDSP_STATIC void skip_bytes(FILE* fid, uint32_t bufsize)
{
	if (fseek(fid, bufsize, SEEK_CUR) != 0) {
		printf("Not enough bytes in file\n");
		return;
	}
	fflush(fid);
}

RDSP_STATIC chunk_header_t read_chunk_header(FILE* fid, const char* Achunk_id)
{
	// Move to beginning of file
	rewind(fid);

	if (strncmp(Achunk_id, "RIFF", 4) != 0) {
		// Skip RIFF chunk
		fseek(fid, 12L, SEEK_SET);
	}

	chunk_header_t hdr = { 0 };
	while (!feof(fid)) {
		read_bytes(fid, &hdr, sizeof(chunk_header_t));
		if (strncmp(hdr.chunk_id, Achunk_id, 4) == 0) {
			// Return correct chunk
			return hdr;
		}
		else {
			// Skip to next chunk
			fseek(fid, hdr.chunk_size, SEEK_CUR);
			continue;
		}
	}

	// Chunk not found
	printf("Expected chunk '%s' not detected\n", Achunk_id);
	return hdr;
}

RDSP_STATIC rdsp_wav_file_t rdsp_wav_read_open(const char* Afilename) {

	FILE* fid = fopen(Afilename, "rb");
	rdsp_wav_file_t wav_file = { 0 };

	if (fid == NULL) {
		printf("Failed to open %s, errno = %d\n", Afilename, errno);
		return wav_file;
	}

	wav_file.fid = fid;

	// Read RIFF chunk
	chunk_header_t hdr = read_chunk_header(fid, "RIFF");
	wav_file.riff_header = hdr;

	char riff_type[4] = { 0 };
	read_bytes(fid, riff_type, sizeof(riff_type)); // should be WAVE
	if (strncmp(riff_type, "WAVE", 4) != 0) {
		printf("Expected type 'WAVE' not detected\n");
	}
	strncpy(wav_file.riff_type, riff_type, 4);

	// Read fmt chunk
	hdr = read_chunk_header(fid, "fmt ");
	if (hdr.chunk_size == 0) {
		printf("Invalid 'fmt' data size detected\n");
	}

	wav_file.fmt_header = hdr;
	read_bytes(fid, &wav_file.fmt, hdr.chunk_size);

	uint16_t* fmt = (uint16_t*)&wav_file.fmt; // Cast as pointer to index into union
	uint32_t sample_rate = fmt[2];
	if ((sample_rate != 16000) && (sample_rate != 48000)) {
		printf("Only input files with 16 or 48 kHz sample rates are supported.\n");
	}

	// Read data chunk
	hdr = read_chunk_header(fid, "data");
	strncpy(wav_file.data_id, hdr.chunk_id, 4);
	wav_file.data_size = hdr.chunk_size;

	wav_file.num_read = 0;
	return wav_file;
}

RDSP_STATIC void rdsp_wav_close(rdsp_wav_file_t* Awav_file) {
	FILE* fid = Awav_file->fid;

	if (fid != NULL) {
		// Write trailing pad byte if the data size is not even
		chunk_header_t hdr = read_chunk_header(fid, "data");
		if ((hdr.chunk_size % 2) != 0) {
			char null = 0;
			fwrite(&null, sizeof(null), 1, fid);
		}
		fclose(Awav_file->fid);
		Awav_file->fid = NULL;
	}
}

RDSP_STATIC size_t rdsp_wav_read_int32(int32_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {
	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	size_t num_read = 0;
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]
	uint32_t bytes_per_sample = bits_per_sample / 8;

	if (((Awav_file->num_read + Anum_samples) * num_channels * bytes_per_sample) >= Awav_file->data_size)
		return 0;

	if ((audio_format == WAVE_FORMAT_PCM) || (audio_format == WAVE_FORMAT_EXTENSIBLE)) {

		// 16 bit integer wav file
		if (bits_per_sample == 16) {
			int16_t* tmp_buf_int = new int16_t[Anum_samples * num_channels];
			num_read = fread(tmp_buf_int, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

			int32_t scale = 65536; // Convert to 32 bit integer
			for (uint16_t ich = 0; ich < num_channels; ich++) {
				int16_t* ptr = tmp_buf_int + ich;
				for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = *ptr * scale;
				}
			}
			delete[] tmp_buf_int;
		}

		// 32 bit integer wav file
		else if (bits_per_sample == 32) {
			int32_t* tmp_buf_int = new int32_t[Anum_samples * num_channels];
			num_read = fread(tmp_buf_int, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

			for (uint16_t ich = 0; ich < num_channels; ich++) {
				int32_t* ptr = tmp_buf_int + ich;
				for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = *ptr;
				}
			}
			delete[] tmp_buf_int;
		}
	}

	// 32 bit float wav file
	else if (audio_format == WAVE_FORMAT_IEEE_FLOAT) {
		if (bits_per_sample == 32) {
			float* tmp_buf_float = new float[Anum_samples * num_channels];
			num_read = fread(tmp_buf_float, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

			float scale = 32768.0f * 65536.0f; // Convert to 32 bit integer

			for (uint16_t ich = 0; ich < num_channels; ich++) {
				float* ptr = tmp_buf_float + ich;
				for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = (int32_t)(*ptr * scale);
				}
			}
			delete[] tmp_buf_float;
		}
	}
	else {
		printf("Only 16/32 bit integer wav files and 32 bit float wav files are supported.\n");
	}

	Awav_file->num_read += num_read;
	return num_read;
}

RDSP_STATIC size_t rdsp_wav_read_float(float** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {
	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	size_t num_read = 0;
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]
	uint32_t bytes_per_sample = bits_per_sample / 8;

	if (((Awav_file->num_read + Anum_samples) * num_channels * bytes_per_sample) >= Awav_file->data_size)
		return 0;

	if ((audio_format == WAVE_FORMAT_PCM) || (audio_format == WAVE_FORMAT_EXTENSIBLE)) {

		// 16 bit integer wav file
		if (bits_per_sample == 16) {
			int16_t* tmp_buf_int = new int16_t[Anum_samples * num_channels];
			num_read = fread(tmp_buf_int, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

			const float scale = 3.0517578125e-05f;
			for (uint16_t ich = 0; ich <num_channels; ich++) {
				int16_t* ptr = tmp_buf_int + ich;
				for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = (float(*ptr)) * scale;
				}
			}
			delete[] tmp_buf_int;
		}

		// 32 bit integer wav file
		else if (bits_per_sample == 32) {
			int32_t* tmp_buf_int = new int32_t[Anum_samples * num_channels];
			num_read = fread(tmp_buf_int, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

			const float scale = 4.656612873077393e-10f; // Convert to float
			for (uint16_t ich = 0; ich < num_channels; ich++) {
				int32_t* ptr = tmp_buf_int + ich;
				for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
					Abuffer[ich][isample] = (float(*ptr)) * scale;
				}
			}
			delete[] tmp_buf_int;
		}
	}

	// 32 bit float wav file
	else if ((bits_per_sample == 32) && (audio_format == WAVE_FORMAT_IEEE_FLOAT)) {
		float* tmp_buf_float = new float[Anum_samples * num_channels];
		num_read = fread(tmp_buf_float, num_channels * bytes_per_sample, Anum_samples, Awav_file->fid);

		for (uint16_t ich = 0; ich < num_channels; ich++) {
			float* ptr = tmp_buf_float + ich;
			for (uint32_t isample = 0; isample < num_read; isample++, ptr += num_channels) {
				Abuffer[ich][isample] = *ptr;
			}
		}
		delete[] tmp_buf_float;
	}
	else {
		printf("Only 16/32 bit integer wav files and 32 bit float wav files are supported.\n");
	}

	Awav_file->num_read += num_read;
	return num_read;
}

RDSP_STATIC rdsp_wav_file_t rdsp_wav_write_open(const char* Afilename, uint32_t Asample_rate, uint16_t Anum_channels, uint16_t Abit_depth, WaveFormat Aformat) {
	if ((Aformat != WAVE_FORMAT_PCM) && (Aformat != WAVE_FORMAT_IEEE_FLOAT)) {
		printf("Only 16/32 bit integer wav files and 32 bit float wav files are supported.\n");
	}

	rdsp_wav_file_t wav_file = { 0 };
	FILE* fid = fopen(Afilename, "wb+");
	if (fid) {
		wav_file.fid = fid;
		wav_file.num_read = 0;

		uint16_t bytes_per_sample = Abit_depth / 8;

		chunk_header_t* riff_header = &wav_file.riff_header;
		strncpy(riff_header->chunk_id, "RIFF", sizeof(riff_header->chunk_id));
		int32_t fmt_size;
		switch (Aformat) {
		case WAVE_FORMAT_PCM:
			fmt_size = 16;
			break;
		case WAVE_FORMAT_EXTENSIBLE:
			fmt_size = 40;
			break;
		case WAVE_FORMAT_IEEE_FLOAT:
		default:
			fmt_size = 18;
			break;
		}
		wav_file.data_size = 0; // Update when writing file. Equal to num_samples * num_channels * bits_per_sample / 8
		riff_header->chunk_size = 4 + (8 + fmt_size) + (8 + wav_file.data_size); // Update when writing file. 4 + (8 + fmt_size) + (8 + data_size)
		if (Aformat == WAVE_FORMAT_IEEE_FLOAT)
			riff_header->chunk_size += 12; //  Add size of fact chunk
		fwrite(riff_header, sizeof(chunk_header_t), 1, fid);

		strncpy(wav_file.riff_type, "WAVE", sizeof(wav_file.riff_type));
		fwrite(&wav_file.riff_type, sizeof(wav_file.riff_type), 1, fid);

		chunk_header_t fmt_header = { 0 };
		strncpy(fmt_header.chunk_id, "fmt ", sizeof(fmt_header.chunk_id));
		fmt_header.chunk_size = fmt_size;
		fwrite(&fmt_header, sizeof(chunk_header_t), 1, fid);
		wav_file.fmt_header = fmt_header;

		wave_fmt_t wave_fmt = { 0 };
		wave_fmt.pcm.audio_format = (uint16_t)Aformat;
		wave_fmt.pcm.num_channels = Anum_channels;
		wave_fmt.pcm.sample_rate = Asample_rate;
		wave_fmt.pcm.byte_rate = Asample_rate * Anum_channels * bytes_per_sample;
		wave_fmt.pcm.block_align = Anum_channels * bytes_per_sample;
		wave_fmt.pcm.bits_per_sample = Abit_depth;
		if (Aformat == WAVE_FORMAT_IEEE_FLOAT) {
			wave_fmt.non_pcm.size = 0;
		}
		fwrite(&wave_fmt, fmt_size, 1, fid);
		wav_file.fmt = wave_fmt;

		if (Aformat == WAVE_FORMAT_IEEE_FLOAT) {
			chunk_header_t fact_header = { 0 };
			strncpy(fact_header.chunk_id, "fact", sizeof(fact_header.chunk_id));
			fact_header.chunk_size = 4;
			fwrite(&fact_header, sizeof(chunk_header_t), 1, fid);
			int32_t samples_per_channel = 0;
			fwrite(&samples_per_channel, sizeof(int32_t), 1, fid); // Update when writing file
		}

		strncpy(wav_file.data_id, "data", sizeof(wav_file.data_id));
		fwrite(&wav_file.data_id, sizeof(wav_file.data_id), 1, fid);
		fwrite(&wav_file.data_size, sizeof(int32_t), 1, fid);

		fflush(fid);
	}
	else {
		printf("Failed to open %s, errno = %d\n", Afilename, errno);
	}
	return wav_file;
}

RDSP_STATIC void update_chunk_size(uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {

	FILE* fid = Awav_file->fid;

	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]
	uint32_t bytes_per_sample = bits_per_sample / 8;

	// Update chunk_size
	chunk_header_t hdr = read_chunk_header(fid, "RIFF");
	// Update chunk_size
	int32_t chunk_size = hdr.chunk_size + Anum_samples * num_channels * bytes_per_sample;
	// Move back to prepare for write
	fseek(fid, -4L, SEEK_CUR);
	// Write chunk_size
	size_t ret = fwrite(&chunk_size, sizeof(int32_t), 1, fid);
	fflush(fid);

	if (audio_format == WAVE_FORMAT_IEEE_FLOAT) {
		// Update fact_size
		hdr = read_chunk_header(fid, "fact");
		// Read old fact_size
		ret = fread(&chunk_size, sizeof(int32_t), 1, fid);
		// Update fact_size
		chunk_size += Anum_samples;
		// Move back to prepare for write
		fseek(fid, -4L, SEEK_CUR);
		// Write fact_size
		ret = fwrite(&chunk_size, sizeof(int32_t), 1, fid);
		fflush(fid);
	}

	// Update data_size
	hdr = read_chunk_header(fid, "data");
	// Update data_size
	chunk_size = hdr.chunk_size + Anum_samples * num_channels * bytes_per_sample;
	// Move back to prepare for write
	fseek(fid, -4L, SEEK_CUR);
	// Write data_size
	ret = fwrite(&chunk_size, sizeof(int32_t), 1, fid);
	fflush(fid);

	// Move to end of file
	fseek(fid, 0L, SEEK_END);
}


RDSP_STATIC size_t rdsp_wav_write_int16(int16_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {
	if (Anum_samples <= 0)
		return 0;

	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]

	size_t write_count = 0;
	if ((audio_format == WAVE_FORMAT_PCM) || (audio_format == WAVE_FORMAT_EXTENSIBLE)) {
		// Interleave samples
		int16_t* tmp_buf_int = new int16_t[Anum_samples * num_channels];
		int16_t* p = tmp_buf_int;
		for (uint32_t isample = 0; isample < Anum_samples; isample++) {
			for (uint16_t ich = 0; ich < num_channels; ich++)
				*p++ = Abuffer[ich][isample];
		}

		if (bits_per_sample == 16) {
			write_count = fwrite(tmp_buf_int, sizeof(int16_t), num_channels * Anum_samples, Awav_file->fid);
		}
		else {
			printf("Cannot write 16 bit integer buffer to %d bit integer wavfile.\n", bits_per_sample);
		}
		delete[] tmp_buf_int;
	}
	else {
		printf("Conversion to 16 bit integer wavfile is not yet supported.\n");
	}

	update_chunk_size(Anum_samples, Awav_file);

	return write_count;
}

RDSP_STATIC size_t rdsp_wav_write_int32(int32_t** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {
	if (Anum_samples <= 0)
		return 0;

	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]

	size_t write_count = 0;
	if ((audio_format == WAVE_FORMAT_PCM) || (audio_format == WAVE_FORMAT_EXTENSIBLE)) {
		// Interleave samples
		int32_t* tmp_buf_int = new int32_t[num_channels * Anum_samples];
		int32_t* p = tmp_buf_int;
		for (uint32_t isample = 0; isample < Anum_samples; isample++) {
			for (uint16_t ich = 0; ich < num_channels; ich++)
				*p++ = Abuffer[ich][isample];
		}
		if (bits_per_sample == 32) {
			write_count = fwrite(tmp_buf_int, sizeof(int32_t), num_channels * Anum_samples, Awav_file->fid);
		}
		else {
			printf("Cannot write 32 bit integer buffer to %d bit integer wavfile.\n", bits_per_sample);
		}
	delete[] tmp_buf_int;
	}
	else {
		printf("Conversion to 32 bit integer wavfile is not yet supported.\n");
	}
	update_chunk_size(Anum_samples, Awav_file);

	return write_count;
}

RDSP_STATIC size_t rdsp_wav_write_interleaved_int32(
    int32_t* interleaved_buffer,
    uint32_t Anum_samples,
    rdsp_wav_file_t* Awav_file) {

    uint32_t write_count = 0;
    uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
    uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]

    write_count = fwrite(
        interleaved_buffer,
        sizeof(int32_t),
        num_channels * Anum_samples,
        Awav_file->fid);

    update_chunk_size(Anum_samples, Awav_file);

    return write_count;
}


RDSP_STATIC size_t rdsp_wav_write_float(float** Abuffer, uint32_t Anum_samples, rdsp_wav_file_t* Awav_file) {
	if (Anum_samples <= 0)
		return 0;

	uint16_t* fmt = (uint16_t*)&Awav_file->fmt; // Cast as pointer to index into union
	uint16_t audio_format = fmt[0]; // audio_format is located at fmt[0]
	uint16_t num_channels = fmt[1]; // num_channels is located at fmt[1]
	uint32_t bits_per_sample = fmt[7]; // bits_per_sample is located at fmt[7]

	size_t write_count = 0;
	if ((audio_format == WAVE_FORMAT_PCM) || (audio_format == WAVE_FORMAT_EXTENSIBLE)) {
		if (bits_per_sample == 32) {
			const float scale = 2147483648.0f; // 2^31, buffer is 32 bit floating-point
			int32_t* tmp_buf_int = new int32_t[num_channels * Anum_samples];
			int32_t* ptr = tmp_buf_int;
			for (uint32_t isample = 0; isample < Anum_samples; isample++) {
				for (uint16_t ich = 0; ich < num_channels; ich++, ptr++) {
					*ptr = (int32_t)(Abuffer[ich][isample] * scale);
				}
			}
			write_count = fwrite(tmp_buf_int, sizeof(tmp_buf_int[0]), num_channels * Anum_samples, Awav_file->fid);
			delete[] tmp_buf_int;
		}
		else if (bits_per_sample == 16) {
			const float scale = 32768.0f; // 2^15, buffer is 32 bit floating-point
			int16_t* tmp_buf_int = new int16_t[num_channels * Anum_samples];
			int16_t* ptr = tmp_buf_int;
			for (uint32_t isample = 0; isample < Anum_samples; isample++) {
				for (uint16_t ich = 0; ich < num_channels; ich++, ptr++) {
					*ptr = (int16_t)(Abuffer[ich][isample] * scale);
				}
			}
			write_count = fwrite(tmp_buf_int, sizeof(tmp_buf_int[0]), num_channels * Anum_samples, Awav_file->fid);
			delete[] tmp_buf_int;
		}
	}
	else if (audio_format == WAVE_FORMAT_IEEE_FLOAT)
	{
		if (bits_per_sample == 32) {
			float* tmp_buf_float = new float[num_channels * Anum_samples];
			float* ptr = tmp_buf_float;
			for (uint32_t isample = 0; isample < Anum_samples; isample++) {
				for (uint16_t ich = 0; ich < num_channels; ich++, ptr++) {
					*ptr = Abuffer[ich][isample];
				}
			}
			write_count = fwrite(tmp_buf_float, sizeof(tmp_buf_float[0]), num_channels * Anum_samples, Awav_file->fid);
			delete[] tmp_buf_float;
		}
	}
	else {
		printf("Audio format %i not supported\n", audio_format);
	}

	update_chunk_size(Anum_samples, Awav_file);

	return write_count;
}
