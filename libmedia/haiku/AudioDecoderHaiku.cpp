// AudioDecoderHaiku.cpp: Audio decoding using the Haiku media kit.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
//


#include "AudioDecoderHaiku.h"
//#include "MediaParserHaiku.h" // for ExtraAudioInfoHaiku
#include "FLVParser.h"
#include "SoundInfo.h"
#include "MediaParser.h" // for AudioInfo

#include "adipe.h"

//#include <cmath> // for std::ceil
//#include <algorithm> // for std::copy, std::max

//#define GNASH_DEBUG_AUDIO_DECODING

namespace gnash {
namespace media {
namespace haiku {

AudioDecoderHaiku::AudioDecoderHaiku(const AudioInfo& info)
{
    QQ(2);
}

AudioDecoderHaiku::AudioDecoderHaiku(SoundInfo& info)
{
    QQ(2);
}

AudioDecoderHaiku::~AudioDecoderHaiku()
{
    QQ(2);
}

boost::uint8_t*
AudioDecoderHaiku::decode(const boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedBytes)
{
    (void) input;
    (void) parse;
    boost::uint8_t *t;
    outputSize = 2048;
    decodedBytes = inputSize;
    t = new boost::uint8_t[outputSize];

    boost::uint16_t *data =
        reinterpret_cast<boost::uint16_t*>(t);

    for (size_t i = 0; i < outputSize / sizeof(boost::uint16_t); ++i)
    {
        data[i] = 1000 * sin(i/10.);
    }


    return t;
}

boost::uint8_t*
AudioDecoderHaiku::decode(const EncodedAudioFrame& af, boost::uint32_t& outputSize)
{
    QQ(2);
    return NULL;
}


} // gnash.media.haiku namespace 
} // gnash.media namespace 
} // gnash namespace
