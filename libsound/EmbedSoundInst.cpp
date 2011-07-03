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
//

#include "EmbedSoundInst.h"

#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>

#include "SoundInfo.h" // for use
#include "MediaHandler.h" // for use
#include "GnashException.h" // for SoundException
#include "AudioDecoder.h" // for use
#include "SoundEnvelope.h" // for use
#include "log.h" // will import boost::format too
#include "SoundUtils.h"

// Debug sound decoding
//#define GNASH_DEBUG_SOUNDS_DECODING

//#define GNASH_DEBUG_SOUNDS_MANAGEMENT

namespace gnash {
namespace sound {

unsigned int
EmbedSoundInst::samplesFetched() const
{
    return _samplesFetched;
}

EmbedSoundInst::EmbedSoundInst(EmbedSound& soundData,
            media::MediaHandler& mediaHandler,
            sound_handler::StreamBlockId blockOffset,
            unsigned int inPoint,
            unsigned int outPoint,
            const SoundEnvelopes* env,
            unsigned int loopCount)
        :

        // should store blockOffset somewhere else too, for resetting
        decodingPosition(blockOffset),

        loopCount(loopCount),

        // parameter is in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _inPoint(inPoint*4),

        // parameter is in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _outPoint( outPoint == std::numeric_limits<unsigned int>::max() ?
                   std::numeric_limits<unsigned long>::max()
                   : outPoint * 4),

        envelopes(env),
        current_env(0),
        _samplesFetched(0),
        _decoder(0),
        _soundDef(soundData),
        _decodedData(0)
{
    playbackPosition = _inPoint; 

    createDecoder(mediaHandler);
}

/*private*/
void
EmbedSoundInst::createDecoder(media::MediaHandler& mediaHandler)
{
    const media::SoundInfo& si = _soundDef.soundinfo;

    media::AudioInfo info(
        (int)si.getFormat(), // codeci
        si.getSampleRate(), // sampleRatei
        si.is16bit() ? 2 : 1, // sampleSizei
        si.isStereo(), // stereoi
        0, // duration unknown, does it matter ?
        media::CODEC_TYPE_FLASH);

    try {
        _decoder = mediaHandler.createAudioDecoder(info);
    }
    catch (const MediaException& e) {
        log_error("AudioDecoder initialization failed: %s", e.what());
    }
}

// Pointer handling and checking functions
boost::int16_t*
EmbedSoundInst::getDecodedData(unsigned long int pos)
{
    if ( _decodedData.get() )
    {
        assert(pos < _decodedData->size());
        return reinterpret_cast<boost::int16_t*>(_decodedData->data()+pos);
    }
    else return 0;
}

bool
EmbedSoundInst::reachedCustomEnd() const
{
    if ( _outPoint == std::numeric_limits<unsigned long>::max() )
            return false;
    if ( playbackPosition >= _outPoint ) return true;
    return false;
}

unsigned int 
EmbedSoundInst::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
    // If there exist no decoder, then we can't decode!
    // TODO: isn't it documented that an EmbedSoundInst w/out a decoder
    //       means that the EmbedSound data is already decoded ?
    if (!_decoder.get())
    {
        return 0;
    }

    unsigned int fetchedSamples=0;

    while ( nSamples )
    {
        unsigned int availableSamples = decodedSamplesAhead();
        if ( availableSamples )
        {
            boost::int16_t* data = getDecodedData(playbackPosition);
            if ( availableSamples >= nSamples )
            {
                std::copy(data, data+nSamples, to);
                fetchedSamples += nSamples;

                // Update playback position (samples are 16bit)
                playbackPosition += nSamples*2;

                break; // fetched all
            }
            else
            {
                // not enough decoded samples available:
                // copy what we have and go on
                std::copy(data, data+availableSamples, to);
                fetchedSamples += availableSamples;

                // Update playback position (samples are 16bit)
                playbackPosition += availableSamples*2;

                to+=availableSamples;
                nSamples-=availableSamples;
                assert ( nSamples );

            }
        }

        // We haven't finished fetching yet, so see if we
        // have more to decode or not

        if ( decodingCompleted() || reachedCustomEnd() )
        {
            if ( loopCount )
            {
#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
                log_debug("Loops left: %d", loopCount);
#endif

                // loops ahead, reset playbackPosition to the starting 
                // position and keep looping
                --loopCount;

                // Start next loop
                playbackPosition = _inPoint; 
                _samplesFetched = 0;

                continue;
            }

#ifdef GNASH_DEBUG_SOUNDS_MANAGEMENT
            if ( reachedCustomEnd() )
            {
                log_debug("Reached custom end (pos:%d out:%d) and no looping, "
                          "sound is over", playbackPosition, _outPoint);
            }
            else
            {
                log_debug("Decoding completed and no looping, sound is over");
            }
#endif
            break; // fetched what possible, filled with silence the rest
        }

        // More to decode, then decode it
        decodeNextBlock();
    }

    // update samples played
    _samplesFetched += fetchedSamples;

    return fetchedSamples;
}

/*private*/
void
EmbedSoundInst::decodeNextBlock()
{
    assert(!decodingCompleted());

    // Should only be called when no more decoded data
    // are available for fetching.
    // Doing so we know what's the sample number
    // of the first sample in the newly decoded block.
    //
    assert( playbackPosition >= decodedDataSize() );

    boost::uint32_t inputSize = 0; // or blockSize
    bool parse = true; // need to parse ?

    // this block figures inputSize (blockSize) and need to parse (parse)
    // @todo: turn into a private function
    {
        const EmbedSound& sndData = _soundDef;

        // Figure the need to parse..
        switch (sndData.soundinfo.getFormat())
        {
            case media::AUDIO_CODEC_ADPCM:
#ifdef GNASH_DEBUG_SOUNDS_DECODING
                log_debug(" sound format is ADPCM");
#endif
                parse = false;
                break;
            default:
                break;
        }

        // Figure the frame size ...
        inputSize = encodedDataSize() - decodingPosition;
        if (!sndData.m_frames_size.empty())
        {
            const EmbedSound::FrameSizeMap& m = sndData.m_frames_size;
            EmbedSound::FrameSizeMap::const_iterator it =
                        m.find(decodingPosition);
            if ( it != m.end() )
            {
                inputSize = it->second; 
#ifdef GNASH_DEBUG_SOUNDS_DECODING
                log_debug(" frame size for frame starting at offset %d is %d",
                    decodingPosition, inputSize);
#endif
            }
            else
            {
                // this should never happen, as we keep track of 
                // sizes for each audio block in input
                log_error("Unknown size of audio block starting at offset %d",
                    " (should never happen)",
                    decodingPosition);
            }
        }
    }

#ifdef GNASH_DEBUG_SOUNDS_DECODING
    log_debug("  decoding %d bytes, parse:%d", inputSize, parse);
#endif

    assert(inputSize);
    const boost::uint8_t* input = getEncodedData(decodingPosition);

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = _decoder->decode(
                                      input, 
                                      inputSize,
                                      decodedDataSize,
                                      consumed,
                                      parse);

    decodingPosition += consumed;

    assert(!(decodedDataSize%2));

    // @todo I hope there are no alignment issues in this cast from int8_t* to int16_t* !
    boost::int16_t* samples = reinterpret_cast<boost::int16_t*>(decodedData);
    unsigned int nSamples = decodedDataSize/2;

#ifdef GNASH_DEBUG_MIXING
    log_debug("  applying volume/envelope to %d bytes (%d samples)"
            "of decoded data", decodedDataSize, nSamples);
#endif

    // If the volume needs adjustments we call a function to do that (why are we doing this manually ?)
    if (_soundDef.volume != 100) // volume is a private member
    {
        adjustVolume(samples, samples + nSamples, _soundDef.volume/100.0);
    }

    /// @todo is use of envelopes really mutually exclusive with
    ///       setting the volume ??
    else if (envelopes) // envelopes are a private member
    {
        unsigned int firstSample = playbackPosition/2;

        applyEnvelopes(samples, nSamples, firstSample, *envelopes);
    }

#ifdef GNASH_DEBUG_MIXING
    log_debug("  appending %d bytes to decoded buffer", decodedDataSize);
#endif


    // decodedData ownership transferred here
    appendDecodedData(decodedData, decodedDataSize);
}


const boost::uint8_t*
EmbedSoundInst::getEncodedData(unsigned long int pos)
{
    return _soundDef.data(pos);
}

/* private */
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
    return ( ( decodingCompleted() || reachedCustomEnd() ) && !loopCount && !decodedSamplesAhead() );
}

EmbedSoundInst::~EmbedSoundInst()
{
    _soundDef.eraseActiveSound(this);
}

void
EmbedSoundInst::appendDecodedData(boost::uint8_t* data, unsigned int size)
{
    if ( ! _decodedData.get() )
    {
        _decodedData.reset( new SimpleBuffer );
    }

    _decodedData->append(data, size);

    delete [] data; // ownership transferred...
}

} // gnash.sound namespace 
} // namespace gnash

