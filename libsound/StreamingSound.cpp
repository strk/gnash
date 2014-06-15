// StreamingSound.cpp - instance of an embedded sound, for gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

namespace {

int
getInPoint(StreamingSoundData& data, size_t block)
{
    if (block >= data.blockCount()) return 0;
    const int latency = data.soundinfo.getDelaySeek();

    // For the first block just return the latency.
    if (block == 0) return latency;

    // For subsequent blocks add latency to seekSamples. This is documented
    // but not verified.
    return swfToOutSamples(data.soundinfo,
            latency + data.getSeekSamples(block));
}

}

StreamingSound::StreamingSound(StreamingSoundData& sd,
            media::MediaHandler& mh, sound_handler::StreamBlockId block)
        :
        LiveSound(mh, sd.soundinfo, getInPoint(sd, block)),
        _currentBlock(block),
        _positionInBlock(0),
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

    // Get the current block of sound data.
    const SimpleBuffer& block = _soundDef.getBlock(_currentBlock);

    // If we didn't decode all of a block, do so now. Not sure if this
    // can happen.
    const std::uint32_t inputSize = block.size() - _positionInBlock;

    std::uint32_t consumed = 0;

    // Empty blocks serve to synchronize, so don't decode but carry on.
    if (inputSize) {
        std::uint32_t decodedDataSize = 0;
        const std::uint8_t* input = block.data() + _positionInBlock;
        std::uint8_t* decodedData = decoder().decode(input, inputSize,
                decodedDataSize, consumed);

        assert(!(decodedDataSize % 2));

        std::int16_t* samples =
            reinterpret_cast<std::int16_t*>(decodedData);
        unsigned int nSamples = decodedDataSize / 2;

        if (_soundDef.volume != 100) {
            adjustVolume(samples, samples + nSamples, _soundDef.volume/100.0);
        }

        // decodedData ownership transferred here
        appendDecodedData(SimpleBuffer(decodedDataSize, decodedData));
    }

    // Check if the entire block was consumed.
    if (consumed == block.size()) {
        // Go to next block
        ++_currentBlock;
        _positionInBlock = 0;
    }
    else _positionInBlock += consumed;

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

