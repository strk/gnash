// AudioDecoderFfmpeg.h: Audio decoding using the FFMPEG library.
// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

// $Id: AudioDecoderFfmpeg.h,v 1.6 2007/12/12 10:23:06 zoulunkai Exp $

#ifndef __AUDIODECODERFFMPEG_H__
#define __AUDIODECODERFFMPEG_H__

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif

extern "C" {
#include <ffmpeg/avcodec.h>
}

#include "log.h"
#include "AudioDecoder.h"

namespace gnash {
namespace media {

/// This class is used to provide an easy interface to libavcodecs audio resampler.
///
class AudioResampler
{
public:
	AudioResampler() : _context(NULL) {}
	~AudioResampler()
	{ 
		if(_context) {
			audio_resample_close (_context);
		}
	}
	
	/// Initializes the resampler
	//
	/// @param ctx
	/// The audio format container.
	///
	/// @return true if resampling is needed, if not false
	///
	bool init(AVCodecContext* ctx)
	{
		if (ctx->sample_rate != 44100 || ctx->channels != 2) {
			if (!_context) {
				_context = audio_resample_init(2,  ctx->channels, 
					44100, ctx->sample_rate);
 			}
			return true;
		}
		return false;
	}
	
	/// Resamples audio
	//
	/// @param input
	/// A pointer to the audio data that needs resampling
	///
	/// @param output
	/// A pointer to where the resampled output should be placed
	///
	/// @param samples
	/// Number of samples in the audio
	///
	/// @return the number of samples in the output data.
	///
	int resample(boost::int16_t* input, boost::int16_t* output, int samples)
	{
		return audio_resample (_context, output, input, samples);
	}

private:
	/// The container of the resample format information.
	ReSampleContext* _context;
};

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
