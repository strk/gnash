// AudioDecoder.h: Audio decoding base class.
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

// $Id: AudioDecoder.h,v 1.5 2007/11/24 17:21:41 strk Exp $

#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__

#include "MediaParser.h"
#include "SoundInfo.h"

namespace gnash {
namespace media {

/// Audio decoding base class.
class AudioDecoder {
	
public:
	AudioDecoder() {}

	// virtual classes need a virtual destructor !
	virtual ~AudioDecoder() {}

	/// Sets up the decoder.
	//
	/// @param info
	/// 	AudioInfo class with all the info needed to decode
	///     the sound correctly.
	///
	/// @return true if succesfull else false
	///
	virtual bool setup(AudioInfo* /*info*/) { return false; }

	/// Sets up the decoder.
	//
	/// @param info
	/// 	SoundInfo class with all the info needed to decode
	///     the audio correctly.
	///
	/// @return true if succesfull else false
	///
	virtual bool setup(SoundInfo* /*info*/) { return false; }

	/// Decodes a frame and returns a pointer to the data
	//
	/// @param input
	/// 	The audio data
	///
	/// @param inputSize
	/// 	The size of the video data
	///
	/// @param outputSize
	/// 	The output size of the video data, is passed by reference.
	///
	/// @param decodedData
	/// 	The amount of bytes that has been decoded when decoding is done,
	///		is passed by reference.
	///
	/// @param parse
	/// 	Should we parse the audio? Needed for embedded MP3 sounds.
	///
	/// @return a pointer to the decoded data, or NULL if decoding fails.
	///     The caller owns the decoded data.
	///
	virtual uint8_t* decode(uint8_t* /*input*/, uint32_t /*inputSize*/, uint32_t& /*outputSize*/, uint32_t& /*decodedData*/, bool /*parse*/) { return NULL; }

};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIODECODER_H__
