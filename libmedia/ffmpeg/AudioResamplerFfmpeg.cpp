// AudioResamplerFfmpeg.cpp - FFMPEG based audio resampler
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
//

#include "AudioResamplerFfmpeg.h"
#include "utility.h"
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
AudioResamplerFfmpeg::~AudioResamplerFfmpeg() {
    if (_context) {
#ifdef HAVE_SWRESAMPLE_H
        swr_free(&_context);
#elif HAVE_AVRESAMPLE_H
        avresample_close(_context);
        avresample_free(&_context);
#else
        audio_resample_close(_context);
#endif
    }
}

bool
AudioResamplerFfmpeg::init(AVCodecContext* ctx) {
    if ((ctx->sample_rate != 44100) ||
#if defined(HAVE_SWRESAMPLE_H) || defined(HAVE_AVRESAMPLE_H)
        (ctx->sample_fmt != AV_SAMPLE_FMT_S16) ||
#endif
        (ctx->channels != 2)) {
        if (! _context) {
#ifdef HAVE_SWRESAMPLE_H
            _context = swr_alloc();
#elif HAVE_AVRESAMPLE_H
            _context = avresample_alloc_context();
#else
            _context = av_audio_resample_init(2, ctx->channels,
                44100, ctx->sample_rate,
                AV_SAMPLE_FMT_S16, AV_SAMPLE_FMT_S16,
                16, 10, 0, 0.8);
#endif
#if defined(HAVE_SWRESAMPLE_H) || defined(HAVE_AVRESAMPLE_H)
            av_opt_set_int(_context, "in_channel_layout",
                av_get_default_channel_layout(ctx->channels), 0);
            av_opt_set_int(_context, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
            av_opt_set_int(_context, "in_sample_rate", ctx->sample_rate, 0);
            av_opt_set_int(_context, "out_sample_rate", 44100, 0);
            av_opt_set_int(_context, "in_sample_fmt", ctx->sample_fmt, 0);
            av_opt_set_int(_context, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
#endif
#ifdef HAVE_SWRESAMPLE_H
            swr_init(_context);
#elif HAVE_AVRESAMPLE_H
            avresample_open(_context);
#endif
        }
        return true;
    }
    return false;
}

int
AudioResamplerFfmpeg::resample(boost::uint8_t** input, int plane_size,
    int samples, boost::uint8_t** output) {
#ifdef HAVE_SWRESAMPLE_H
    return swr_convert(_context,
        output, MAX_AUDIO_FRAME_SIZE,
        const_cast<const uint8_t**>(input), samples);
#elif HAVE_AVRESAMPLE_H
    return avresample_convert(_context,
        output, 0, MAX_AUDIO_FRAME_SIZE,
        input, plane_size, samples);
#else
    UNUSED( plane_size );
    return audio_resample(_context, reinterpret_cast<short*>(*output),
        reinterpret_cast<short*>(*input), samples); 
#endif
}

} // gnash.media.ffmpeg namespace 
} // gnash.media namespace 
} // namespace gnash

// Local Variables:
// mode: C++
// End:

