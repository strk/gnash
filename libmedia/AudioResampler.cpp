// AudioResampler.cpp -- custom audio resampler
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "AudioResampler.h"

#include <cstring>
#include <cassert>

namespace gnash {
namespace media {

void 
AudioResampler::convert_raw_data(
    boost::int16_t** adjusted_data,
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

    assert(sample_size == 2); // at least it seems the code relies on this...

    // simple hack to handle dup'ing mono to stereo
    if ( !stereo && m_stereo)
    {
	sample_rate >>= 1;
    }

    // simple hack to lose half the samples to get mono from stereo
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
    boost::int16_t*	out_data = new boost::int16_t[output_sample_count];
    *adjusted_data = out_data;
    *adjusted_size = output_sample_count * sizeof(boost::int16_t); // in bytes

    // Either inc > 1 (decimate the audio)
    // or dup > 1 (repeat samples)
    // or both == 1 (no transformation required)
    if (inc == 1 && dup == 1)
    {
	    // No tranformation required
	    std::memcpy(out_data, data, output_sample_count * sizeof(boost::int16_t));
    }
    else if (inc > 1)
    {
	// Downsample by skipping samples from the input
	boost::int16_t*	in = (boost::int16_t*) data;
	for (int i = output_sample_count; i > 0; i--)
	{
	    *out_data++ = *in;
	    in += inc;
	}
    }
    else if (dup > 1)
    {
	// Upsample by duplicating input samples in the output.

	// The straight sample-replication code handles mono-to-stereo (sort of)
	// and upsampling of mono but would make a botch of stereo-to-stereo
	// upsampling, giving the left sample in both channels
	// then the right sample in both channels alternately.
	// So for stereo-stereo transforms we have a stereo routine.

	boost::int16_t*	in = (boost::int16_t*) data;

	if (stereo && m_stereo) {
	    // Stereo-to-stereo upsampling: Replicate pairs of samples
	    for (int i = output_sample_count / dup / 2; i > 0; i--)
	    {
		for (int j = dup; j > 0; j--)
		{
			out_data[0] = in[0];
			out_data[1] = in[1];
			out_data += 2;
		}
		in += 2;
	    }
	} else {

	    // Linear upsampling, either to increase a sample rate
	    // or to convert a mono file to stereo or both:
	    // replicate each sample several times.
	    switch (dup)
	    {
	    case 2:
		for (int i = output_sample_count / dup; i > 0; i--)
		{
		    *out_data++ = *in;
		    *out_data++ = *in;
		    in++;
		}
		break;
	    case 4:
		for (int i = output_sample_count / dup; i > 0; i--)
		{
		    *out_data++ = *in;
		    *out_data++ = *in;
		    *out_data++ = *in;
		    *out_data++ = *in;
		    in++;
		}
		break;
	    default:
		for (int i = output_sample_count / dup; i > 0; i--)
		{
		    for (int j = dup; j > 0; j--)
		    {
			    *out_data++ = *in;
		    }
		    in++;
		}
		break;
	    }
	}
    }
}

} // namespace media
} // namespace gnash 

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
