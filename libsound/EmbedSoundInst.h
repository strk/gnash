// EmbedSoundInst.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_EMBEDSOUNDINST_H
#define SOUND_EMBEDSOUNDINST_H

#include <memory>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types

#include "LiveSound.h"
#include "SoundEnvelope.h" 
#include "EmbedSound.h" 

// Forward declarations
namespace gnash {
    namespace sound {
        class EmbedSound;
    }
    namespace media {
        class MediaHandler;
    }
}

namespace gnash {
namespace sound {

/// Instance of a defined %sound (EmbedSound)
class EmbedSoundInst : public LiveSound
{
public:

    /// Create an embedded %sound instance
    //
    /// @param def       The definition of this sound (the immutable data)
    /// @param mh        The MediaHandler to use for on-demand decoding
    /// @param inPoint   Offset in output samples this instance should start
    ///                  playing from. These are post-resampling samples (44100 
    ///                  for one second of samples).
    /// @param outPoint  Offset in output samples this instance should stop
    ///                  playing at. These are post-resampling samples (44100 
    ///                  for one second of samples).
    ///                  Use numeric_limits<unsigned int>::max() for never
    /// @param envelopes SoundEnvelopes to apply to this sound. May be 0 for
    ///                  none.
    /// @param loopCount Number of times this instance should loop over the
    ///                  defined sound. Note that every loop begins and ends
    ///                  at the range given by inPoint and outPoint.
    EmbedSoundInst(EmbedSound& def, media::MediaHandler& mh,
            unsigned int inPoint, unsigned int outPoint,
            const SoundEnvelopes* envelopes, int loopCount);

    // See dox in sound_handler.h (InputStream)
    virtual bool eof() const;

    /// Unregister self from the associated EmbedSound
    //
    /// WARNING: must be thread-safe!
    virtual ~EmbedSoundInst();

private:

    virtual size_t checkEarlierEnd(size_t bytesAhead, size_t pos) const {
        if (_outPoint < std::numeric_limits<unsigned long>::max()) {
            const size_t toCustomEnd = _outPoint - pos;
            return std::min(toCustomEnd, bytesAhead);
        }
        return bytesAhead;
    }

    virtual bool moreData();

    /// Apply envelope-volume adjustments
    //
    /// Modified envelopes cursor (current_env)
    ///
    /// @param samples
    ///     The samples to apply envelopes to
    ///
    /// @param nSamples
    ///     Number of samples in the samples array.
    ///     (nSamples*2 bytes).
    ///
    /// @param firstSampleNum
    ///     Logical position of first sample in the array.
    ///     This number gives the sample position referred to
    ///     by SoundEnvelope objects.
    ///
    /// @param env
    ///     SoundEnvelopes to apply.
    void applyEnvelopes(boost::int16_t* samples, unsigned int nSamples,
            unsigned int firstSampleNum, const SoundEnvelopes& env);

    bool reachedCustomEnd() const;

    /// Return true if there's nothing more to decode
    virtual bool decodingCompleted() const {
        return (decodingPosition >= _soundDef.size());
    }

    /// Decode next input block
    //
    /// It's assumed !decodingCompleted()
    virtual void decodeNextBlock();

    /// Current decoding position in the encoded stream
    unsigned long decodingPosition;

    /// Numbers of loops: -1 means loop forever, 0 means play once.
    /// For every loop completed, it is decremented.
    long loopCount;

    /// Offset in bytes to end playback at
    /// Never if numeric_limits<unsigned long>::max()
    const unsigned long _outPoint;

    /// Sound envelopes for the current sound, which determine the volume level
    /// from a given position. Only used with event sounds.
    const SoundEnvelopes* envelopes;

    /// Index of current envelope.
    boost::uint32_t current_env;

    /// The encoded data
    //
    /// It is non-const because we deregister ourselves
    /// from its container of playing instances on destruction
    ///
    EmbedSound& _soundDef;

};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
