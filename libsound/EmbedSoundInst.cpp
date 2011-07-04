// EmbedSoundInst.cpp - instance of an embedded sound, for gnash
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

#include "EmbedSoundInst.h"

#include <cmath>
#include <vector>

#include "SoundInfo.h" // for use
#include "MediaHandler.h" // for use
#include "AudioDecoder.h" // for use
#include "SoundEnvelope.h" // for use
#include "log.h" 
#include "SoundUtils.h"

// Debug sound decoding
//#define GNASH_DEBUG_SOUNDS_DECODING

//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

namespace gnash {
namespace sound {

EmbedSoundInst::EmbedSoundInst(EmbedSound& soundData,
            media::MediaHandler& mediaHandler,
            unsigned int inPoint, unsigned int outPoint,
            const SoundEnvelopes* env, unsigned int loopCount)
        :
        LiveSound(mediaHandler, soundData.soundinfo, inPoint),
        decodingPosition(0),
        loopCount(loopCount),
        // parameters are in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _outPoint( outPoint == std::numeric_limits<unsigned int>::max() ?
                   std::numeric_limits<unsigned long>::max()
                   : outPoint * 4),
        envelopes(env),
        current_env(0),
        _soundDef(soundData)
{
}

bool
EmbedSoundInst::reachedCustomEnd() const
{
    if (_outPoint == std::numeric_limits<unsigned long>::max()) return false;
    if (playbackPosition() >= _outPoint) return true;
    return false;
}

bool
EmbedSoundInst::moreData()
{
    if (decodingCompleted() || reachedCustomEnd()) {

        if (loopCount) {
            --loopCount;
            restart();
            return true;
        }
        // Nothing more to do.
        return false;
    }

    // It's not clear if this happens for Embedded sounds, but it
    // would permit incremental decoding.
    decodeNextBlock();
    return true;
}

void
EmbedSoundInst::decodeNextBlock()
{
    assert(!decodingCompleted());

    const bool parse = requiresParsing(_soundDef.soundinfo);
    const boost::uint32_t inputSize = _soundDef.size() - decodingPosition;

#ifdef GNASH_DEBUG_SOUNDS_DECODING
    log_debug("  decoding %d bytes, parse:%d", inputSize, parse);
#endif

    assert(inputSize);
    const boost::uint8_t* input = _soundDef.data(decodingPosition);

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = decoder().decode(input, inputSize,
            decodedDataSize, consumed, parse);

    decodingPosition += consumed;

    assert(!(decodedDataSize%2));

    // @todo I hope there are no alignment issues in this cast from int8_t* to int16_t* !
    boost::int16_t* samples = reinterpret_cast<boost::int16_t*>(decodedData);
    unsigned int nSamples = decodedDataSize/2;

#ifdef GNASH_DEBUG_MIXING
    log_debug("  applying volume/envelope to %d bytes (%d samples)"
            "of decoded data", decodedDataSize, nSamples);
#endif

    // Adjust volume
    if (_soundDef.volume != 100) {
        adjustVolume(samples, samples + nSamples, _soundDef.volume/100.0);
    }

    /// @todo is use of envelopes really mutually exclusive with
    ///       setting the volume ??
    else if (envelopes) {
        unsigned int firstSample = playbackPosition() / 2;
        applyEnvelopes(samples, nSamples, firstSample, *envelopes);
    }

#ifdef GNASH_DEBUG_MIXING
    log_debug("  appending %d bytes to decoded buffer", decodedDataSize);
#endif


    // decodedData ownership transferred here
    appendDecodedData(decodedData, decodedDataSize);
}

void
EmbedSoundInst::applyEnvelopes(boost::int16_t* samples, unsigned int nSamples,
        unsigned int firstSampleOffset, const SoundEnvelopes& env)
{

    // Number of envelopes defined
    size_t numEnvs = env.size();

    // Nothing to do if we applied all envelopes already
    if (numEnvs <= current_env) {
        return;
    }

    // Not yet time to use the current envelope
    if (env[current_env].m_mark44 >= firstSampleOffset+nSamples) {
        return;
    }

    assert(env[current_env].m_mark44 < firstSampleOffset+nSamples);

    // Get next envelope position (absolute samples offset)
    boost::uint32_t next_env_pos = 0;
    if (current_env == (env.size() - 1)) {
        // If there is no "next envelope" then set the next envelope
        // start point to be unreachable
        next_env_pos = env[current_env].m_mark44 + nSamples + 1;
    }
    else {
        next_env_pos = env[current_env + 1].m_mark44;
    }

    // Scan all samples in the block, applying the envelope
    // which is in effect in each subportion
    for (unsigned int i = 0; i < nSamples / 2; i += 2) {

        // @todo cache these left/right floats (in the SoundEnvelope class?)
        float left = env[current_env].m_level0 / 32768.0;
        float right = env[current_env].m_level1 / 32768.0;

        samples[i] = samples[i] * left; // Left
        samples[i + 1] = samples[i + 1] * right; // Right

        // TODO: continue from here (what is the code below doing ??

        // if we encounter the offset of next envelope,
        // switch to it !
        if ((firstSampleOffset+nSamples-i) >= next_env_pos)
        {
            if (numEnvs <= ++current_env) {
                // no more envelopes to apply
                return;
            }

            // Get next envelope position (absolute samples offset)
            if (current_env == (env.size() - 1)) {
                // If there is no "next envelope" then set the next
                // envelope start point to be unreachable
                next_env_pos = env[current_env].m_mark44 + nSamples + 1;
            }
            else {
                next_env_pos = env[current_env + 1].m_mark44;
            }
        }
    }
}

bool
EmbedSoundInst::eof() const
{
    // it isn't threaded, but just in case, we call decodingCompleted first
    // and we also check loopCount... (over paranoid?)
    return ((decodingCompleted() || reachedCustomEnd())
            && !loopCount && !decodedSamplesAhead());
}

EmbedSoundInst::~EmbedSoundInst()
{
    _soundDef.eraseActiveSound(this);
}

} // gnash.sound namespace 
} // namespace gnash

