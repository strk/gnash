// AudioDecoderMad.h: Audio decoding using the mad library.
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

// $Id:

#ifndef __AUDIODECODERMAD_H__
#define __AUDIODECODERMAD_H__

#include "log.h"
#include "AudioDecoder.h"

#include <mad.h>


namespace gnash {

class AudioDecoderMad : public AudioDecoder {

public:
	AudioDecoderMad();
	~AudioDecoderMad();

	bool setup(AudioInfo* info);
	bool setup(SoundInfo* info);

	uint8_t* decode(uint8_t* input, uint32_t inputSize, uint32_t& outputSize, uint32_t& decodedBytes, bool parse);

private:

	/// mad stuff
	mad_stream _stream;
	mad_frame _frame;
	mad_synth _synth;
};
	
} // gnash namespace

#endif // __AUDIODECODERMAD_H__
	
