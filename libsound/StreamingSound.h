// StreamingSound.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_STREAMINGSOUND_H
#define SOUND_STREAMINGSOUND_H

#include <boost/scoped_ptr.hpp>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types

#include "InputStream.h" 
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
///
/// When the SimpleBuffer is NULL we'll play the StreamingSoundData bytes directly
/// (we assume they are decoded already)
///
class StreamingSound : public InputStream
{
public:

    /// Create an embedded %sound instance
    //
    /// @param def
    ///     The definition of this sound (where immutable data is kept)
    ///
    /// @param mh
    ///     The MediaHandler to use for on-demand decoding
    ///
    /// @param blockId
    ///     Identifier of the encoded block to start decoding from.
    ///     @see gnash::swf::StreamSoundBlockTag
    ///
    /// @param inPoint
    ///     Offset in output samples this instance should start
    ///     playing from. These are post-resampling samples (44100 
    ///     for one second of samples).
    StreamingSound(StreamingSoundData& def, media::MediaHandler& mh,
            sound_handler::StreamBlockId blockId,
            unsigned int inPoint);

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples);

    // See dox in sound_handler.h (InputStream)
    unsigned int samplesFetched() const;

    // See dox in sound_handler.h (InputStream)
    bool eof() const;

    /// Unregister self from the associated StreamingSoundData
    //
    /// WARNING: must be thread-safe!
    ~StreamingSound();

private:

    /// Append size bytes to this raw data 
    //
    /// @param data
    /// Data bytes, allocated with new[]. Ownership transferred.
    ///
    /// @param size
    /// Size of the 'data' buffer.
    void appendDecodedData(boost::uint8_t* data, unsigned int size);

    /// Returns the data pointer in the encoded datastream
    /// for the given position. Boundaries are checked.
    //
    /// Uses _samplesFetched and _playbackPosition
    const boost::uint8_t* getEncodedData(unsigned long int pos);

    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead() const {

        const unsigned int dds = decodedDataSize();
        if (dds <= _playbackPosition) return 0; 

        const unsigned int bytesAhead = dds - _playbackPosition;
        assert(!(bytesAhead%2));

        const unsigned int samplesAhead = bytesAhead / 2;
        return samplesAhead;
    }

    /// Return true if there's nothing more to decode
    bool decodingCompleted() const {
        return _positionInBlock == 0 && 
            _currentBlock >= _soundDef._buffers.size();
    }

    /// Create a decoder for this instance
    //
    /// If decoder creation fails an error will
    /// be logged, and _decoder won't be set
    void createDecoder(media::MediaHandler& mediaHandler);

    /// Return full size of the decoded data buffer
    size_t decodedDataSize() const {
        return _decodedData.get() ? _decodedData->size() : 0;
    }

    /// Access data in the decoded datastream for the given byte offset.
    //
    /// Boundaries are checked.
    ///
    /// @param pos offsets in bytes. This should usually be
    ///        a multiple of two, since decoded data is
    ///        composed of signed 16bit PCM samples..
    boost::int16_t* getDecodedData(unsigned long int pos);

    /// Decode next input block
    //
    /// It's assumed !decodingCompleted()
    ///
    void decodeNextBlock();

    // The current block of sound.
    size_t _currentBlock;

    // The position within the current block.
    size_t _positionInBlock;

    /// Current playback position in the decoded stream
    size_t _playbackPosition;

    /// Offset in bytes samples from start of the block
    /// to begin playback from
    unsigned long _inPoint;

    /// Number of samples fetched so far.
    unsigned long _samplesFetched;

    /// The decoder object used to convert the data into the playable format
    boost::scoped_ptr<media::AudioDecoder> _decoder;

    /// The encoded data
    //
    /// It is non-const because we deregister ourselves
    /// from its container of playing instances on destruction
    ///
    StreamingSoundData& _soundDef;

    /// The decoded buffer
    //
    /// If NULL, the _soundDef will be considered
    /// decoded instead
    ///
    boost::scoped_ptr<SimpleBuffer> _decodedData;
};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
