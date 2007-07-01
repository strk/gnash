// sound_handler_mp3.cpp: Audio output via libmad (MP3), for Gnash.
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
//

// Based on sound_handler_mp3.cpp by "tbp", 2003
// which has been donated to the Public Domain.

/*
 * Some brain damaged helpers to decode mp3 streams for use in
 * a gnash::sound_handler that uses SDL for output.
 * (even comments are cut&paste compliant)
 */

/* $Id: sound_handler_mp3.cpp,v 1.7 2007/07/01 10:53:49 bjacques Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_MAD_H
#error "You need to have the libmad development package installed\
to compile this file. You can either reconfigure without --enable-mp3,\
 or install libmad0-dev (using apt-get) or libmad (using yum)."
#else

#include "gnash.h"
#include "container.h"
#include "log.h"

#include <cstdio>

#include <mad.h>
#ifdef _MSC_VER
	#pragma comment(lib, "libmad")
	#define snprintf _snprintf
#endif

using gnash::log_error;
using gnash::log_msg;

namespace mad_helpers {
	static const char *parse_layer(const mad_header& h)
	{
		switch(h.layer) {
			case MAD_LAYER_I:	return "I";
			case MAD_LAYER_II:	return "II";
			case MAD_LAYER_III:	return "III";
			default: return "bogus";
		}
	}

	static const char *parse_channel_mode(const mad_header& h)
	{
		switch (h.mode) {
			case MAD_MODE_SINGLE_CHANNEL:
				return "single channel";
			case MAD_MODE_DUAL_CHANNEL:
				return "dual channel";
			case MAD_MODE_JOINT_STEREO:
				return "joint (MS/intensity) stereo";
			case MAD_MODE_STEREO:
				return "normal LR stereo";
			default:
				return "bogus";
		}
	}

	static const char *parse_emphasis(const mad_header& h)
	{
		switch (h.emphasis) {
			case MAD_EMPHASIS_NONE:
				return "none";
			case MAD_EMPHASIS_50_15_US:
				return "50/15 us";
			case MAD_EMPHASIS_CCITT_J_17:
				return "CCITT J.17";
			default:
				return "bogus";
		}
	}

#if 1
	static const char* parse_frame_info(const mad_header& h)
	{
		static char buf[1024];
		size_t len = 1024;

		snprintf(buf, len, "%lu kb/s audio mpeg layer %s "
			"stream crc [%s] mode '%s' with '%s' "
			"emphasis at %u Hz sample rate",
			h.bitrate, parse_layer(h),
			(h.flags&MAD_FLAG_PROTECTION) ? "X" : " ",
			parse_channel_mode(h), parse_emphasis(h),
			h.samplerate);
		buf[len-1] = 0;
		return buf;
	}
#endif
  
	template <const unsigned int stride> static void pcm_fixed_to_native(const mad_fixed_t *src, int16_t *dst, const unsigned int count)
	{
		assert(count > 0);
		unsigned int 
			dec = count,
			idx = 0;			
		do {
			dst[idx*stride] = src[idx] >> (MAD_F_FRACBITS-15); // no fancy dithering...
			++idx;
		} while (--dec);
	}

}

// some intermediary buffer to hold a frame worth of samples
// fugly.
class pcm_buff_t {
public:
	//enum { frame_payload = 1152 };
	int16_t *samples;
	unsigned int count;

	~pcm_buff_t() {
		delete samples;
	}

	// from mad fixed point to native 16 bits in a temp. buffer
	unsigned int transmogrify(const mad_synth &synth, const bool stereo) {
		count = synth.pcm.length;
		if (stereo) {
			samples = new int16_t[count*2];
			mad_helpers::pcm_fixed_to_native<2>(&synth.pcm.samples[0][0], &samples[0], count);
			mad_helpers::pcm_fixed_to_native<2>(&synth.pcm.samples[1][0], &samples[1], count);
			return count * 2;
		}
		else {
			samples = new int16_t[count];
			mad_helpers::pcm_fixed_to_native<1>(&synth.pcm.samples[0][0], samples, count);
			return count;
		}
	}

	void *collate(void *p, const bool stereo) const
	{
		const unsigned int bytes = count * (stereo ? 2 : 1) * sizeof(int16_t);
		memcpy(p, samples, bytes);
		return (void *) (((char *)p) + bytes); // geez
	}
};

// there's quite some (useless) copying around since there's no infrastructure
// for streaming and we need to decode it all at once
void convert_mp3_data(int16_t **adjusted_data, int *adjusted_size, void *data,
		const int sample_count, const int /*sample_size*/,
		const int sample_rate, const bool stereo)
{

	//log_msg("convert_mp3_data sample count %d rate %d stereo %s", sample_count, sample_rate, stereo?"yes":"no");

	mad_stream	stream;
	mad_frame	frame;
	mad_synth	synth;
	mad_timer_t	timer;

	mad_stream_init(&stream);
	mad_frame_init(&frame);
	mad_synth_init(&synth);
	mad_timer_reset(&timer);

	// decode stream
	mad_stream_buffer(&stream, (const unsigned char *)data, sample_count);
	stream.error = MAD_ERROR_NONE;

	// decode frames
	unsigned int fc = 0, total = 0;
	std::vector<pcm_buff_t *> out; // holds decoded frames

	while (true)
	{
		if (mad_frame_decode(&frame, &stream)) {
			// there's always some garbage in front of the buffer
			// so i guess, it's not so garbagish. anyway, skip.
			if (fc == 0 && stream.error == MAD_ERROR_LOSTSYNC)
			{
				continue;
			}
			else
			{
				// kludge as we stop decoding on LOSTSYNC.
				if (stream.error != MAD_ERROR_LOSTSYNC)
				{
					log_error(_("** MP3 frame error: %s"), mad_stream_errorstr(&stream));
				}
				break;
			}
		}

		if (fc == 0)
		{
			log_msg("%s", mad_helpers::parse_frame_info(frame.header));
		}

		++fc;
		mad_timer_add(&timer,frame.header.duration);

		mad_synth_frame(&synth,&frame);
		pcm_buff_t *pcm = new pcm_buff_t;
		total += pcm->transmogrify(synth, stereo);
		out.push_back(pcm);
	}

	if (total == 0) goto cleanup; // yay

	log_msg("decoded frames %d bytes %d (diff %d) -- original rate %d", fc, total, total - sample_count, sample_rate);

	// assemble together all decoded frames. another round of memcpy.
	{
		void *p = new int16_t[total];
		*adjusted_data = (int16_t*) p;
		*adjusted_size = total * sizeof(int16_t);
		// stuff all that crap together
		{for (unsigned int i=0; i<out.size(); ++i)
			p = out[i]->collate(p,stereo);
		}
	}

cleanup:
	{for (unsigned int i=0; i<out.size(); ++i) delete out[i]; }
	mad_synth_finish(&synth);
	mad_frame_finish(&frame);
	mad_stream_finish(&stream);
}

// HAVE_MAD_H
#endif

