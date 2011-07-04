// LiveSound.h - instance of an embedded sound, for gnash
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

#ifndef SOUND_LIVESOUND_H
#define SOUND_LIVESOUND_H

#include <boost/scoped_ptr.hpp>
#include <cassert>
#include <boost/cstdint.hpp> // For C99 int types

#include "InputStream.h" 
#include "AudioDecoder.h" 
#include "SoundEnvelope.h"
#include "SimpleBuffer.h" 
#include "sound_handler.h" 
#include "SoundInfo.h"

// Forward declarations
namespace gnash {
    namespace sound {
        class LiveSoundData;
    }
    namespace media {
        class MediaHandler;
    }
}

namespace gnash {
namespace sound {

/// Instance of a defined %sound (LiveSoundData)
//
/// This class contains a pointer to the LiveSoundData used for playing
/// and a SimpleBuffer to use when decoding is needed.
class LiveSound : public InputStream
{
public:

    /// Create an embedded %sound instance
    //
    /// @param def      The sound data for this sound 
    /// @param mh       The MediaHandler to use for on-demand decoding
    /// @param blockId  Identifier of the encoded block to start decoding from.
    ///                 @see gnash::swf::StreamSoundBlockTag
    /// @param inPoint  Offset in output samples this instance should start
    ///                 playing from. These are post-resampling samples (44100 
    ///                 for one second of samples).
    LiveSound(media::MediaHandler& mh, const media::SoundInfo& info, int inPoint);

    // Pointer handling and checking functions
    const boost::int16_t*
    getDecodedData(unsigned long int pos) const {
        assert(pos < _decodedData.size());
        return reinterpret_cast<const boost::int16_t*>(_decodedData.data() + pos);
    }

    virtual bool eof() const = 0;

    unsigned int samplesFetched() const {
        return _samplesFetched;
    }

protected:

    media::AudioDecoder* decoder() const {
        return _decoder.get();
    }

    void appendDecodedData(boost::uint8_t* data, unsigned int size)
    {
        _decodedData.append(data, size);
        delete [] data; // ownership transferred...
    }


    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead() const {

        const unsigned int dds = _decodedData.size();
        if (dds <= _playbackPosition) return 0; 

        const unsigned int bytesAhead = dds - _playbackPosition;
        assert(!(bytesAhead % 2));

        const unsigned int samplesAhead = bytesAhead / 2;
        return samplesAhead;
    }

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(boost::int16_t* to, unsigned int nSamples);

    void createDecoder(media::MediaHandler& mediaHandler,
            const media::SoundInfo& info);

    /// Current playback position in the decoded stream
    size_t _playbackPosition;

    /// Number of samples fetched so far.
    unsigned long _samplesFetched;

private:

    virtual void decodeNextBlock() = 0;

    virtual bool decodingCompleted() const = 0;

    boost::scoped_ptr<media::AudioDecoder> _decoder;

    /// The decoded buffer
    SimpleBuffer _decodedData;

};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
