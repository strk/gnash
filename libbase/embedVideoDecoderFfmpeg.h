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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// $Id: embedVideoDecoderFfmpeg.h,v 1.1 2007/02/08 13:25:41 tgc Exp $

#ifndef __EMBEDVIDEODECODERFFMPEG_H__
#define __EMBEDVIDEODECODERFFMPEG_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_FFMPEG

#include <vector>
#include "embedVideoDecoder.h"
#include <ffmpeg/avcodec.h>
#include "image.h"



class embedVideoDecoderFfmpeg: public embedVideoDecoder {
public:
	embedVideoDecoderFfmpeg();
	
	~embedVideoDecoderFfmpeg();

	void createDecoder(
		int width,
		int height,
		int deblocking,
		bool smoothing,
		int format,
		int outputFormat);

	// gnash calls this when it wants you to decode the given videoframe
	image::image_base*	decodeFrame(uint8_t* data, int size);


private:

	/// ffmpeg stuff
	AVCodec *codec;
	AVCodecContext *cc;

	/// Info from the video tag header. Might be usefull...
	uint32_t width;
	uint32_t height;
	int deblocking;
	bool smoothing;
	int format;
	int outputFormat;

	/// Last decoded frame
	image::image_base* decodedFrame;

};


#endif // USE_FFMPEG

#endif //  __EMBEDVIDEODECODERFFMPEG_H__
