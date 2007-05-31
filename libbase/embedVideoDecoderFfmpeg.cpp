// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_FFMPEG

#include <cstring>

#include "embedVideoDecoderFfmpeg.h"
#include <ffmpeg/swscale.h>
#include <boost/scoped_array.hpp>

embedVideoDecoderFfmpeg::embedVideoDecoderFfmpeg() :
	codec(NULL),
	cc(NULL),
	decodedFrame(NULL)
{
}

embedVideoDecoderFfmpeg::~embedVideoDecoderFfmpeg()
{
	if (cc) avcodec_close(cc);

	if(decodedFrame) delete decodedFrame;

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
#ifdef FFMPEG_VP6
	} else if (format == CODEC_VP6) {
		codec = avcodec_find_decoder(CODEC_ID_VP6F);
#endif
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

// FIXME: This function (and a lot of other code in this file) is
//        duplicated in NetStreamFfmpeg.

/// Convert the given srcFrame to RGB24 pixel format.
//
/// @param srcCtx The codec context with which srcFrame is associated.
/// @param srcFrame The source frame to convert. The data and linesize members
///                 of srcFrame will be changed to match the conversion.
/// @return A pointer to the newly allocated and freshly converted video data.
///         The caller owns the pointer! It must be freed with delete [] when
///	    the frame has been processed.
uint8_t*
convertRGB24(AVCodecContext* srcCtx, AVFrame* srcFrame)
{
	static SwsContext* context = NULL;
	int width = srcCtx->width, height = srcCtx->height;

	if (!context) {
		context = sws_getContext(width, height, srcCtx->pix_fmt,
					 width, height, PIX_FMT_RGB24,
					 SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (!context) {
			return NULL;
		}
	}

	int bufsize = avpicture_get_size(PIX_FMT_RGB24, width, height);
	if (bufsize == -1) {
		return NULL;
	}

	uint8_t* buffer = new uint8_t[bufsize];
	if (!buffer) {
		return NULL;
	}

	AVPicture picture;

	avpicture_fill(&picture, buffer, PIX_FMT_RGB24, width, height);


	int rv = sws_scale(context, srcFrame->data, srcFrame->linesize, 0, 
			   width, picture.data, picture.linesize);
	if (rv == -1) {
		delete [] buffer;
		return NULL;
	}

	srcFrame->linesize[0] = picture.linesize[0];
	srcFrame->data[0] = picture.data[0];

	return buffer;
}

// gnash calls this when it wants you to decode the given videoframe
image::image_base*
embedVideoDecoderFfmpeg::decodeFrame(uint8_t* data, int size)
{

	if (data == NULL || codec == NULL) return decodedFrame;

	// Allocate a frame to store the decoded frame in
	AVFrame* frame = avcodec_alloc_frame();
	
	int got = 0;
	avcodec_decode_video(cc, frame, &got, data, size);

	if (got) {
		boost::scoped_array<uint8_t> buffer;

		if (outputFormat == NONE) { // NullGui?
			av_free(frame);
			return NULL;

		} else if (outputFormat == YUV && cc->pix_fmt != PIX_FMT_YUV420P) {
			//assert(0);	// TODO
			//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);

		} else if (outputFormat == RGB && cc->pix_fmt != PIX_FMT_RGB24) {
			buffer.reset(convertRGB24(cc, frame));
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
	} else {
		return decodedFrame;
	}

	return decodedFrame;
}



#endif // USE_FFMPEG
