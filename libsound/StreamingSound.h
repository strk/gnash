// StreamingSound.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_STREAMINGSOUND_H
#define SOUND_STREAMINGSOUND_H

#include <boost/scoped_ptr.hpp>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types

#include "LiveSound.h" 
#include "AudioDecoder.h" 
#include "SoundEnvelope.h"
#include "SimpleBuffer.h" 
#include "StreamingSoundData.h" 
#include "sound_handler.h" 

// Forward declarations
namespace gnash {
    namespace sound {
        class StreamingSoundData;
    }
    namespace media {
        class MediaHandler;
    }
}

namespace gnash {
namespace sound {

/// Instance of a defined %sound (StreamingSoundData)
//
/// This class contains a pointer to the StreamingSoundData used for playing
/// and a SimpleBuffer to use when decoding is needed.
class StreamingSound : public LiveSound
{
public:

    /// Create an embedded %sound instance
    //
    /// @param def      The sound data for this sound 
    /// @param mh       The MediaHandler to use for on-demand decoding
    /// @param blockId  Identifier of the encoded block to start decoding from.
    ///                 @see gnash::swf::StreamSoundBlockTag
    StreamingSound(StreamingSoundData& def, media::MediaHandler& mh,
            sound_handler::StreamBlockId blockId);

    // See dox in sound_handler.h (InputStream)
    virtual bool eof() const;

    /// Unregister self from the associated StreamingSoundData
    //
    /// WARNING: must be thread-safe!
    ~StreamingSound();

    size_t currentBlock() const {
        return _currentBlock;
    }

private:

    /// Called when more data is required.
    //
    /// StreamingSounds can temporarily run out of data if not enough
    /// samples are present for a frame.
    virtual bool moreData();

    /// Return true if there's nothing more to decode
    virtual bool decodingCompleted() const {
        return _positionInBlock == 0 && 
            _currentBlock >= _soundDef.blockCount();
    }

    /// Decode next input block
    //
    /// It's assumed !decodingCompleted()
    void decodeNextBlock();

    // The current block of sound.
    size_t _currentBlock;

    // The position within the current block.
    size_t _positionInBlock;

    /// The encoded data
    //
    /// It is non-const because we deregister ourselves
    /// from its container of playing instances on destruction
    ///
    StreamingSoundData& _soundDef;
};


} // gnash.sound namespace 
} // namespace gnash

#endif
