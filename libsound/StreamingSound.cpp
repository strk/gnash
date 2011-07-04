// StreamingSound.cpp - instance of an embedded sound, for gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
//   Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "StreamingSound.h"

#include <cmath>

#include "AudioDecoder.h" 
#include "log.h" 
#include "SoundUtils.h"

namespace gnash {
namespace sound {

StreamingSound::StreamingSound(StreamingSoundData& sd,
            media::MediaHandler& mh, sound_handler::StreamBlockId block,
            unsigned int inPoint)
        :
        LiveSound(mh, sd.soundinfo, inPoint),
        _currentBlock(block),
        _positionInBlock(0),
        // parameter is in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _inPoint(inPoint * 4),
        _soundDef(sd)
{
}

bool
StreamingSound::moreData()
{
    if (decodingCompleted()) return false;

    decodeNextBlock();
    return true;
}

void
StreamingSound::decodeNextBlock()
{
    assert(!decodingCompleted());

    const bool parse = requiresParsing(_soundDef.soundinfo);

    // Get the current block of sound data.
    const SimpleBuffer& block = _soundDef.getBlock(_currentBlock);

    // If we didn't decode all of a block, do so now. Not sure if this
    // can happen.
    const boost::uint32_t inputSize = block.size() - _positionInBlock; 

    assert(inputSize);
    const boost::uint8_t* input = block.data() + _positionInBlock;

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = decoder().decode(input, inputSize,
            decodedDataSize, consumed, parse);

    // Check if the entire block was consumed.
    if (consumed == block.size()) {
        // Go to next block
        ++_currentBlock;
        _positionInBlock = 0;
    }
    else _positionInBlock += consumed;

    assert(!(decodedDataSize % 2));

    boost::int16_t* samples = reinterpret_cast<boost::int16_t*>(decodedData);
    unsigned int nSamples = decodedDataSize / 2;

    if (_soundDef.volume != 100) {
        adjustVolume(samples, samples + nSamples, _soundDef.volume/100.0);
    }

    // decodedData ownership transferred here
    appendDecodedData(decodedData, decodedDataSize);
}

bool
StreamingSound::eof() const
{
    // it isn't threaded, but just in case, we call decodingCompleted first
    return (decodingCompleted() && !decodedSamplesAhead());
}

StreamingSound::~StreamingSound()
{
    _soundDef.eraseActiveSound(this);
}

} // gnash.sound namespace 
} // namespace gnash

