// VideoDecoderFfmpeg.h: Video decoding using the FFMPEG library.
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

//  $Id:

#ifndef __VIDEODECODERFFMPEG_H__
#define __VIDEODECODERFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "VideoDecoder.h"

extern "C" {
#include <ffmpeg/avcodec.h>
}


namespace gnash {


class VideoDecoderFfmpeg : public VideoDecoder {
	
public:
	VideoDecoderFfmpeg();
	~VideoDecoderFfmpeg();

	bool setup(VideoInfo* info);

	bool setup(
		int /*width*/,
		int /*height*/,
		int /*deblocking*/,
		bool /*smoothing*/,
		videoCodecType /*format*/, // should this argument be of codecType type ?
		int /*outputFormat*/);

	uint8_t* decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize);

	std::auto_ptr<image::image_base> decodeToImage(uint8_t* /*input*/, uint32_t /*inputSize*/);
private:

	uint8_t* convertRGB24(AVCodecContext* srcCtx, AVFrame* srcFrame);

	AVCodec* _videoCodec;
	AVCodecContext* _videoCodecCtx;

};
	
} // gnash namespace

#endif // __VIDEODECODERFFMPEG_H__
