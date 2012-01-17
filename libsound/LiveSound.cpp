// LiveSound.cpp - instance of an embedded sound, for gnash
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

#include "LiveSound.h"

#include <algorithm>
#include <cassert>

#include "log.h"
#include "SoundInfo.h"
#include "MediaHandler.h"

namespace gnash {
namespace sound {

LiveSound::LiveSound(media::MediaHandler& mh, const media::SoundInfo& info,
        size_t inPoint)
    :
    _inPoint(inPoint * 4),
    _playbackPosition(_inPoint),
    _samplesFetched(0)
{
    createDecoder(mh, info);
}

void
LiveSound::createDecoder(media::MediaHandler& mh, const media::SoundInfo& si)
{

    media::AudioInfo info(si.getFormat(), si.getSampleRate(), 
        si.is16bit() ? 2 : 1, si.isStereo(), 0, media::CODEC_TYPE_FLASH);

    _decoder.reset(mh.createAudioDecoder(info).release());
}

unsigned int 
LiveSound::fetchSamples(boost::int16_t* to, unsigned int nSamples)
{
    unsigned int fetchedSamples = 0;

    while (nSamples) {
        unsigned int availableSamples = decodedSamplesAhead();

        if (availableSamples) {
            const boost::int16_t* data = getDecodedData(_playbackPosition);

            if (availableSamples >= nSamples) {
                std::copy(data, data + nSamples, to);
                fetchedSamples += nSamples;

                // Update playback position (samples are 16bit)
                _playbackPosition += nSamples * 2;

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

        // Get more data if it's ready. This could involve looping.
        // Even if no data is available now, it can become available
        // later.
        if (!moreData()) break;

        // We have looped.
        if (!_samplesFetched) continue;
    }

    // update samples played
    _samplesFetched += fetchedSamples;

    return fetchedSamples;
}


} // sound namespace 
} // namespace gnash

