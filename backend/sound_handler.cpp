// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#include "sound_handler.h"

namespace gnash {

// VERY crude sample-rate conversion.
// Converts input data to output format.
// sample_size

void sound_handler::convert_raw_data(
	int16_t** adjusted_data,
	int* adjusted_size,
	void* data,
	int sample_count,	// A stereo pair counts as one
	int sample_size,	// Should now always == 2
	// sample_rate and stereo are those of the incoming sample
	int sample_rate, 
	bool stereo,
	// m_sample_rate, m_stereo are the format we must convert to.
	int m_sample_rate,
	bool m_stereo)
{
	// simple hack to handle dup'ing mono to stereo
	if ( !stereo && m_stereo)
	{
		sample_rate >>= 1;
	}

	// simple hack to lose half the samples to get mono from stereo
	// Unfortunately, this gives two copies of the left channel.
	if ( stereo && !m_stereo)
	{
		sample_rate <<= 1;
	}

	// Brain-dead sample-rate conversion: duplicate or
	// skip input samples an integral number of times.
	int	inc = 1;	// increment
	int	dup = 1;	// duplicate
	if (sample_rate > m_sample_rate)
	{
		inc = sample_rate / m_sample_rate;
	}
	else if (sample_rate < m_sample_rate)
	{
		dup = m_sample_rate / sample_rate;
	}

	int	output_sample_count = (sample_count * dup * (stereo ? 2 : 1)) / inc;
	int16_t*	out_data = new int16_t[output_sample_count];
	*adjusted_data = out_data;
	*adjusted_size = output_sample_count * 2;	// 2 bytes per sample

	if (inc == 1 && dup == 1) {
		// Speed up no-op case
		memcpy(out_data, data, output_sample_count * sizeof(int16_t));
	} else {
		// crude sample rate conversion.
		int16_t*	in = (int16_t*) data;
		for (int i = 0; i < output_sample_count; i += dup)
		{
			int16_t	val = *in;
			for (int j = 0; j < dup; j++)
			{
				*out_data++ = val;
			}
			in += inc;
		}
	}
}


} // namespace gnash


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
