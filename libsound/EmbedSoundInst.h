// EmbedSoundInst.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_EMBEDSOUNDINST_H
#define SOUND_EMBEDSOUNDINST_H

#include "InputStream.h" // for inheritance
#include "AudioDecoder.h" // for dtor visibility
#include "SoundEnvelope.h" // for SoundEnvelopes typedef
#include "SimpleBuffer.h" // for composition (decoded data)
#include "EmbedSound.h" // for inlines

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
    EmbedSoundInst(const EmbedSound& def, media::MediaHandler& mh,
            unsigned long blockOffset, unsigned int secsOffset,
            const SoundEnvelopes* envelopes,
            unsigned int loopCount);

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples);

    // See dox in sound_handler.h (InputStream)
    unsigned int samplesFetched() const;

    // See dox in sound_handler.h (InputStream)
    bool eof() const;

private:

    /// Current decoding position in the encoded stream
    unsigned long decodingPosition;

    /// Current playback position in the decoded stream
    unsigned long playbackPosition;

    /// Numbers of loops: -1 means loop forever, 0 means play once.
    /// For every loop completed, it is decremented.
    long loopCount;

    /// Offset in seconds to make playback start in-sync
    //
    /// only used with mp3 streams.
    ///
    unsigned int offSecs;

    /// Sound envelopes for the current sound, which determine the volume level
    /// from a given position. Only used with event sounds.
    const SoundEnvelopes* envelopes;

    /// Index of current envelope.
    boost::uint32_t current_env;

    /// Number of samples fetched so far.
    unsigned long _samplesFetched;

    /// Get the sound definition this object is an instance of
    const EmbedSound& getSoundData() {    
        return _soundDef;
    }

    /// Append size bytes to this raw data 
    //
    /// @param data
    /// Data bytes, allocated with new[]. Ownership transferred.
    ///
    /// @param size
    /// Size of the 'data' buffer.
    ///
    void appendDecodedData(boost::uint8_t* data, unsigned int size)
    {
        if ( ! _decodedData.get() )
        {
            _decodedData.reset( new SimpleBuffer );
        }
  
        _decodedData->append(data, size);
        delete [] data; // ownership transferred...
    }
  
    /// Set decoded data
    //
    /// @param data
    /// Data bytes, allocated with new[]. Ownership transferred.
    ///
    /// @param size
    /// Size of the 'data' buffer.
    ///
    void setDecodedData(boost::uint8_t* data, unsigned int size)
    {
        if ( ! _decodedData.get() )
        {
            _decodedData.reset( new SimpleBuffer() );
        }

        _decodedData->resize(0); // shouldn't release memory
        _decodedData->append(data, size);
        delete [] data; // ownership transferred...
    }

    size_t encodedDataSize() const
    {
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
    ///
    ///
    void applyEnvelopes(boost::int16_t* samples, unsigned int nSamples,
            unsigned int firstSampleNum,
            const SoundEnvelopes& env);

    /// AS-volume adjustment
    //
    /// @param samples
    ///     The 16bit samples to adjust volume of
    ///
    /// @param nSamples
    ///     Number of samples in the array
    ///
    /// @param volume
    ///     Volume factor
    ///
    static void adjustVolume(boost::int16_t* samples,
            unsigned int nSamples, float volume);

    /// Returns the data pointer in the encoded datastream
    /// for the given position. Boundaries are checked.
    //
    /// Uses _samplesFetched and playbackPosition
    ///
    /// @todo make private
    ///
    const boost::uint8_t* getEncodedData(unsigned long int pos);

    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead() const
    {
        unsigned int bytesAhead = decodedDataSize() - playbackPosition;
        assert(!(bytesAhead%2));

        unsigned int samplesAhead = bytesAhead/2;
        return samplesAhead;
    }

    /// Return true if there's nothing more to decode
    bool decodingCompleted() const
    {
        // example: 10 bytes of encoded data, decodingPosition 8 : more to decode
        // example: 10 bytes of encoded data, decodingPosition 10 : nothing more to decode

        return ( decodingPosition >= encodedDataSize() );
    }
  

    /// The decoder object used to convert the data into the playable format
    std::auto_ptr<media::AudioDecoder> _decoder;

    /// Create a decoder for this instance
    //
    /// If decoder creation fails an error will
    /// be logged, and _decoder won't be set
    /// 
    void createDecoder(media::MediaHandler& mediaHandler);

    /// Return full size of the decoded data buffer
    size_t decodedDataSize() const
    {
        if ( _decodedData.get() )
        {
            return _decodedData->size();
        }
        else return 0;
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

    /// The encoded data
    const EmbedSound& _soundDef;

    /// The decoded buffer
    //
    /// If NULL, the _soundDef will be considered
    /// decoded instead
    ///
    std::auto_ptr<SimpleBuffer> _decodedData;

    /// Decode next input block
    //
    /// It's assumed !decodingCompleted()
    ///
    void decodeNextBlock();
};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
