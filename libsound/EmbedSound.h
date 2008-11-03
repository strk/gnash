// EmbedSound.h - embedded sound definition, for gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "SimpleBuffer.h" // for composition
#include "SoundInfo.h" // for composition
#include "SoundEnvelope.h" // for SoundEnvelopes define

#include <map> // for composition (m_frame_size)
#include <memory> // for auto_ptr (composition)
#include <cassert>


// Forward declarations
namespace gnash {
    namespace sound {
        class EmbedSoundInst;
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
    /// The undecoded data
    std::auto_ptr<SimpleBuffer> _buf;

    void ensureBufferPadding();

public:

    /// Construct a sound with given data, info and volume.
    //
    /// @param data The encoded sound data. May be the NULL pointer for streaming sounds,
    ///     in which case data will be appended later using ::append()
    ///
    /// @param info encoding info
    ///
    /// @param nVolume initial volume (0..100). Optional, defaults to 100.
    ///
    EmbedSound(std::auto_ptr<SimpleBuffer> data, std::auto_ptr<media::SoundInfo> info, int nVolume=100);

    ~EmbedSound();

    /// Object holding information about the sound
    std::auto_ptr<media::SoundInfo> soundinfo;

    typedef std::map<boost::uint32_t,boost::uint32_t> FrameSizeMap;

    /// Maps frame sizes to start-of-frame offsets
    FrameSizeMap m_frames_size;

    /// Append size bytes to this sound
    //
    /// @param data
    /// Data bytes, allocated with new[]. Ownership transferred.
    ///
    /// @param size
    /// Size of the 'data' buffer.
    ///
    void append(boost::uint8_t* data, unsigned int size);

    /// Return size of the data buffer
    size_t size() const 
    {
        return _buf->size();
    }

    /// Is the data buffer empty ?
    bool empty() const 
    {
        return _buf->empty();
    }

    /// Return a pointer to the underlying buffer
    const boost::uint8_t* data() const {
        return _buf->data();
    }

    /// Return a pointer to the underlying buffer
    boost::uint8_t* data() {
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

    /// Return a pointer to an offset in the underlying buffer
    //
    /// @param pos The offset value.
    ///     An assertion will fail if pos > size()
    ///
    boost::uint8_t* data(size_t pos) {
        assert(pos < _buf->size());
        return _buf->data()+pos;
    }

    /// Are there known playing instances of this sound ?
    bool isPlaying() const {
        return !_soundInstances.empty();
    }

    /// Create an instance of this sound
    //
    /// The returned instance is owned by this class
    ///
    /// @param mh
    ///     The MediaHandler to use for on-demand decoding
    ///
    /// @param blockOffset
    ///     Byte offset in the immutable (encoded) data this
    ///     instance should start decoding.
    ///     This is currently used for streaming embedded sounds
    ///     to refer to a specific StreamSoundBlock.
    ///     @see gnash::swf::StreamSoundBlockTag
    ///
    /// @param secsOffset
    ///     Offset, in seconds, this instance should start playing
    ///     from. @todo take samples (for easier implementation of
    ///     seekSamples in streaming sound).
    ///
    /// @param envelopes
    ///     SoundEnvelopes to apply to this sound. May be 0 for none.
    ///
    /// @param loopCount
    ///     Number of times this instance should loop over the defined sound.
    ///     @todo document if every loop starts at secsOffset !
    ///
    /// @todo split this in createEventSoundInstance
    ///                 and createStreamingSoundInstance
    ///
    EmbedSoundInst* createInstance( media::MediaHandler& mh,
            unsigned long blockOffset, unsigned int secsOffset,
            const SoundEnvelopes* envelopes, unsigned int loopCount);

    /// Volume for AS-sounds, range: 0-100.
    /// It's the SWF range that is represented here.
    int volume;

    /// Vector containing the active instances of this sounds being played
    //
    /// NOTE: This class *owns* all active sounds
    ///
    typedef std::list<EmbedSoundInst*> Instances;

    Instances _soundInstances;

    /// Drop all active sounds
    void clearInstances();

    /// Drop an active sound (by iterator)
    //
    /// @return iterator after the one being erased
    ///
    Instances::iterator eraseActiveSound(Instances::iterator i);
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUND_H
