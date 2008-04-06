// Time.h: clock and local time functions for Gnash
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#ifndef GNASH_TIME_H
#define GNASH_TIME_H

#include <boost/cstdint.hpp>
#include "dsodefs.h"

namespace clocktime
{
    /// Wall clock timer, uses highest available resolution.
    /// Generally microseconds on Linux / Unix, microseconds with
    /// millisecond resolution on Windows. Nanosecond resolution is
    /// theoretically available.
	DSOEXPORT boost::uint64_t getTicks();

	/// Converts ticks to seconds.
	DSOEXPORT double ticksToSeconds(boost::uint64_t ticks);

	/// Returns the offset between actual clock time and UTC.
	/// It relies on the system's time zone settings, so
	/// cannot be regarded as reliable.
	DSOEXPORT boost::int32_t getTimeZoneOffset(double time);

}

#endif
