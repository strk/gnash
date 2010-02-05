// GnashSleep.h -- How do you sleep?
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_SLEEP_H
#define GNASH_SLEEP_H

// Headers for sleep
#if defined(_WIN32) || defined(WIN32)
# include <windows.h>
#else
# include <unistd.h>
#endif

namespace gnash {

/// Sleep compatibly for the specified number of microseconds
//
/// @param useconds     microseconds to sleep.
inline void gnashSleep(size_t useconds)
{
#if defined(_WIN32) || defined(WIN32)
    Sleep(useconds / 1000);
#else
    usleep(useconds);
#endif
}

} // namespace gnash

#endif
