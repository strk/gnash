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

// 
//
//

//  $Id: embedVideoDecoder.h,v 1.7 2007/06/06 15:41:12 tgc Exp $

#ifndef __EMBEDVIDEODECODER_H__
#define __EMBEDVIDEODECODER_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "image.h"
#include "log.h"

namespace gnash {

///
/// \brief
/// The embedVideoDecoder is used to decodes a video frame which has been
/// embedded in a SWF.
///

class embedVideoDecoder
{
public:

	/// This is copied from the render and should be changed if the original is.
	enum videoOutputFormat
	{
		NONE,
		YUV,
		RGB
	};

	/// Codecs type we know of
	enum codecType
	{
		CODEC_H263 = 2,	// H263/SVQ3 video codec
		CODEC_SCREENVIDEO = 3,	// Screenvideo codec
		CODEC_VP6 = 4,		// On2 VP6 video codec
		CODEC_VP6A = 5,		// On2 VP6 Alpha video codec
		CODEC_SCREENVIDEO2 = 6	// Screenvideo2 codec
	};

	///
	/// Sets up the decoder.
	//
	/// @param width
	/// The width of the video
	///
	/// @param height
	/// The height of the video
	///
	/// @param deblocking
	/// Should a deblocking filter be used? 1 = off, 2 = on
	///
	/// @param smoothing
	/// Should the video be smoothed?
	///
	/// @param format
	/// The codec of the video, see codecType
	///
	/// @param outputFormat
	/// The outputFormat of the video, see videoOutputFormat
	///
	virtual void createDecoder(int /*width*/,
		int /*height*/,
		int /*deblocking*/,
		bool /*smoothing*/,
		int /*format*/,
		int /*outputFormat*/){}

	///
	/// gnash calls this when it wants to decode the given videoframe.
	//
	/// @param data
	/// The video frame that is to be decoded.
	///
	/// @param size
	/// The sizes of the undecoded videoframe in bytes.
	///
	/// @return a auto_ptr containing the image which is a result of the decoding.
	/// The caller owns the returned image.
	///
	virtual std::auto_ptr<image::image_base> decodeFrame(uint8_t* /*data*/, int /*size*/) { std::auto_ptr<image::image_base> i (NULL); return i; }

	virtual ~embedVideoDecoder() {};
};

} // end of gnash namespace
#endif // __EMBEDVIDEODECODER_H__
