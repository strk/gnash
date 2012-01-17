// StreamingSoundData.h - embedded sound definition, for gnash
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

#ifndef SOUND_STREAMING_SOUND_DATA_H
#define SOUND_STREAMING_SOUND_DATA_H

#include <vector>
#include <map> 
#include <memory> 
#include <set> 
#include <cassert>
#include <boost/thread/mutex.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "SimpleBuffer.h" 
#include "SoundInfo.h" 

// Forward declarations
namespace gnash {
    namespace sound {
        class InputStream;
        class StreamingSound;
    }
    namespace media {
        class MediaHandler;
    }
}

namespace gnash {
namespace sound {

/// Definition of an embedded sound
class StreamingSoundData
{
public:

    /// Container for the active instances of this sounds being played
    //
    /// NOTE: This class does NOT own the active sounds
    typedef std::list<InputStream*> Instances;

    /// Construct a sound with given data, info and volume.
    //
    /// @param info encoding info
    /// @param nVolume initial volume (0..100).
    StreamingSoundData(const media::SoundInfo& info, int nVolume);

    ~StreamingSoundData();

    /// Append a sound data block
    //
    /// @param data          Undecoded sound data. Must be appropriately
    ///                      padded (see MediaHandler::getInputPaddingBytes())
    /// @param sampleCount   The number of samples when decoded.
    /// @param seekSamples   Where to start playing from at a particular frame.
    size_t append(std::auto_ptr<SimpleBuffer> data, size_t sampleCount,
            int seekSamples);

    /// Do we have any data?
    bool empty() const {
        return _buffers.empty();
    }

    const SimpleBuffer& getBlock(size_t index) const {
        return _buffers[index];
    }

    size_t getSampleCount(size_t index) const {
        return _blockData[index].sampleCount;
    }

    size_t getSeekSamples(size_t index) const {
        return _blockData[index].seekSamples;
    }

    size_t blockCount() const {
        return _buffers.size();
    }

    size_t playingBlock() const;

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
    InputStream* firstPlayingInstance() const;

    /// Create an instance of this sound
    //
    /// The returned instance ownership is transferred
    ///
    /// @param mh               The MediaHandler to use for on-demand decoding
    /// @param blockOffset      Block number in the immutable (encoded) data
    ///                         this instance should start decoding.
    ///                         This refers to a specific StreamSoundBlock.
    ///                         @see gnash::swf::StreamSoundBlockTag
    /// Locks the _soundInstancesMutex when pushing to it
    std::auto_ptr<StreamingSound> createInstance(media::MediaHandler& mh,
            unsigned long blockOffset);

    /// Drop all active sounds
    //
    /// Locks _soundInstancesMutex
    ///
    void clearInstances();

    /// Drop an active sound (by iterator)
    //
    /// Does *NOT* lock the _soundInstancesMutex
    ///
    /// @return iterator after the one being erased
    ///
    Instances::iterator eraseActiveSound(Instances::iterator i);

    /// Drop an active sound (by pointer)
    //
    /// @param inst The active sound instance to unregister
    ///
    /// This is intended to be called by StreamingSoundDataInst
    /// destructor, which may be called by a separate thread
    /// so MUST be thread-safe
    ///
    /// Does lock the _soundInstancesMutex
    void eraseActiveSound(InputStream* inst);

    /// Object holding information about the sound
    media::SoundInfo soundinfo;

    /// Volume for AS-sounds, range: 0-100.
    /// It's the SWF range that is represented here.
    int volume;

private:

    struct BlockData
    {
        BlockData(size_t count, int seek)
            :
            sampleCount(count),
            seekSamples(seek)
        {}

        size_t sampleCount;
        size_t seekSamples;
    };

    /// Playing instances of this sound definition
    //
    /// Multithread access to this member is protected
    /// by the _soundInstancesMutex mutex
    Instances _soundInstances;

    /// Mutex protecting access to _soundInstances
    mutable boost::mutex _soundInstancesMutex;

    boost::ptr_vector<SimpleBuffer> _buffers;

    std::vector<BlockData> _blockData;
};

} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUND_H
