// AudioDecoderFfmpeg.h: Audio decoding using the FFMPEG library.
// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_AUDIODECODERFFMPEG_H
#define GNASH_AUDIODECODERFFMPEG_H

#include "ffmpegHeaders.h"

#include "log.h"
#include "AudioDecoder.h" // for inheritance
#include "AudioResamplerFfmpeg.h" // for composition

// Forward declarations
namespace gnash {
    namespace media {
        class SoundInfo;
        class AudioInfo;
    }
}

namespace gnash {
namespace media {
namespace ffmpeg {

/// FFMPEG based AudioDecoder
class AudioDecoderFfmpeg : public AudioDecoder {
	
public:
	/// @param info
	/// 	AudioInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.
	AudioDecoderFfmpeg(const AudioInfo& info);

	/// @param info
	/// 	SoundInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.
	AudioDecoderFfmpeg(SoundInfo& info);
	~AudioDecoderFfmpeg();

    // See dox in AudioDecoder.h
	boost::uint8_t* decode(const boost::uint8_t* input,
            boost::uint32_t inputSize, boost::uint32_t& outputSize,
            boost::uint32_t& decodedBytes);

	boost::uint8_t* decode(const EncodedAudioFrame& af,
            boost::uint32_t& outputSize);

private:

	void setup(const AudioInfo& info);
	void setup(SoundInfo& info);

	boost::uint8_t* decodeFrame(const boost::uint8_t* input,
            boost::uint32_t inputSize, boost::uint32_t& outputSize);

	AVCodec* _audioCodec;
	AVCodecContext* _audioCodecCtx;
	AVCodecParserContext* _parser;

	// Use for resampling audio
	AudioResamplerFfmpeg _resampler;

    /// True if a parser is required to decode the format
    bool _needsParsing;

    /// Parse input
    //
    /// @param input
    ///     Pointer to frame we want to start parsing at.
    ///
    /// @param inputSize
    ///     Number of bytes available in input
    ///
    /// @param outFrame
    ///     Output parameter, will be set to the start
    ///     of first frame found in input.
    ///
    /// @param outFrameSize
    ///     Output parameter, will be set to size in bytes
    ///     of the first frame found.
    ///
    /// @return number of input bytes parsed, or -1 on error
    ///
    int parseInput(const boost::uint8_t* input, boost::uint32_t inputSize,
            boost::uint8_t const ** outFrame, int* outFrameSize);
};
	
} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // gnash namespace

#endif 
