// AudioResamplerFfmpeg.h - FFMPEG based audio resampler
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#ifndef GNASH_MEDIA_FFMPEG_AUDIORESAMPLERFFMPEG_H
#define GNASH_MEDIA_FFMPEG_AUDIORESAMPLERFFMPEG_H

#include "log.h"
#include "dsodefs.h" //For DSOEXPORT

#include "ffmpegHeaders.h"

#include <boost/cstdint.hpp>

namespace gnash {
namespace media {
namespace ffmpeg {

/// FFMPEG based AudioResampler
//
/// This class is used to provide an easy interface to libavcodecs audio resampler.
///
class AudioResamplerFfmpeg
{
public:
	DSOEXPORT AudioResamplerFfmpeg();

	DSOEXPORT ~AudioResamplerFfmpeg();
	
	/// Initializes the resampler
	//
	/// @param ctx
	/// The audio format container.
	///
	/// @return true if resampling is needed, if not false
	///
	DSOEXPORT bool init(AVCodecContext* ctx);
	
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
	DSOEXPORT int resample(
		boost::int16_t* input, boost::int16_t* output, int samples
	);

private:
	// The container of the resample format information.
	ReSampleContext* _context;
};

} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // namespace gnash


#endif // GNASH_MEDIA_FFMPEG_AUDIORESAMPLERFFMPEG_H
