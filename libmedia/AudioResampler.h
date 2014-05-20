// AudioResampler.h -- custom audio resampler
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#ifndef __GNASH_UTIL_H
#define __GNASH_UTIL_H

#include <cstdint> // for std::int16_t

namespace gnash {
namespace media {


/// VERY crude audio resampler
class AudioResampler {

public:

	/// VERY crude sample-rate and stereo conversion.
	//
	/// Converts input data to output format.
	//
	/// @param adjusted_data
	/// Where the converted data is placed (output). WARNING: even though
	/// the type of the output data is int16, the adjusted_size output
	/// parameter is in bytes.
	///
	/// @param adjusted_size
	/// The size of the converted data (output) in bytes.
	///
	/// @param data
	/// Data that needs to be converted (input).
	///
	/// @param sample_count
	/// The datas current sample count (input).
	/// 
	/// @param sample_size
	/// The datas current sample size (input) in bytes.
	///
	/// @param sample_rate
	/// The datas current sample rate (input).
	///
	/// @param stereo
	/// Whether the current data is in stereo (input).
	///
	/// @param m_sample_rate
	/// The samplerate we which to convert to (output).
	///
	/// @param m_stereo
	/// Do we want the output data to be in stereo (output)?
	static void convert_raw_data(std::int16_t** adjusted_data,
		  int* adjusted_size, void* data, int sample_count,
		  int sample_size, int sample_rate, bool stereo,
		  int m_sample_rate, bool m_stereo);
};

} // namespace media
} // namespace gnash

#endif // __GNASH_UTIL_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
