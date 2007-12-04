// VideoDecoder.h: Video decoding base class.
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

// $Id: VideoDecoder.h,v 1.9 2007/12/04 11:45:26 strk Exp $

#ifndef __VIDEODECODER_H__
#define __VIDEODECODER_H__

#include "MediaParser.h"
#include "image.h"

namespace gnash {
namespace media {

/// Video decoding base class.
class VideoDecoder {
	
public:
	VideoDecoder() {}

	// virtual classes need a virtual destructor !
	virtual ~VideoDecoder() {}

	/// Return the number of bytes input frame data is expected
	/// to be padded with zeroes. Make sure to provide such
	/// padding to avoid illegal reads.
	///
	virtual unsigned getPaddingBytes() const { return 8; }

	/// Sets up the decoder.
	//
	/// @param info
	/// 	VideoInfo class with all the info needed to decode
	///     the video correctly.
	///
	/// @return true if succesfull else false
	///
	virtual bool setup(VideoInfo* /*info*/) { return false; }

	/// Sets up the decoder.
	//
	/// @param width
	/// 	The width of the video
	///
	/// @param height
	/// 	The height of the video
	///
	/// @param deblocking
	/// 	Should a deblocking filter be used? 1 = off, 2 = on
	///
	/// @param smoothing
	/// 	Should the video be smoothed?
	///
	/// @param format
	/// 	The codec of the video, see codecType
	///
	/// @param outputFormat
	/// 	The outputFormat of the video, see videoOutputFormat
	///
	/// @return true if succesfull else false
	///
	virtual bool setup(
		int /*width*/,
		int /*height*/,
		int /*deblocking*/,
		bool /*smoothing*/,
		videoCodecType /*format*/,
		int /*outputFormat*/) /* should this argument be of VideoOutputFormat type ?*/ { return false; }

	/// Decodes a frame and returns a pointer to the data
	//
	/// @param input
	/// 	The video data
	///
	/// @param inputSize
	/// 	The size of the video data
	///
	/// @param outputSize
	/// 	The output size of the video data, is passed by reference.
	///
	/// @return a pointer to the decoded data, or NULL if decoding fails.
	///     The caller owns the decoded data.
	///	
	virtual uint8_t* decode(uint8_t* /*input*/, boost::uint32_t /*inputSize*/, boost::uint32_t& /*outputSize*/) { return NULL; }

	/// Decodes a frame and returns an image::base containing it
	//
	/// @param input
	/// 	The video data
	///
	/// @param inputSize
	/// 	The size of the video data
	///
	/// @return a pointer to the image with the decoded data, or NULL if decoding fails.
	///     The caller owns the decoded data.
	///
	virtual std::auto_ptr<image::image_base> decodeToImage(uint8_t* /*input*/, boost::uint32_t /*inputSize*/) { return std::auto_ptr<image::image_base>(NULL); }

};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __VIDEODECODER_H__
