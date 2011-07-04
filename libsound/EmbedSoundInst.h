// EmbedSoundInst.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_EMBEDSOUNDINST_H
#define SOUND_EMBEDSOUNDINST_H

#include "InputStream.h" // for inheritance
#include "AudioDecoder.h" // for dtor visibility
#include "SoundEnvelope.h" // for SoundEnvelopes typedef
#include "SimpleBuffer.h" // for composition (decoded data)
#include "EmbedSound.h" // for inlines
#include "sound_handler.h" // for StreamBlockId typedef

#include <memory>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types


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
//
/// This class contains a pointer to the EmbedSound used for playing
/// and a SimpleBuffer to use when decoding is needed.
///
/// When the SimpleBuffer is NULL we'll play the EmbedSound bytes directly
/// (we assume they are decoded already)
///
class EmbedSoundInst : public InputStream
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
    /// @param inPoint
    ///     Offset in output samples this instance should start
    ///     playing from. These are post-resampling samples (44100 
    ///     for one second of samples).
    ///
    /// @param outPoint
    ///     Offset in output samples this instance should stop
    ///     playing at. These are post-resampling samples (44100 
    ///     for one second of samples).
    ///     Use numeric_limits<unsigned int>::max() for never
    ///
    /// @param envelopes
    ///     SoundEnvelopes to apply to this sound. May be 0 for none.
    ///
    /// @param loopCount
    ///     Number of times this instance should loop over the defined sound.
    ///     Note that every loop begins and ends at the range given by
    ///     inPoint and outPoint.
    ///
    EmbedSoundInst(EmbedSound& def, media::MediaHandler& mh,
            unsigned int inPoint,
            unsigned int outPoint,
            const SoundEnvelopes* envelopes,
            unsigned int loopCount);

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples);

    // See dox in sound_handler.h (InputStream)
    unsigned int samplesFetched() const;

    // See dox in sound_handler.h (InputStream)
    bool eof() const;

    /// Unregister self from the associated EmbedSound
    //
    /// WARNING: must be thread-safe!
    ///
    ~EmbedSoundInst();

private:

    /// Append size bytes to this raw data 
    //
    /// @param data
    /// Data bytes, allocated with new[]. Ownership transferred.
    ///
    /// @param size
    /// Size of the 'data' buffer.
    ///
    void appendDecodedData(boost::uint8_t* data, unsigned int size);

    size_t encodedDataSize() const {
        return _soundDef.size();
    }

    /// Apply envelope-volume adjustments
    //
    ///
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

    /// Returns the data pointer in the encoded datastream
    /// for the given position. Boundaries are checked.
    //
    /// Uses _samplesFetched and playbackPosition
    const boost::uint8_t* getEncodedData(unsigned long int pos);

    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead() const {
        unsigned int dds = decodedDataSize();
        if ( dds <= playbackPosition ) return 0; 
        unsigned int bytesAhead = dds - playbackPosition;
        assert(!(bytesAhead%2));

        if ( _outPoint < std::numeric_limits<unsigned long>::max() )
        {
            unsigned int toCustomEnd = _outPoint-playbackPosition;
            if ( toCustomEnd < bytesAhead ) bytesAhead = toCustomEnd;
        }

        unsigned int samplesAhead = bytesAhead/2;

        return samplesAhead;
    }

    bool reachedCustomEnd() const;

    /// Return true if there's nothing more to decode
    bool decodingCompleted() const
    {
        // example: 10 bytes of encoded data, decodingPosition 8 : more to decode
        // example: 10 bytes of encoded data, decodingPosition 10 : nothing more to decode

        return ( decodingPosition >= encodedDataSize() );
    }
  
    /// Create a decoder for this instance
    //
    /// If decoder creation fails an error will
    /// be logged, and _decoder won't be set
    /// 
    void createDecoder(media::MediaHandler& mediaHandler);

    /// Return full size of the decoded data buffer
    size_t decodedDataSize() const
    {
        return _decodedData.size();
    }

    /// \brief
    /// Returns the data pointer in the decoded datastream
    /// for the given byte-offset.
    //
    /// Boundaries are checked.
    ///
    /// @param pos offsets in bytes. This should usually be
    ///        a multiple of two, since decoded data is
    ///        composed of signed 16bit PCM samples..
    ///
    boost::int16_t* getDecodedData(unsigned long int pos);


    /// Decode next input block
    //
    /// It's assumed !decodingCompleted()
    ///
    void decodeNextBlock();

    /// Current decoding position in the encoded stream
    unsigned long decodingPosition;

    /// Current playback position in the decoded stream
    unsigned long playbackPosition;

    /// Numbers of loops: -1 means loop forever, 0 means play once.
    /// For every loop completed, it is decremented.
    long loopCount;

    /// Offset in bytes samples from start of the block
    /// to begin playback from
    unsigned long _inPoint;

    /// Offset in bytes to end playback at
    /// Never if numeric_limits<unsigned long>::max()
    unsigned long _outPoint;

    /// Sound envelopes for the current sound, which determine the volume level
    /// from a given position. Only used with event sounds.
    const SoundEnvelopes* envelopes;

    /// Index of current envelope.
    boost::uint32_t current_env;

    /// Number of samples fetched so far.
    unsigned long _samplesFetched;

    /// The decoder object used to convert the data into the playable format
    std::auto_ptr<media::AudioDecoder> _decoder;
    /// The encoded data
    //
    /// It is non-const because we deregister ourselves
    /// from its container of playing instances on destruction
    ///
    EmbedSound& _soundDef;

    /// The decoded buffer
    SimpleBuffer _decodedData;
};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
