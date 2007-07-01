// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
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

#ifdef HAVE_SWSCALE_H
#include <ffmpeg/swscale.h>
#endif

#include <boost/scoped_array.hpp>

namespace gnash {

embedVideoDecoderFfmpeg::embedVideoDecoderFfmpeg() :
	codec(NULL),
	cc(NULL),
	decodedFrame(NULL)
{
}

embedVideoDecoderFfmpeg::~embedVideoDecoderFfmpeg()
{
	if (cc) avcodec_close(cc);

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
		gnash::log_error("Unsupported embedded video format, it might help if you upgrade ffmpeg and recompile gnash");
		return;
	}

	if (codec == NULL) {
		gnash::log_error("Unsupported embedded video format, it might help if you upgrade ffmpeg and recompile gnash");
		return;
	}

	cc = avcodec_alloc_context();
	avcodec_open(cc, codec);
	cc->width = width;
	cc->height = height;

	// Determine required buffer size and allocate buffer
	if (outputFormat == YUV) {
		decodedFrame.reset(new image::yuv(width, height));
	} else if (outputFormat == RGB) {
		decodedFrame.reset(new image::rgb(width, height));
	}
}

uint8_t*
embedVideoDecoderFfmpeg::convertRGB24(AVCodecContext* srcCtx, AVFrame* srcFrame)
{
	int width = srcCtx->width, height = srcCtx->height;

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
#ifndef HAVE_SWSCALE_H
	img_convert(&picture, PIX_FMT_RGB24, (AVPicture*) srcFrame, srcCtx->pix_fmt,
		    width, height);
#else
	static SwsContext* context = NULL;

	if (!context) {
		context = sws_getContext(width, height, srcCtx->pix_fmt,
					 width, height, PIX_FMT_RGB24,
					 SWS_FAST_BILINEAR, NULL, NULL, NULL);
		if (!context) {
			delete [] buffer;
			return NULL;
		}
	}

	int rv = sws_scale(context, srcFrame->data, srcFrame->linesize, 0, 
			   width, picture.data, picture.linesize);
	if (rv == -1) {
		delete [] buffer;
		return NULL;
	}

#endif // HAVE_SWSCALE_H

	srcFrame->linesize[0] = picture.linesize[0];
	srcFrame->data[0] = picture.data[0];

	return buffer;
}

// gnash calls this when it wants you to decode the given videoframe
std::auto_ptr<image::image_base> 
embedVideoDecoderFfmpeg::decodeFrame(uint8_t* data, int size)
{

	std::auto_ptr<image::image_base> ret_image;

	if (outputFormat == YUV) {
		ret_image.reset(new image::yuv(width, height));
	} else if (outputFormat == RGB) {
		ret_image.reset(new image::rgb(width, height));
	} else {
		ret_image.reset(NULL);
		return ret_image;
	}

	// If there is nothing to decode in the new frame
	// we just return the lastest.
	if (data == NULL || codec == NULL || size == 0)
	{
		// If haven't decoded any frame yet, return
		// the null pointer.
		if ( ! decodedFrame.get() )
		{
			ret_image.reset(NULL);
			return ret_image;
		}

		ret_image->update(decodedFrame->m_data);
		return ret_image;
	}


	// Allocate a frame to store the decoded frame in
	AVFrame* frame = avcodec_alloc_frame();
	
	int got = 0;
	avcodec_decode_video(cc, frame, &got, data, size);

	// If the size of the video frame changed, adjust.
	// This could happen if the decoded video frame is
	// bigger than the defined SWF videoframe.
	if (static_cast<uint32_t>(cc->width) != width || static_cast<uint32_t>(cc->height) != height) {
		width = cc->width;
		height = cc->height;
		if (outputFormat == YUV) {
			decodedFrame.reset(new image::yuv(width, height));
			ret_image.reset(new image::yuv(width, height));
		} else if (outputFormat == RGB) {
			decodedFrame.reset(new image::rgb(width, height));
			ret_image.reset(new image::rgb(width, height));
		}
	}
	
	if (got) {
		boost::scoped_array<uint8_t> buffer;

		if (outputFormat == NONE) { // NullGui?
			av_free(frame);
			ret_image->update(decodedFrame->m_data);
			return ret_image;

		} else if (outputFormat == YUV && cc->pix_fmt != PIX_FMT_YUV420P) {
			//assert(0);	// TODO
			//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
			// Don't use depreceted img_convert, use sws_scale

		} else if (outputFormat == RGB && cc->pix_fmt != PIX_FMT_RGB24) {
			buffer.reset(convertRGB24(cc, frame));
		}

		if (outputFormat == YUV) {
			image::yuv* yuvframe = static_cast<image::yuv*>(decodedFrame.get());
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

			uint8_t* srcptr = frame->data[0];
			uint8_t* srcend = frame->data[0] + frame->linesize[0] * cc->height;
			uint8_t* dstptr = decodedFrame->m_data;
			unsigned int srcwidth = cc->width * 3;

			while (srcptr < srcend) {
				memcpy(dstptr, srcptr, srcwidth);
				srcptr += frame->linesize[0];
				dstptr += srcwidth;
			}

		}
	}

	av_free(frame);
	// If haven't decoded any frame yet, return
	// the null pointer.
	if ( ! decodedFrame.get() )
	{
		ret_image.reset(NULL);
		return ret_image;
	}
	ret_image->update(decodedFrame->m_data);
	return ret_image;
}

} // end of gnash namespace

#endif // USE_FFMPEG
