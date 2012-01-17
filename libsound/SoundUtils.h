// SoundUtils.h     Utilities for handling sound data.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_SOUND_UTILS_H
#define GNASH_SOUND_UTILS_H

#include <algorithm>
#include <boost/bind.hpp>
#include <boost/cstdint.hpp>

#include "SoundInfo.h"

namespace gnash {
namespace sound {

/// Volume adjustment
//
/// @param start        The beginning of the range to adjust volume for
/// @param end         The end of the range to adjust volume for
/// @param volume       Volume factor
template<typename T>
inline void
adjustVolume(T* start, T* end, float volume)
{
    std::transform(start, end, start,
            boost::bind(std::multiplies<float>(), volume, _1));
}

/// Convert SWF-specified number of samples to output number of samples
//
/// SWF-specified number of samples are: delaySeek in DEFINESOUND,
/// latency in STREAMSOUNDHEAD and seekSamples in STREAMSOUNDBLOCK.
/// These refer to samples at the sampleRate of input.
///
/// As gnash will resample the sounds to match expected output
/// (44100 Hz, stereo 16bit) this function is handy to convert
/// for simpler use later.
inline size_t
swfToOutSamples(const media::SoundInfo& sinfo, size_t swfSamples,
        const size_t outRate = 44100)
{
    // NOTE: this was tested with inputs:
    //     - isStereo?0 is16bit()?1 sampleRate?11025
    //     - isStereo?0 is16bit()?1 sampleRate?22050
    //     - isStereo?1 is16bit()?1 sampleRate?22050
    //     - isStereo?0 is16bit()?1 sampleRate?44100
    //     - isStereo?1 is16bit()?1 sampleRate?44100
    //
    // TODO: test with other sample sizes !
    return swfSamples * (outRate / sinfo.getSampleRate());
}

} // namespace sound
} // namespace gnash

#endif
