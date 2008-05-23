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


#ifndef __AUDIODECODERFFMPEG_H__
#define __AUDIODECODERFFMPEG_H__

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

#ifdef HAVE_FFMPEG_AVCODEC_H
extern "C" {
# include "ffmpeg/avcodec.h"
}
#endif

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
extern "C" {
# include "libavcodec/avcodec.h"
}
#endif

#include "log.h"
#include "AudioDecoder.h"
#include "ffmpegNetStreamUtil.h"

namespace gnash {
namespace media {

class AudioDecoderFfmpeg : public AudioDecoder {
	
public:
	AudioDecoderFfmpeg();
	~AudioDecoderFfmpeg();

	bool setup(AudioInfo* info);
	bool setup(SoundInfo* info);

	boost::uint8_t* decode(boost::uint8_t* input, boost::uint32_t inputSize, boost::uint32_t& outputSize, boost::uint32_t& decodedBytes, bool parse);

private:

	AVCodec* _audioCodec;
	AVCodecContext* _audioCodecCtx;
	AVCodecParserContext* _parser;

	// Use for resampling audio
	AudioResampler _resampler;
};
	
} // gnash.media namespace 
} // gnash namespace

#endif // __AUDIODECODERFFMPEG_H__
