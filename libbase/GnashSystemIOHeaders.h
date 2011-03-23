// GnashImageSystemIOHeaders.h: Compatibility IO header for Gnash.
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
//
//

/// This file should be included for:
//
/// read()
/// write()

#ifndef GNASH_IO_HEADERS_H
#define GNASH_IO_HEADERS_H

/// Always include unistd.h unless compiling under MSVC++
//
/// This isn't always necessary on GNU/Linux under gcc, but it's still good
/// practice to include it.
//
/// @todo A configure 'test' (a #define in gnashconfig.h) might be a better
///       way of checking for compiler.

#if !defined(_MSC_VER)
# include <unistd.h>
#else
#include <io.h>
#endif

#endif
