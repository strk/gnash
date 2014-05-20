// AudioDecoder.h: Audio decoding base class.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_AUDIODECODER_H
#define GNASH_AUDIODECODER_H

#include <cstdint> // for C99 int types

// Forward declarations
namespace gnash {
	namespace media {
		class EncodedAudioFrame;
	}
}

namespace gnash {
namespace media {

/// Audio decoding base class.
class AudioDecoder {
	
public:

	AudioDecoder() {}

	// virtual classes need a virtual destructor !
	virtual ~AudioDecoder() {}

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
	/// @return a pointer to the decoded data, or NULL if decoding fails.
	///     The caller owns the decoded data, which was allocated with new [].
	///
	/// @todo return a SimpleBuffer by unique_ptr
	///
	virtual std::uint8_t* decode(const std::uint8_t* input,
        std::uint32_t inputSize, std::uint32_t& outputSize,
        std::uint32_t& decodedData);

	/// Decodes an EncodedAudioFrame and returns a pointer to the decoded data
	//
	/// @param input
	/// 	The audio data
	///
	/// @param outputSize
	/// 	The output size of the video data, is passed by reference.
	///
	/// @return a pointer to the decoded data, or NULL if decoding fails.
	///     The caller owns the decoded data, which was allocated with new [].
	///
	/// @todo return a SimpleBuffer by unique_ptr
	///
	virtual std::uint8_t* decode(const EncodedAudioFrame& input,
	                               std::uint32_t& outputSize);

};

inline std::uint8_t*
AudioDecoder::decode(const std::uint8_t*, std::uint32_t, std::uint32_t&,
        std::uint32_t&)
{
    return 0;
}

inline std::uint8_t*
AudioDecoder::decode(const EncodedAudioFrame&, std::uint32_t&)
{
    return 0;
}
	
} // gnash.media namespace 
} // gnash namespace

#endif 
