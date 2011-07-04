// StreamingSound.cpp - instance of an embedded sound, for gnash
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "SoundInfo.h" // for use
#include "MediaHandler.h" // for use
#include "GnashException.h" // for SoundException
#include "AudioDecoder.h" // for use
#include "log.h" // will import boost::format too
#include "SoundUtils.h"

// Debug sound decoding
//#define GNASH_DEBUG_SOUNDS_DECODING

//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

namespace gnash {
namespace sound {

StreamingSound::StreamingSound(StreamingSoundData& soundData,
            media::MediaHandler& mh,
            sound_handler::StreamBlockId blockOffset,
            unsigned int inPoint)
        :
        LiveSound(mh, soundData.soundinfo, inPoint),
        _currentBlock(blockOffset),
        _positionInBlock(0),
        // parameter is in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _inPoint(inPoint * 4),
        _soundDef(soundData)
{
}

void
StreamingSound::decodeNextBlock()
{
    assert(!decodingCompleted());

    const StreamingSoundData& sndData = _soundDef;
    const bool parse =
        sndData.soundinfo.getFormat() == media::AUDIO_CODEC_ADPCM ?
        false : true;

    const SimpleBuffer& block = _soundDef.getBlock(_currentBlock);

    // Move onto next block.
    // decode it, add to decoded sound.

    // This is a streaming sound. This function is only called if there is
    // data to decode. If there is data, there must be frames.
    const boost::uint32_t inputSize = block.size() - _positionInBlock; 

#ifdef GNASH_DEBUG_SOUNDS_DECODING
    log_debug(" frame size for frame starting at offset %d is %d",
        decodingPosition, inputSize);
#endif

    assert(inputSize);
    const boost::uint8_t* input = block.data() + _positionInBlock;

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = decoder()->decode(
                                      input, 
                                      inputSize,
                                      decodedDataSize,
                                      consumed,
                                      parse);

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

#ifdef GNASH_DEBUG_MIXING
    log_debug("  appending %d bytes to decoded buffer", decodedDataSize);
#endif

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

