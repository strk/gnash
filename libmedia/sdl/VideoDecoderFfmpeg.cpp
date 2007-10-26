// VideoDecoderFfmpeg.cpp: Video decoding using the FFMPEG library.
// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
//

// $Id: VideoDecoderFfmpeg.cpp,v 1.7 2007/10/26 18:43:36 tgc Exp $

#include "VideoDecoderFfmpeg.h"

#ifdef HAVE_SWSCALE_H
extern "C" {
#include <ffmpeg/swscale.h>
}
#endif
#include <boost/scoped_array.hpp>

namespace gnash {
	
VideoDecoderFfmpeg::VideoDecoderFfmpeg ()
	:
	_videoCodec(NULL),
	_videoCodecCtx(NULL)
{}

VideoDecoderFfmpeg::~VideoDecoderFfmpeg()
{
	if (_videoCodec) avcodec_close(_videoCodecCtx);
}

bool VideoDecoderFfmpeg::setup(
		int width,
		int height,
		int /*deblocking*/,
		bool /*smoothing*/,
		videoCodecType format, // should this argument be of codecType type ?
		int /*outputFormat*/)
{
	// Init the avdecoder-decoder
	avcodec_init();
	avcodec_register_all();// change this to only register need codec?

	enum CodecID codec_id;

	// Find the decoder and init the parser
	switch(format) {
		case VIDEO_CODEC_H263:
			codec_id = CODEC_ID_FLV1;
			break;
#ifdef FFMPEG_VP6
		case VIDEO_CODEC_VP6:
			codec_id = CODEC_ID_VP6F;
			break;
#endif
		case VIDEO_CODEC_SCREENVIDEO:
			codec_id = CODEC_ID_FLASHSV;
			break;
		default:
			log_error(_("Unsupported video codec %d"), static_cast<int>(format));
			return false;
	}

	_videoCodec = avcodec_find_decoder(static_cast<CodecID>(codec_id));

	if (!_videoCodec) {
		log_error(_("libavcodec can't decode the current video format"));
		return false;
	}

	_videoCodecCtx = avcodec_alloc_context();
	if (!_videoCodecCtx) {
		log_error(_("libavcodec couldn't allocate context"));
		return false;
	}

	int ret = avcodec_open(_videoCodecCtx, _videoCodec);
	if (ret < 0) {
		avcodec_close(_videoCodecCtx);
		log_error(_("libavcodec failed to initialize codec"));
		return false;
	}
	_videoCodecCtx->width = width;
	_videoCodecCtx->height = height;

	assert(_videoCodecCtx->width > 0);
	assert(_videoCodecCtx->height > 0);
	return true;
}

bool VideoDecoderFfmpeg::setup(VideoInfo* info)
{
	// Init the avdecoder-decoder
	avcodec_init();
	avcodec_register_all();// change this to only register need codec?

	if (info->type == FLASH) {
		enum CodecID codec_id;

		// Find the decoder and init the parser
		switch(info->codec) {
			case VIDEO_CODEC_H263:
				codec_id = CODEC_ID_FLV1;
				break;
#ifdef FFMPEG_VP6
			case VIDEO_CODEC_VP6:
				codec_id = CODEC_ID_VP6F;
				break;
#endif
			case VIDEO_CODEC_SCREENVIDEO:
				codec_id = CODEC_ID_FLASHSV;
				break;
			default:
				log_error(_("Unsupported video codec %d"), static_cast<int>(info->codec));
				return false;
		}
		_videoCodec = avcodec_find_decoder(static_cast<CodecID>(codec_id));
	} else if (info->type == FFMPEG) {
		_videoCodec = avcodec_find_decoder(static_cast<CodecID>(info->codec));
	} else {
		//log_error("Video codecType unknown: %d, %d, %d", info->type, FLASH, FFMPEG);
		return false;
	}

	if (!_videoCodec) {
		log_error(_("libavcodec can't decode the current video format"));
		return false;
	}

	// Reuse the videoCodecCtx from the ffmpeg parser if exists/possible
	if (info->videoCodecCtx) {
		log_debug("re-using the parsers videoCodecCtx");
		_videoCodecCtx = info->videoCodecCtx;
	} else {
		_videoCodecCtx = avcodec_alloc_context();
	}

	if (!_videoCodecCtx) {
		log_error(_("libavcodec couldn't allocate context"));
		return false;
	}

	int ret = avcodec_open(_videoCodecCtx, _videoCodec);
	if (ret < 0) {
		avcodec_close(_videoCodecCtx);
		log_error(_("libavcodec failed to initialize codec"));
		return false;
	}

	return true;
}

uint8_t*
VideoDecoderFfmpeg::convertRGB24(AVCodecContext* srcCtx, AVFrame* srcFrame)
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

uint8_t* VideoDecoderFfmpeg::decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize)
{
	// Allocate a frame to store the decoded frame in
	AVFrame* frame = avcodec_alloc_frame();
	if ( ! frame )
	{
		log_error(_("Out of memory while allocating avcodec frame"));
		throw std::bad_alloc();
	}

	int got = 0;
	avcodec_decode_video(_videoCodecCtx, frame, &got, input, inputSize);
	if (got) {
		boost::scoped_array<uint8_t> buffer;

		uint8_t* decodedData = new uint8_t[_videoCodecCtx->width * _videoCodecCtx->height * 3];
		buffer.reset(convertRGB24(_videoCodecCtx, frame));

		// Copy the data to the buffer in the correct RGB format
		uint8_t* srcptr = frame->data[0];
		uint8_t* srcend = frame->data[0] + frame->linesize[0] * _videoCodecCtx->height;
		uint8_t* dstptr = decodedData;
		unsigned int srcwidth = _videoCodecCtx->width * 3;

		outputSize = 0;

		while (srcptr < srcend) {
			memcpy(dstptr, srcptr, srcwidth);
			srcptr += frame->linesize[0];
			dstptr += srcwidth;
			outputSize += srcwidth;
		}

		av_free(frame);
		return decodedData;

/*		if (_videoFrameFormat == NONE) { // NullGui?
			return;

		} else if (_videoFrameFormat == YUV && _videoCodecCtx->pix_fmt != PIX_FMT_YUV420P) {
			assert(0);	// TODO
			//img_convert((AVPicture*) pFrameYUV, PIX_FMT_YUV420P, (AVPicture*) pFrame, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
			// Don't use depreceted img_convert, use sws_scale

		} else if (_videoFrameFormat == RGB && _videoCodecCtx->pix_fmt != PIX_FMT_RGB24) {
			buffer.reset(convertRGB24(_videoCodecCtx, frame));
		}

		raw_mediadata_t* video = new raw_mediadata_t;
		if (_videoFrameFormat == YUV) {
			assert(0); // See image.cpp to see what yuv size is
			//video->m_data = new uint8_t[static_cast<image::yuv*>(m_imageframe)->size()];
		} else if (_videoFrameFormat == RGB) {
			video->m_data = new uint8_t[_videoCodecCtx->width * _videoCodecCtx->height * 3];
		//}

		video->m_ptr = video->m_data;
		video->m_stream_index = _videoIndex;
		video->m_pts = 0;

		video->m_pts = static_cast<uint32_t>((as_double(_videoStream->time_base) * packet->dts) * 1000.0);


		if (_videoFrameFormat == YUV) {
			//image::yuv* yuvframe = static_cast<image::yuv*>(_imageframe);
			int copied = 0;
			uint8_t* ptr = video->m_data;
			for (int i = 0; i < 3 ; i++)
			{
				int shift = (i == 0 ? 0 : 1);
				uint8_t* yuv_factor = _frame->data[i];
				int h = _videoCodecCtx->height >> shift;
				int w = _videoCodecCtx->width >> shift;
				for (int j = 0; j < h; j++)
				{
					copied += w;
					//assert(copied <= yuvframe->size());
					memcpy(ptr, yuv_factor, w);
					yuv_factor += _frame->linesize[i];
					ptr += w;
				}
			}
			video->m_size = copied;
		} else if (_videoFrameFormat == RGB) {

			uint8_t* srcptr = _frame->data[0];
			uint8_t* srcend = _frame->data[0] + _frame->linesize[0] * _videoCodecCtx->height;
			uint8_t* dstptr = video->m_data;
			unsigned int srcwidth = _videoCodecCtx->width * 3;

			video->m_size = 0;

			while (srcptr < srcend) {
				memcpy(dstptr, srcptr, srcwidth);
				srcptr += _frame->linesize[0];
				dstptr += srcwidth;
				video->m_size += srcwidth;
			}

		}*/
	} else {
		log_error("Decoding of a video frame failed");
		av_free(frame);
		return NULL;
	}
}

std::auto_ptr<image::image_base>
VideoDecoderFfmpeg::decodeToImage(uint8_t* input, uint32_t inputSize)
{
	uint32_t outputSize = 0;
	uint8_t* decodedData = decode(input, inputSize, outputSize);

	if (!decodedData || outputSize == 0) {
		return std::auto_ptr<image::image_base>(NULL);
	}

	std::auto_ptr<image::image_base> ret(new image::rgb(_videoCodecCtx->width, _videoCodecCtx->height));
	ret->update(decodedData);
	delete [] decodedData;
	return ret;
	
}

}
