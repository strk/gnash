// AudioDecoderSimple.h: Audio decoding using "simple" internal decoders.
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


#ifndef GNASH_AUDIODECODERSIMPLE_H
#define GNASH_AUDIODECODERSIMPLE_H

#include "AudioDecoder.h" // for inheritance
#include "MediaParser.h" // for audioCodecType enum (composition)

// Forward declarations
namespace gnash {
    namespace media {
        class SoundInfo;
        class AudioInfo;
    }
}

namespace gnash {
namespace media {

/// Audio decoding using "simple" internal decoders.
class AudioDecoderSimple : public AudioDecoder {

public:

	/// @param info
	/// 	AudioInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.
    ///
    /// @throws MediaException on failure
    ///
	AudioDecoderSimple(const AudioInfo& info);
	
	/// @param info
	/// 	SoundInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.	
    ///
    /// @throws MediaException on failure
    ///
	AudioDecoderSimple(const SoundInfo& info);

	~AudioDecoderSimple();

    // See dox in AudioDecoder.h
	boost::uint8_t* decode(const boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedBytes);

private:

    // throws MediaException on failure
	void setup(const AudioInfo& info);

    // throws MediaException on failure
	void setup(const SoundInfo& info);

	// codec
	audioCodecType _codec;

	// samplerate
	boost::uint16_t _sampleRate;

	// sampleCount
	boost::uint32_t _sampleCount;

	// stereo
	bool _stereo;

	// samplesize: 8 or 16 bit
	bool _is16bit;


	// 
};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIODECODERSIMPLE_H__

