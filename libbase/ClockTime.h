// Time.h: clock and local time functions for Gnash
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
//

#ifndef GNASH_TIME_H
#define GNASH_TIME_H

#include <cstdint>
#include "dsodefs.h"

namespace gnash {

namespace clocktime {
    /// Wall clock timer, returns current POSIX time in milliseconds.
	DSOEXPORT std::uint64_t getTicks();

	/// Returns the offset between actual clock time and UTC.
	/// It relies on the system's time zone settings, so
	/// cannot be regarded as reliable.
	DSOEXPORT std::int32_t getTimeZoneOffset(double time);

} // namespace clocktime
} // namespace gnash

#endif
