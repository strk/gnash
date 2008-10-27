// AudioDecoderNellymoser.h: Nellymoser decoding
// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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


// This file incorporates work covered by the following copyright and permission
// notice:

/*
 * Copyright (c) 2007 a840bda5870ba11f19698ff6eb9581dfb0f95fa5,
 *                    539459aeb7d425140b62a3ec7dbf6dc8e408a306, and
 *                    520e17cd55896441042b14df2566a6eb610ed444
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef GNASH_AUDIODECODERNELLYMOSER_H
#define GNASH_AUDIODECODERNELLYMOSER_H

#include "AudioDecoder.h" // for inheritance

#define NELLY_BLOCK_LEN 64
#define NELLY_HEADER_BITS 116
#define NELLY_DETAIL_BITS 198
#define NELLY_BUF_LEN 128
#define NELLY_FILL_LEN 124
#define NELLY_BIT_CAP 6
#define NELLY_BASE_OFF 4228
#define NELLY_BASE_SHIFT 19

// Forward declarations
namespace gnash {
    namespace media {
        class AudioInfo;
        class SoundInfo;
    }
}


typedef struct nelly_handle_struct {
    float state[64];
} nelly_handle;

namespace gnash {
namespace media {

/// Audio decoding using internal Nellymoser decoder.
//
/// TODO: as ffmpeg now has Nellymoser support (maintained
/// by people who know what they're doing, I hope), do
/// we need this?
class AudioDecoderNellymoser : public AudioDecoder {

public:

    /// This is needed by gstreamer still. TODO: drop.
    AudioDecoderNellymoser();

    /// @param info
    ///     AudioInfo class with all the info needed to decode
    ///     the sound correctly. Throws a MediaException on fatal
    ///     error.
    AudioDecoderNellymoser(AudioInfo& info);

    /// @param info
    ///     SoundInfo class with all the info needed to decode
    ///     the sound correctly. Throws a MediaException on fatal
    ///      error.
    /// 
    /// @deprecated use the AudioInfo based constructor
    ///
    AudioDecoderNellymoser(SoundInfo& info);

    ~AudioDecoderNellymoser();

    // See dox in AudioDecoder.h
    boost::uint8_t* decode(boost::uint8_t* input,
        boost::uint32_t inputSize, boost::uint32_t& outputSize,
        boost::uint32_t& decodedBytes, bool parse);
    
private:

    /// @return a new[]-allocated pointer to decoded data in floats.
    float* decode(boost::uint8_t* in_buf, boost::uint32_t inputSize,
            boost::uint32_t* outputSize);


    void setup(AudioInfo& info);
    void setup(SoundInfo& info);

    // The handle used by the decoder
    nelly_handle* _nh;

    // samplerate
    boost::uint16_t _sampleRate;

    // stereo
    bool _stereo;
};
    
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIODECODERNELLYMOSER_H__
    
