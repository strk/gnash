// EmbedSound.h - embedded sound definition, for gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef SOUND_EMBEDSOUND_H
#define SOUND_EMBEDSOUND_H

#include <vector>
#include <memory> // for auto_ptr (composition)
#include <set> // for composition (_soundInstances)
#include <cassert>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>

#include "SimpleBuffer.h" // for composition
#include "SoundInfo.h" // for composition
#include "SoundEnvelope.h" // for SoundEnvelopes define

// Forward declarations
namespace gnash {
    namespace sound {
        class EmbedSoundInst;
        class InputStream;
    }
    namespace media {
        class MediaHandler;
    }
}

namespace gnash {
namespace sound {

/// Definition of an embedded sound
class EmbedSound
{
public:

    /// Vector containing the active instances of this sounds being played
    //
    /// NOTE: This class does NOT own the active sounds
    typedef std::list<EmbedSoundInst*> Instances;

    /// Construct a sound with given data, info and volume.
    //
    /// @param data The encoded sound data.
    /// @param info encoding info
    /// @param volume initial volume (0..100). Optional, defaults to 100.
    EmbedSound(std::auto_ptr<SimpleBuffer> data, const media::SoundInfo& info,
            int volume);

    ~EmbedSound();

    /// Return size of the data buffer
    size_t size() const {
        return _buf->size();
    }

    /// Is the data buffer empty ?
    bool empty() const {
        return _buf->empty();
    }

    /// Return a pointer to the underlying buffer
    const boost::uint8_t* data() const {
        return _buf->data();
    }

    /// Return a pointer to an offset in the underlying buffer
    //
    /// @param pos The offset value.
    ///     An assertion will fail if pos > size()
    ///
    const boost::uint8_t* data(size_t pos) const {
        assert(pos < _buf->size());
        return _buf->data()+pos;
    }

    /// Are there known playing instances of this sound ?
    //
    /// Locks _soundInstancesMutex
    ///
    bool isPlaying() const;

    /// Return number of playing instances of this sound
    //
    /// Locks _soundInstancesMutex
    ///
    size_t numPlayingInstances() const;

    /// Append to the given vector all playing instances of this sound def
    void getPlayingInstances(std::vector<InputStream*>& to) const;

    /// Return the first created instance of this sound
    //
    /// Locks _soundInstancesMutex
    ///
    EmbedSoundInst* firstPlayingInstance() const;

    /// Create an instance of this sound
    //
    /// The returned instance ownership is transferred
    ///
    /// @param mh
    ///     The MediaHandler to use for on-demand decoding
    ///
    /// @param inPoint
    ///     Offset in output samples this instance should start
    ///     playing from. These are post-resampling samples from
    ///     the start of the specified blockId.
    ///     
    ///
    /// @param outPoint
    ///     Offset in output samples this instance should stop
    ///     playing at. These are post-resampling samples from
    ///     the start of the specified blockId.
    ///
    /// @param envelopes
    ///     SoundEnvelopes to apply to this sound. May be 0 for none.
    ///
    /// @param loopCount
    ///     Number of times this instance should loop over the defined sound.
    ///     @todo document if every loop starts at secsOffset !
    /// Locks the _soundInstancesMutex when pushing to it
    ///
    std::auto_ptr<EmbedSoundInst> createInstance(media::MediaHandler& mh,
            unsigned int inPoint, unsigned int outPoint,
            const SoundEnvelopes* envelopes, unsigned int loopCount);

    /// Drop all active sounds
    //
    /// Locks _soundInstancesMutex
    void clearInstances();

    /// Drop an active sound (by iterator)
    //
    /// Does *NOT* lock the _soundInstancesMutex
    ///
    /// @return iterator after the one being erased
    Instances::iterator eraseActiveSound(Instances::iterator i);

    /// Drop an active sound (by pointer)
    //
    /// @param inst The active sound instance to unregister
    ///
    /// This is intended to be called by EmbedSoundInst
    /// destructor, which may be called by a separate thread
    /// so MUST be thread-safe
    ///
    /// Does lock the _soundInstancesMutex
    ///
    /// @todo make private and mark EmbedSoundInst as friend ?
    ///
    void eraseActiveSound(EmbedSoundInst* inst);

    /// Object holding information about the sound
    media::SoundInfo soundinfo;

    /// Volume for AS-sounds, range: 0-100.
    /// It's the SWF range that is represented here.
    int volume;

private:

    /// The undecoded data
    boost::scoped_ptr<SimpleBuffer> _buf;

    /// Playing instances of this sound definition
    //
    /// Multithread access to this member is protected
    /// by the _soundInstancesMutex mutex
    Instances _soundInstances;

    /// Mutex protecting access to _soundInstances
    //
    mutable boost::mutex _soundInstancesMutex;
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUND_H
