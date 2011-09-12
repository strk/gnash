// AudioResamplerFfmpeg.cpp - FFMPEG based audio resampler
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#include "AudioResamplerFfmpeg.h"
#include "log.h"

#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>

namespace gnash {
namespace media {
namespace ffmpeg {

AudioResamplerFfmpeg::AudioResamplerFfmpeg()
	:_context(NULL)
{
}

AudioResamplerFfmpeg::~AudioResamplerFfmpeg()
{
  if ( _context ) {
    audio_resample_close( _context );
  }
}

bool
AudioResamplerFfmpeg::init( AVCodecContext* ctx ) 
{
  if ( (ctx->sample_rate != 44100) || (ctx->channels != 2) ) {
    if ( ! _context ) {
#if !defined (LIBAVFORMAT_VERSION_MAJOR) || LIBAVFORMAT_VERSION_MAJOR >= 53
      _context = av_audio_resample_init(
		2, ctx->channels, 44100, ctx->sample_rate,
		AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16,
		16, 10, 0, 0.8
#else
      _context = audio_resample_init(
		2, ctx->channels, 44100, ctx->sample_rate
#endif
	);
    }

    return true;
  }

  return false;
}

int
AudioResamplerFfmpeg::resample( 
				boost::int16_t* input, 
				boost::int16_t* output, 
				int samples 
			) 
{
  return audio_resample( _context, output, input, samples );
}


} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

