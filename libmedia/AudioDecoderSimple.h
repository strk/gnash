// AudioDecoderSimple.h: Audio decoding using "simple" internal decoders.
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

//  $Id: AudioDecoderSimple.h,v 1.6 2007/12/12 10:06:59 zoulunkai Exp $

#ifndef __AUDIODECODERSIMPLE_H__
#define __AUDIODECODERSIMPLE_H__

#include "log.h"
#include "AudioDecoder.h"

namespace gnash {
	class SoundInfo;
}

namespace gnash {
namespace media {

/// Audio decoding using "simple" internal decoders.
class AudioDecoderSimple : public AudioDecoder {

public:
	AudioDecoderSimple();
	~AudioDecoderSimple();

	bool setup(AudioInfo* info);

	bool setup(SoundInfo* info);

	boost::uint8_t* decode(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedBytes, bool parse);

private:

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

