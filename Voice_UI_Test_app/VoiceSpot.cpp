/*----------------------------------------------------------------------------
	Copyright 2020-2021 NXP
	SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/

#include <cstring>
#include <AudioStream.h>

#include "RdspAppUtilities.h"
#include "SignalProcessor_VoiceSpot.h"
#include "SignalProcessor_VIT.h"
#include "rdsp_buffer.h"

std::string commandUsageStr =
    "Invalid input arguments!\n" \
    "Refer to the following command:\n" \
    "./voicespot <-notify>\n";

using namespace SignalProcessor;
using namespace AudioStreamWrapper;

static const char * captureOutputName = "default";
static const int captureOutputChannels = 1;
static snd_pcm_format_t format = SND_PCM_FORMAT_S32_LE;
static int period_size = 512;
static int buffer_size = period_size * 4;
static int rate = 16000;

//Define structure for circular buffer
typedef struct {
	char* samples;
	int32_t head, tail;
	size_t size, num_entries;
} queue;

void initQueue(queue* q, size_t samples_delay, size_t maxSize) {
	q->size = maxSize;
	q->samples = (char*)malloc(q->size);
	q->num_entries = 0;
	q->head = 0;
	q->tail = 0;

	//Initialize buffer to zeros
	memset(q->samples, 0, q->size);
}

void queueDestroy(queue* q) {
	free(q->samples);
}

void enqueue(queue* q, const char* samples_ref, size_t sizeBuff) {
	int32_t avail_bytes = q->size - q->tail;

	if (avail_bytes >= sizeBuff) {
		memcpy(q->samples + q->tail, (char*)samples_ref, sizeBuff);
		q->tail = (q->tail + sizeBuff) % q->size;
		q->num_entries += sizeBuff;
	} else {
		memcpy((q->samples) + q->tail, (char*)samples_ref, avail_bytes);
		q->tail = 0;
		int32_t rest_of_samples = sizeBuff - avail_bytes;
		memcpy((q->samples) + q->tail, (char*)samples_ref + avail_bytes, rest_of_samples);
		q->tail = (q->tail + rest_of_samples) % q->size;
		q->num_entries += sizeBuff;
	}
}

const char* dequeue(queue* q, size_t sizeBuff) {
	char* sample_buff = q->samples + q->head;
	q->head = (q->head + sizeBuff) % q->size;

	q->num_entries -= sizeBuff;

	return sample_buff;
}

int match(char *src, int src_len, queue* q, int *index) {

	int found = 0;
	int idx = 0;
	int i,j,a;
	int *dst = (int *)q->samples;
	int *src_tmp = (int *)src;
	int dst_len = q->num_entries;

	/* Samplesize is 4 bytes */
	for (i = 0; i < dst_len/4; i++) {
		for (j = 0; j < src_len/4; j++) {
			a = (j + idx + q->head/4) % (q->size/4);

			if (dst[a] != src_tmp[j])
				break;
		}

		if (j == src_len/4) {
			found = 1;
			*index = idx*4;
			break;
		}
		idx++;
	}

	return found;
}

//VoiceSpot's main
int main(int argc, char *argv[]) {
	ssize_t bytes_read;
	char buffer[VSLOUTBUFFERSIZE];
	int32_t iterations;
	int32_t enable_triggering;
	int32_t keyword_start_offset_samples;
	bool wakewordnotify = false;
	bool micSamplesReady = false;
	bool voice_cmd_detect = false;

	struct streamSettings captureOutputSettings =
	{
		captureOutputName,
		format,
		StreamType::eInterleaved,
		StreamDirection::eInput,
		captureOutputChannels,
		rate,
		buffer_size,
		period_size,
	};

	int sampleSize = snd_pcm_format_width(format) / 8;
	char* captureBuffer = (char *)calloc(period_size * captureOutputChannels, sampleSize);
	char* tmp_buf = (char*)malloc(VOICESEEKER_OUT_NHOP * sampleSize);
	float* float_buffer = (float*)malloc(sizeof(float) * period_size * captureOutputChannels);
	int tmp_pos = 0;
	int capture_pos = period_size;
	int err;
	int queue_size = VSLOUTBUFFERSIZE * 40;
	queue seekeroutput;
	int index;
	int framenum = 0;
	int frameoffset = 0;
	/* VIT uses a frame size of 160 samples */
	int vit_frame_size = 160;
	int vit_frame_count = 3 * 80;  /* 3 seconds*/
	rdsp_buffer vit_frame_buf;
	VIT_Handle_t VITHandle = PL_NULL;

	initQueue(&seekeroutput, 0, queue_size);

	if (argc == 2 && !strcmp(argv[1], "-notify"))
		wakewordnotify = true;
	else if (argc > 1) {
		std::cout << commandUsageStr << std::endl;
		exit(1);
	}

	RdspBuffer_Create(&vit_frame_buf, 1, sizeof(int16_t), 2 * VOICESEEKER_OUT_NHOP);
	vit_frame_buf.assume_full = 0;

	SignalProcessor_VoiceSpot VoiceSpot{};
	SignalProcessor_VIT VIT{};
	VITHandle = VIT.VIT_open_model();

	AudioStream captureOutput;
	captureOutput.open(captureOutputSettings);
	captureOutput.start();

	while (true) {
		mqd_t mqVslOut = VoiceSpot.get_mqVslout();
		mqd_t mqIter = VoiceSpot.get_mqIter();
		mqd_t mqTrigg = VoiceSpot.get_mqTrigg();
		mqd_t mqOffset = VoiceSpot.get_mqOffset();

		while (tmp_pos < VOICESEEKER_OUT_NHOP) {
			if (capture_pos == period_size) {
				err = captureOutput.readFrames(captureBuffer, period_size * captureOutputChannels * sampleSize);
				if (err < 0)
					throw AudioStreamException(snd_strerror(err), "readFrames", __FILE__, __LINE__, err);

				if (period_size == err)
					capture_pos = 0;
				else {
					usleep(10000);
				}
			}

			if (VOICESEEKER_OUT_NHOP - tmp_pos > period_size - capture_pos) {
				memcpy(tmp_buf + tmp_pos* sampleSize,
				       captureBuffer + capture_pos * sampleSize,
				       (period_size - capture_pos)* sampleSize);
				tmp_pos += period_size - capture_pos;
				capture_pos = period_size;
			} else {
				memcpy(tmp_buf + tmp_pos* sampleSize,
				       captureBuffer + capture_pos * sampleSize,
				       (VOICESEEKER_OUT_NHOP - tmp_pos)* sampleSize);
				capture_pos += VOICESEEKER_OUT_NHOP - tmp_pos;
				tmp_pos = VOICESEEKER_OUT_NHOP;
			}
		}

		rdsp_pcm_to_float(tmp_buf, &float_buffer, VOICESEEKER_OUT_NHOP, 1, sampleSize);
		tmp_pos = 0;

		bytes_read = mq_receive(mqVslOut, (char*)buffer, VSLOUTBUFFERSIZE, NULL);
		bytes_read = mq_receive(mqIter, (char*)&iterations, sizeof(int32_t), NULL);
		bytes_read = mq_receive(mqTrigg, (char*)&enable_triggering, sizeof(int32_t), NULL);
		framenum++;

		if (seekeroutput.num_entries >= (queue_size - VSLOUTBUFFERSIZE))
			dequeue(&seekeroutput, VSLOUTBUFFERSIZE);

		enqueue(&seekeroutput, buffer, VSLOUTBUFFERSIZE);

		/* 200 * 100 / 16000 = 1.25s, check the offset between voiceseeker and voicespot */
		if (framenum > 100) {
			if (seekeroutput.num_entries >= 2*VSLOUTBUFFERSIZE) {
				int found = 0;
				found = match(tmp_buf, VSLOUTBUFFERSIZE, &seekeroutput, &index);
				if (found) {
					dequeue(&seekeroutput, index + VSLOUTBUFFERSIZE);
					frameoffset = seekeroutput.num_entries;
					framenum = 0;
				}
			}
		}

		keyword_start_offset_samples = 0;
		if (!voice_cmd_detect){
			keyword_start_offset_samples = VoiceSpot.voiceSpot_process(buffer, wakewordnotify, iterations, enable_triggering);
			if (keyword_start_offset_samples){
				keyword_start_offset_samples += frameoffset/sampleSize;
				voice_cmd_detect = true;
				vit_frame_count = 3* 80;
				// Reset VIT frame buffer
				RdspBuffer_Reset(&vit_frame_buf);
			}
		}

		CHECK(0 <= mq_send(mqOffset, (char*)&keyword_start_offset_samples, sizeof(int32_t), 0));

		if (voice_cmd_detect) {
			/* Since the frame size is different between VoiceSpot and VIT, a frame buffer is needed for VIT input audio */
			int16_t vit_frame_buffer_lin[VOICESEEKER_OUT_NHOP];
			float* frame_buffer_float = (float *)buffer;
			rdsp_float_to_pcm((char *)vit_frame_buffer_lin, &frame_buffer_float, VOICESEEKER_OUT_NHOP, 1, 2);

			/* Write buffered VoiceSpot audio frame to VIT frame buffer */
			RdspBuffer_WriteInputBlocks(&vit_frame_buf, VOICESEEKER_OUT_NHOP, (uint8_t*)vit_frame_buffer_lin);
			bool command_found = false;
			int16_t cmd_id = 0;
			while (RdspBuffer_NumBlocksAvailable(&vit_frame_buf, 0) >= (int32_t)vit_frame_size) {
				/* Get vit_frame_buffer_lin samples for voice trigger */
				RdspBuffer_ReadInputBlocks(&vit_frame_buf, 0, vit_frame_size, (uint8_t*)vit_frame_buffer_lin);

				/* Run VIT processing */
				command_found = VIT.VIT_Process_Phase(VITHandle, vit_frame_buffer_lin, &cmd_id);
				/* VIT command recognition phase is finalized */
				/* command_found triggered when targeted Voice command is recognized or VIT detection timeout is reached */
				if (command_found) {
					/* Close VIT model */
					voice_cmd_detect = false;
					break;
				}
			}
			vit_frame_count--;
			if (!vit_frame_count)
				voice_cmd_detect = false;
		}
	}

	/* Close VIT model */
	VIT.VIT_close_model(VITHandle);
	RdspBuffer_Destroy(&vit_frame_buf);
	free(captureBuffer);
	free(tmp_buf);
	free(float_buffer);

	return 0;
}

