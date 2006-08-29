// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
#ifndef TU_TYPES_H
#define TU_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "tu_config.h"

#if defined(_WIN32) || defined(WIN32)
typedef unsigned char uint8_t;
typedef char int8_t;
typedef unsigned short int uint16_t;
typedef short int int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
# define __PRETTY_FUNCTION__ __FUNCDNAME__
# define BYTE_ORDER SDL_BYTEORDER
#else
# include <inttypes.h>
#endif

#ifndef __FUNCTION__
	#undef dummystr
	#undef dummyestr
	#define dummystr(x) # x
	#define dummyestr(x) dummystr(x)
	#define __FUNCTION__ __FILE__":"dummyestr(__LINE__)
	#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#ifndef BYTE_ORDER
#if defined(__sgi) || defined(SGI) || defined(__sgi__)
	#include <sys/endian.h>
#else
	#include <endian.h>
#endif
#  ifndef BYTE_ORDER
#    error BYTE_ORDER not defined by endian.h. :(
#  endif // BYTE_ORDER
#endif // BYTE_ORDER

#if ((BYTE_ORDER == __LITTLE_ENDIAN)||(BYTE_ORDER == _LITTLE_ENDIAN))
#	define _TU_LITTLE_ENDIAN_ 1
#else
#	undef _TU_LITTLE_ENDIAN_
#endif //BYTE_ORDER == SDL_LIL_ENDIAN

typedef uint8_t uint8;
typedef int8_t sint8;
typedef int8_t int8;
typedef uint16_t uint16;
typedef int16_t sint16;
typedef int16_t int16;
typedef uint32_t uint32;
typedef int32_t sint32;
typedef int32_t int32;

#ifndef PROTYPES_H
typedef uint64_t uint64;
typedef int64_t sint64;
typedef int64_t int64;
#endif

// A function to run some validation checks.
bool	tu_types_validate();

#endif // TU_TYPES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
