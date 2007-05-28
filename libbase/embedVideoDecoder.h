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

// 
//
//

//  $Id: embedVideoDecoder.h,v 1.5 2007/05/28 14:59:29 ann Exp $

#ifndef __EMBEDVIDEODECODER_H__
#define __EMBEDVIDEODECODER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "image.h"

//
// Decoder for embedded video.
//

class embedVideoDecoder
{
public:

	// This is copied from the render and should be changed if the original is.
	enum videoOutputFormat
	{
		NONE,
		YUV,
		RGB
	};

	enum codecType
	{
		CODEC_H263 = 2,	// H263/SVQ3 video codec
		CODEC_SCREENVIDEO = 3,	// Screenvideo codec
		CODEC_VP6 = 4,		// On2 VP6 video codec
		CODEC_VP6A = 5,		// On2 VP6 Alpha video codec
		CODEC_SCREENVIDEO2 = 6	// Screenvideo2 codec
	};

	// Assign handles however you like.
	virtual void createDecoder(
		int /*width*/,
		int /*height*/,
		int /*deblocking*/,
		bool /*smoothing*/,
		int /*format*/,
		int /*outputFormat*/){}

	// gnash calls this when it wants you to decode the given videoframe
	virtual image::image_base*	decodeFrame(uint8_t* /*data*/, int /*size*/) { return NULL; }

	virtual ~embedVideoDecoder() {};
};

#endif // __EMBEDVIDEODECODER_H__
