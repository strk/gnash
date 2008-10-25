// ffmpegNetStreamUtil.cpp: Utility classes for use in 
// server/asobj/NetStreamFfmpeg.*
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

// #include "utility.h" // for convert_raw_data
// #include "AudioDecoderSimple.h"
// #include "AudioDecoderNellymoser.h"

#ifdef USE_FFMPEG
# include "ffmpegNetStreamUtil.h"
#endif


#include "log.h"
#include <cmath>
#include <vector>
#include <boost/scoped_array.hpp>

namespace gnash {
namespace media {

raw_mediadata_t::raw_mediadata_t()
	:
	m_stream_index(-1),
	m_size(0),
	m_data(NULL),
	m_ptr(NULL),
	m_pts(0)
{
}

raw_mediadata_t::~raw_mediadata_t()
{
  if ( m_data ) {
    delete [] m_data;
  }
}


AudioResampler::AudioResampler()
	:_context(NULL)
{
}

AudioResampler::~AudioResampler()
{
  if ( _context ) {
    audio_resample_close( _context );
  }
}

/// Initialize the resampler
//	
/// @param ctx
/// The audio format container.
///
/// @return true if resampling is needed, false if not
///
bool
AudioResampler::init( AVCodecContext* ctx ) 
{
  if ( (ctx->sample_rate != 44100) || (ctx->channels != 2) ) {
    if ( ! _context ) {
      _context = audio_resample_init( 
		2, ctx->channels, 44100, ctx->sample_rate 
	);
    }

    return true;
  }

  return false;
}

/// Resample audio
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
int
AudioResampler::resample( 
				boost::int16_t* input, 
				boost::int16_t* output, 
				int samples 
			) 
{
  return audio_resample( _context, output, input, samples );
}


} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

