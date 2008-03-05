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


#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "tu_config.h"

#ifdef HAVE_STRINGCASECMP
# define STRCASECMP strcasecmp
#else
# define STRCASECMP _stricmp
#endif

// FIXME: This ugly hack is for NetBSD, which seems to have a
// preprocessor problem, and won't define anything sensible like
// NETBSD we can use. Basically the problem is NetBSD has two thread
// implementations. One if the older pth library in /usr/pkg, and the
// other (the one we want to use) is /usr/pkg/phtread. Even with the
// corrent paths supplied, this one file barfs with GCC 3.3.3 on
// NetBSD, so screw it, and just hack it for now. We hope this entire
// file will be gone soon anyway.
#if !defined(HAVE_WINSOCK_H) || defined(__OS2__)
#define _LIB_PTHREAD_ 1
#ifndef _LIB_PTHREAD_TYPES_H
#  define _LIB_PTHREAD_TYPES_H 1
#endif

#include <sys/types.h>

// This screws up MingW
#if 0
#include <ctime>
// And what's the point?
clock_t clock __P((void));
size_t strftime __P((char *, size_t, const char *, const struct tm *));
#endif // 0

#endif // ! HAVE_WINSOCK_H

//#include "tu_config.h"
#include "utility.h"
#include <cstddef>
#include <cstring>	// for strcmp and friends
//#include <new>	// for placement new
#include <vector>

namespace gnash {}

#if defined(_WIN32) || defined(WIN32)
#pragma warning(disable : 4345)	// in MSVC 7.1, warning about placement new POD default initializer
#endif // _WIN32


#endif // __CONTAINER_H__

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
