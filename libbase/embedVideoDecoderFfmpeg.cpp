// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// $Id: embedVideoDecoderFfmpeg.cpp,v 1.3 2007/02/09 16:40:42 tgc Exp $

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_FFMPEG

#include "embedVideoDecoderFfmpeg.h"

embedVideoDecoderFfmpeg::embedVideoDecoderFfmpeg()
{
}

void
embedVideoDecoderFfmpeg::createDecoder(int widthi, int heighti, int deblockingi, bool smoothingi, int formati, int outputFormati)
{
	// Init the avdecoder-decoder
	avcodec_init();
	avcodec_register_all();

	// Save video attributes
	width = widthi;
	height = heighti;
	deblocking = deblockingi;
	smoothing = smoothingi;
	format = formati;
	outputFormat = outputFormati;

	// Find the decoder and init the parser
	if (format == CODEC_H263) {
		codec = avcodec_find_decoder(CODEC_ID_FLV1);
	} else if (format == CODEC_VP6) {
		codec = avcodec_find_decoder(CODEC_ID_VP6F);
	} else if (format == CODEC_SCREENVIDEO) {
		codec = avcodec_find_decoder(CODEC_ID_FLASHSV);
	} else {
		return;
	}

	if (codec == NULL) {
		return;
	}

	cc = avcodec_alloc_context();
	avcodec_open(cc, codec);
	cc->width = width;
	cc->height = height;

	// Determine required buffer size and allocate buffer
	if (outputFormat == YUV) {
		decodedFrame = new image::yuv(width, height);
	} else if (outputFormat == RGB) {
		decodedFrame = new image::rgb(width, height);
	}
}

embedVideoDecoderFfmpeg::~embedVideoDecoderFfmpeg()
{
	if (decodedFrame) delete decodedFrame;

	if (cc) avcodec_close(cc);

}
// gnash calls this when it wants you to decode the given videoframe
image::image_base*
embedVideoDecoderFfmpeg::decodeFrame(uint8_t* data, int size)
{

	if (data == NULL) return decodedFrame;

	// Allocate a frame to store the decoded frame in
	AVFrame* frame = avcodec_alloc_frame();
	
	int got = 0;
	avcodec_decode_video(cc, frame, &got, data, size);

	if (got) {
		uint8_t *buffer = NULL;

		if (outputFormat == NONE) { // NullGui?
			av_free(frame);
			return NULL;

		} else if (outputFormat == YUV && cc->pix_fmt != PIX_FMT_YUV420P) {
			//assert(0);	// TODO
			//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

		} else if (outputFormat == RGB && cc->pix_fmt != PIX_FMT_RGB24) {
			AVFrame* frameRGB = avcodec_alloc_frame();
			unsigned int numBytes = avpicture_get_size(PIX_FMT_RGB24, cc->width, cc->height);
			buffer = new uint8_t[numBytes];
			avpicture_fill((AVPicture *)frameRGB, buffer, PIX_FMT_RGB24, cc->width, cc->height);
			img_convert((AVPicture*) frameRGB, PIX_FMT_RGB24, (AVPicture*) frame, cc->pix_fmt, cc->width, cc->height);
			av_free(frame);
			frame = frameRGB;
		}

		if (outputFormat == YUV) {
			image::yuv* yuvframe = static_cast<image::yuv*>(decodedFrame);
			int copied = 0;
			uint8_t* ptr = yuvframe->m_data;
			for (int i = 0; i < 3 ; i++)
			{
				int shift = (i == 0 ? 0 : 1);
				uint8_t* yuv_factor = frame->data[i];
				int h = cc->height >> shift;
				int w = cc->width >> shift;
				for (int j = 0; j < h; j++)
				{
					copied += w;
					//assert(copied <= yuvframe->size());
					memcpy(ptr, yuv_factor, w);
					yuv_factor += frame->linesize[i];
					ptr += w;
				}
			}
			yuvframe->m_size = copied;
		} else if (outputFormat == RGB) {
			for(int line = 0; line < cc->height; line++)
			{
				for(int byte = 0; byte < (cc->width*3); byte++)
				{
					decodedFrame->m_data[byte + (line*cc->width*3)] = (unsigned char) *(frame->data[0]+(line*frame->linesize[0])+byte);
				}
			}
		}
		delete [] buffer;
	} else {
		return decodedFrame;
	}

	return decodedFrame;
}



#endif // USE_FFMPEG
