// LiveSound.h: - base class for embedded sound handling, for gnash
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

#ifndef SOUND_LIVESOUND_H
#define SOUND_LIVESOUND_H

#include <memory>
#include <cassert>
#include <cstdint> // For C99 int types
#include <iostream>

#include "InputStream.h" 
#include "AudioDecoder.h" 
#include "SimpleBuffer.h" 

// Forward declarations
namespace gnash {
    namespace media {
        class MediaHandler;
        class SoundInfo;
    }
}

namespace gnash {
namespace sound {


/// Maintains a collection of SimpleBuffers, providing stateful sequential
/// read access to the data contained therein.
//
// TODO: this shares some functionality with CursoredBuffer, and the two
// classes might be merged.
class Buffers {
public:
    Buffers(size_t in_point)
    : _buffers(),
      _index(0),
      _pos(0),
      _consumed(0),
      _in_point(in_point)
    {}

    Buffers(const Buffers&) = delete;
    Buffers& operator=(const Buffers&) = delete;

    /// Append a buffer of data to be read by the consumer later.
    void append(SimpleBuffer buf) {
        _buffers.push_back(std::move(buf));
        consumeInPoint();
    }

    void restart()
    {
        _index = 0;
        _consumed = 0;
        consumeInPoint();
    }

    /// Copy up to the given number of bytes to the given buffer.
    //
    /// @to points to a buffer to be written to.
    /// @bytes number of bytes to be written.
    /// @return number of bytes actually written.
    size_t copy(std::uint8_t* to, size_t bytes) {
        assert(_consumed >= _in_point);

        size_t bytes_remaining = bytes;

        for (; _index < _buffers.size(); ++_index) {
            const SimpleBuffer& buffer = _buffers[_index];

            size_t to_copy = std::min(bytes_remaining, buffer.size() - _pos);

            std::copy(buffer.data() + _pos, buffer.data() + _pos + to_copy, to);
            to += to_copy;
            bytes_remaining -= to_copy;
            _pos += to_copy;

            if (_pos == buffer.size()) {
                ++_index;
                _pos = 0;
                break;
            }

            if (bytes_remaining == 0) {
                break;
            }
        }

        size_t written = bytes - bytes_remaining;
        _consumed += written;
        return written;
    }

    /// @return total number of bytes contained.
    std::uint64_t countBytes() const
    {
        std::uint64_t bytes = 0;
        for (const SimpleBuffer& buffer : _buffers) {
            bytes += buffer.size();
        }
        return bytes;
    }

    /// @return number of bytes previously copied by calls to copy().
    std::uint64_t consumed() const
    {
        return std::max(_consumed, _in_point);
    }

private:
    void consumeInPoint() {
        if (_consumed >= _in_point) {
            return;
        }
        size_t inPoint = _in_point;

        for (const SimpleBuffer& buffer : _buffers) {
            size_t advance = std::min(inPoint, buffer.size());
            if (advance == buffer.size()) {
                ++_index;
                inPoint -= advance;
            } else {
                _pos = advance;
                break;
            }
        }
        _consumed = _in_point;
    }

    std::vector<SimpleBuffer> _buffers;
    /// Zero-based index of the buffer currently being indicated.
    size_t _index;
    /// Current position inside the current buffer.
    size_t _pos;
    /// Total bytes consumed by calls to copy().
    std::uint64_t _consumed;
    /// Number of bytes to skip from the input.
    size_t _in_point;
};

/// Instance of a defined %sound (LiveSoundData)
//
/// This class contains a pointer to the LiveSoundData used for playing
/// and a SimpleBuffer to use when decoding is needed.
class LiveSound : public InputStream
{
protected:

    /// Create an embedded %sound instance
    //
    /// @param mh       The MediaHandler to use for on-demand decoding
    /// @param inPoint  Offset in output samples this instance should start
    ///                 playing from. These are post-resampling samples (44100 
    ///                 for one second of samples).
    /// @param info     The media::SoundInfo for this sound.
    LiveSound(media::MediaHandler& mh, const media::SoundInfo& info,
            size_t inPoint);

    /// Called when more decoded sound data is required.
    //
    /// This will be called whenever no more decoded data is available
    /// but decoding is not complete.
    virtual bool moreData() = 0;

    /// True if there is no more data ever.
    //
    /// The InputStream will be disconnected when this is true.
    virtual bool eof() const = 0;

    /// Start from the beginning again.
    void restart() {
        _samplesFetched = 0;
        _decodedBuffers.restart();
    }

    /// How many samples have been fetched since the beginning
    //
    /// Note that this is reset on each loop.
    unsigned int samplesFetched() const {
        return _samplesFetched;
    }

    std::uint64_t playbackPosition() const {
        return _decodedBuffers.consumed();
    }

    media::AudioDecoder& decoder() const {
        return *_decoder;
    }

    void appendDecodedData(SimpleBuffer data) {
        _decodedBuffers.append(std::move(data));
    }

    /// Return number of already-decoded samples available
    /// from playback position on
    unsigned int decodedSamplesAhead() const {

        const unsigned int dds = _decodedBuffers.countBytes();
        if (dds <= playbackPosition()) return 0;

        size_t bytesAhead = dds - playbackPosition();
        bytesAhead = checkEarlierEnd(bytesAhead, playbackPosition());

        assert(!(bytesAhead % 2));

        const unsigned int samplesAhead = bytesAhead / 2;
        return samplesAhead;
    }

private:

    /// Check if the sound data ends earlier than expected.
    //
    /// This is a way to deal with the outpoint in EmbedSoundInst, but isn't
    /// very tidy.
    virtual size_t checkEarlierEnd(size_t left, size_t) const {
        return left;
    }

    // See dox in sound_handler.h (InputStream)
    unsigned int fetchSamples(std::int16_t* to, unsigned int nSamples);

    void createDecoder(media::MediaHandler& mediaHandler,
            const media::SoundInfo& info);

    virtual bool decodingCompleted() const = 0;

    /// Number of samples fetched so far.
    unsigned long _samplesFetched;

    std::unique_ptr<media::AudioDecoder> _decoder;

    /// The decoded buffers
    Buffers _decodedBuffers;

};


} // gnash.sound namespace 
} // namespace gnash

#endif // SOUND_EMBEDSOUNDINST_H
