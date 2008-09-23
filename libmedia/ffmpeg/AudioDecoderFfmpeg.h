// AudioDecoderFfmpeg.h: Audio decoding using the FFMPEG library.
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


#ifndef GNASH_AUDIODECODERFFMPEG_H
#define GNASH_AUDIODECODERFFMPEG_H

// TODO: What's this for ?
#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#include "log.h"
#include "AudioDecoder.h"
#include "ffmpegNetStreamUtil.h"
#include "ffmpegHeaders.h"

namespace gnash {
namespace media {

class AudioDecoderFfmpeg : public AudioDecoder {
	
public:
	/// @param info
	/// 	AudioInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.
	AudioDecoderFfmpeg(AudioInfo& info);

	/// @param info
	/// 	SoundInfo class with all the info needed to decode
	///     the sound correctly. Throws a MediaException on fatal
	///     error.
	AudioDecoderFfmpeg(SoundInfo& info);
	~AudioDecoderFfmpeg();

	boost::uint8_t* decode(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedBytes, bool parse);

	boost::uint8_t* decode(const EncodedAudioFrame& af, boost::uint32_t& outputSize);

private:

	void setup(AudioInfo& info);
	void setup(SoundInfo& info);

	boost::uint8_t* decodeFrame(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize);

	AVCodec* _audioCodec;
	AVCodecContext* _audioCodecCtx;
	AVCodecParserContext* _parser;

	// Use for resampling audio
	AudioResampler _resampler;
};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIODECODERFFMPEG_H__
