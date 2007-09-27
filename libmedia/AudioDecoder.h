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

#ifndef __AUDIODECODER_H__
#define __AUDIODECODER_H__

#include "MediaParser.h"
#include "SoundInfo.h"

namespace gnash {

class AudioDecoder {
	
public:
	AudioDecoder() {}
	~AudioDecoder() {}

	virtual bool setup(AudioInfo* /*info*/) { return false; }
	virtual bool setup(SoundInfo* /*info*/) { return false; }
	virtual uint8_t* decode(uint8_t* /*input*/, uint32_t /*inputSize*/, uint32_t& /*outputSize*/, uint32_t& /*decodedData*/, bool /*parse*/) { return NULL; }

};
	
} // gnash namespace

#endif // __AUDIODECODER_H__
