/****************************************
 *
 * Copyright (c) 2025 Fahim Faisal
 *
 ****************************************/

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
#include "input_media.h"

uint8_t *data[4] = {NULL};
int width = 0;
int height = 0;
int file_ended = FALSE;
int media_open = FALSE;

static int response = 0;
static AVFrame *frame = NULL;
static AVPacket *packet = NULL;
static int packet_processed = TRUE;
static AVFormatContext *fmt_ctx = NULL;
static AVCodecContext *vid_codec_ctx = NULL;
static int video_stream_index = -1;
static int data_linesize[4] = {0, 0, 0, 0};
static struct SwsContext *sws_scaler_ctx = NULL;

int get_input_media_handlers(char *filename, int scr_width, int scr_height) {

	// Open input file and populate context
	if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) != 0) {
		fprintf(stderr, "Error opening input file: %s.\n", filename);
		return FALSE;
	}

	// Populate fmt_ctx->streams
	if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find information.\n");
		avformat_close_input(&fmt_ctx);
		return FALSE;
	}

	AVCodecParameters *codec_params = NULL;
	const AVCodec *codec = NULL;

	// Loop through all the streams; find video stream and its codec
	for (int i = 0; i < fmt_ctx->nb_streams; i++) {
		AVCodecParameters *local_codec_params = fmt_ctx->streams[i]->codecpar;

		const AVCodec *local_codec = avcodec_find_decoder(local_codec_params->codec_id);
		if (local_codec == NULL)
			continue;

		if (local_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			if (video_stream_index == -1) {
				video_stream_index = i;
				codec = local_codec;
				codec_params = local_codec_params;
				break;
			}
		}
	}

	// Handle no video stream or video stream with unsupported codec
	if (video_stream_index == -1) {
		fprintf(stderr, "Could not find video stream.\n");
		avformat_close_input(&fmt_ctx);
		return FALSE;
	}

	// Allocate codec context, populate context, and open the context 
	vid_codec_ctx = avcodec_alloc_context3(codec);
	if (!vid_codec_ctx) {
		fprintf(stderr, "Could not allocate codec context.\n");
		avformat_close_input(&fmt_ctx);
		return FALSE;
	}

	if (avcodec_parameters_to_context(vid_codec_ctx, codec_params) < 0) {
		fprintf(stderr, "Could not copy codec parameters to context.\n");
		avformat_close_input(&fmt_ctx);
		avcodec_free_context(&vid_codec_ctx);
		return FALSE;
	}

	if (avcodec_open2(vid_codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec context.\n");
		avformat_close_input(&fmt_ctx);
		avcodec_free_context(&vid_codec_ctx);
		return FALSE;
	}

	// Allocate decoded data buffer
	int full_width = vid_codec_ctx->width;
	int full_height = vid_codec_ctx->height;
	int adj_height = full_height * ((float) scr_width / full_width);
	width = scr_width;
	height = adj_height;

	data[0] = (uint8_t *) malloc(width * height * 3 * sizeof(uint8_t));
	data_linesize[0] = width * 3;

	// Allocate Scaler Context
	sws_scaler_ctx = sws_getContext(full_width, full_height, vid_codec_ctx->pix_fmt,
				   					width, height, AV_PIX_FMT_RGB24,
									SWS_BILINEAR, NULL, NULL, NULL);
	if (!sws_scaler_ctx) {
		fprintf(stderr, "Couldn't initialize SWScaler.\n");
		return FALSE;
	}

	// Allocate AVPacket and AVFrame
	packet = av_packet_alloc();
	if (!packet) {
		fprintf(stderr, "Couldn't allocate AVPacket.\n");
		avformat_close_input(&fmt_ctx);
		avcodec_free_context(&vid_codec_ctx);
		sws_freeContext(sws_scaler_ctx);
		return FALSE;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Couldn't allocate AVFrame.\n");
		avformat_close_input(&fmt_ctx);
		avcodec_free_context(&vid_codec_ctx);
		sws_freeContext(sws_scaler_ctx);
		av_packet_free(&packet);
		return FALSE;
	}

	media_open = TRUE;
	return TRUE;
}

int decode_next_frame() {
	if (file_ended)
		return FALSE;

	while (TRUE) {

		// If the previous packet was completely processed, read a new one
		if (packet_processed) {

			// Read new packet from codec context
			if (av_read_frame(fmt_ctx, packet) < 0)
				break;

			// If it's a video packet, send it to the codec context
			if (packet->stream_index == video_stream_index) {
				response = avcodec_send_packet(vid_codec_ctx, packet);
				if (response < 0) {
					fprintf(stderr, "Error while sending packet to the decoder: %s\n", av_err2str(response));
					return FALSE;
				}
				
				packet_processed = FALSE;
			} else {
			// Else skip over rest of outer while to read new packet
				av_packet_unref(packet);
				continue;
			}
		}

		// Decode loop
		while (response >= 0) {
			// Receive frame in decode loop
			response = avcodec_receive_frame(vid_codec_ctx, frame);
			if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				// New input is needed or we reached end of file 
				packet_processed = TRUE;
				av_packet_unref(packet);
				break;
			} else if (response < 0) {
				fprintf(stderr, "Error while receiving packet from the decoder: %s\n", av_err2str(response));
				return FALSE;
			}

			// Here we have video frame
			sws_scale(sws_scaler_ctx, (const uint8_t * const *) frame->data, frame->linesize, 0, frame->height, data, data_linesize);
			return TRUE;
		}
	}	

	close_input_media();
	file_ended = TRUE;
	return FALSE;
}

void close_input_media() {
	avformat_close_input(&fmt_ctx);
	avcodec_free_context(&vid_codec_ctx);
	sws_freeContext(sws_scaler_ctx);
	av_packet_free(&packet);
	av_frame_free(&frame);
	free(data[0]);
	media_open = FALSE;
}
