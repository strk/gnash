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
#include <vector>

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

unsigned int
StreamingSound::samplesFetched() const
{
    return _samplesFetched;
}

StreamingSound::StreamingSound(StreamingSoundData& soundData,
            media::MediaHandler& mediaHandler,
            sound_handler::StreamBlockId blockOffset,
            unsigned int inPoint)
        :
        _currentBlock(blockOffset),
        _positionInBlock(0),
        // parameter is in stereo samples (44100 per second)
        // we double to take 2 channels into account
        // and double again to use bytes
        _inPoint(inPoint * 4),
        _samplesFetched(0),
        _decoder(0),
        _soundDef(soundData),
        _decodedData(0)
{
    _playbackPosition = _inPoint; 
    createDecoder(mediaHandler);
}

/*private*/
void
StreamingSound::createDecoder(media::MediaHandler& mediaHandler)
{
    const media::SoundInfo& si = _soundDef.soundinfo;

    media::AudioInfo info(
        (int)si.getFormat(), 
        si.getSampleRate(), 
        si.is16bit() ? 2 : 1, 
        si.isStereo(), 
        0, media::CODEC_TYPE_FLASH);

    try {
        _decoder.reset(mediaHandler.createAudioDecoder(info).release());
    }
    catch (const MediaException& e) {
        log_error("AudioDecoder initialization failed: %s", e.what());
    }
}

// Pointer handling and checking functions
boost::int16_t*
StreamingSound::getDecodedData(unsigned long int pos)
{
    if (_decodedData.get()) {
        assert(pos < _decodedData->size());
        return reinterpret_cast<boost::int16_t*>(_decodedData->data() + pos);
    }
    return 0;
}

unsigned int 
StreamingSound::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
    // If there is no decoder, then we can't decode!
    // TODO: isn't it documented that an StreamingSound w/out a decoder
    //       means that the StreamingSoundData data is already decoded ?
    if (!_decoder.get()) return 0;

    unsigned int fetchedSamples = 0;

    while (nSamples) {
        unsigned int availableSamples = decodedSamplesAhead();

        if (availableSamples) {
            boost::int16_t* data = getDecodedData(_playbackPosition);

            if (availableSamples >= nSamples) {
                std::copy(data, data + nSamples, to);
                fetchedSamples += nSamples;

                // Update playback position (samples are 16bit)
                _playbackPosition += nSamples*2;

                break; // fetched all
            }
            else {
                // not enough decoded samples available:
                // copy what we have and go on
                std::copy(data, data + availableSamples, to);
                fetchedSamples += availableSamples;

                // Update playback position (samples are 16bit)
                _playbackPosition += availableSamples * 2;

                to += availableSamples;
                nSamples -= availableSamples;
                assert(nSamples);
            }
        }

        // We haven't finished fetching yet, so see if we
        // have more to decode or not
        if (decodingCompleted()) {
            break; 
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
StreamingSound::decodeNextBlock()
{
    assert(!decodingCompleted());

    // Should only be called when no more decoded data
    // are available for fetching.
    // Doing so we know what's the sample number
    // of the first sample in the newly decoded block.
    assert(_playbackPosition >= decodedDataSize());

    const StreamingSoundData& sndData = _soundDef;
    const bool parse =
        sndData.soundinfo.getFormat() == media::AUDIO_CODEC_ADPCM ?
        false : true;

    // Move onto next block.
    // decode it, add to decoded sound.

    // This is a streaming sound. This function is only called if there is
    // data to decode. If there is data, there must be frames.
    const boost::uint32_t inputSize = sndData._buffers[_currentBlock].size() -
        _positionInBlock; 

#ifdef GNASH_DEBUG_SOUNDS_DECODING
    log_debug(" frame size for frame starting at offset %d is %d",
        decodingPosition, inputSize);
#endif

    assert(inputSize);
    const boost::uint8_t* input = sndData._buffers[_currentBlock].data() + 
        _positionInBlock;

    boost::uint32_t consumed = 0;
    boost::uint32_t decodedDataSize = 0;
    boost::uint8_t* decodedData = _decoder->decode(
                                      input, 
                                      inputSize,
                                      decodedDataSize,
                                      consumed,
                                      parse);

    if (consumed == sndData._buffers[_currentBlock].size()) {
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

void
StreamingSound::appendDecodedData(boost::uint8_t* data, unsigned int size)
{
    if (!_decodedData.get()) {
        _decodedData.reset(new SimpleBuffer);
    }

    _decodedData->append(data, size);

    delete [] data; // ownership transferred...
}

} // gnash.sound namespace 
} // namespace gnash

