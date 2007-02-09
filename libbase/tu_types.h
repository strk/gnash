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

// 
//

/* $Id: tu_types.h,v 1.32 2007/02/09 17:23:03 bjacques Exp $ */

#ifndef TU_TYPES_H
#define TU_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "tu_config.h"

#if defined(_WIN32) || defined(WIN32)
#include <sdl_stdinc.h>	
//typedef unsigned char uint8_t;
//typedef char int8_t;
//typedef unsigned short int uint16_t;
//typedef short int int16_t;
//typedef unsigned int uint32_t;
//typedef int int32_t;
//typedef unsigned __int64 uint64_t;
//typedef __int64 int64_t;
# define __PRETTY_FUNCTION__ __FUNCDNAME__
# define BYTE_ORDER SDL_BYTEORDER
#else
# include <inttypes.h>

#ifndef HAVE_FUNCTION
	#ifndef HAVE_func
		#define dummystr(x) # x
		#define dummyestr(x) dummystr(x)
		#define __FUNCTION__ __FILE__":"dummyestr(__LINE__)
	#else
		#define __FUNCTION__ __func__	
	#endif
#endif

#ifndef HAVE_PRETTY_FUNCTION
	#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#endif

#ifndef BYTE_ORDER
#ifdef HAVE_ENDIAN_H
	#include <endian.h>
#elif HAVE_SYS_ENDIAN_H
	#include <sys/endian.h>
#elif HAVE_MACHINE_ENDIAN_H
	#include <machine/endian.h>
#endif
#  ifndef BYTE_ORDER
#    error BYTE_ORDER not defined by endian.h. :(
#  endif // BYTE_ORDER
#endif // BYTE_ORDER

#if ((BYTE_ORDER == LITTLE_ENDIAN) || (BYTE_ORDER == __LITTLE_ENDIAN) || (BYTE_ORDER == _LITTLE_ENDIAN))
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
DSOEXPORT bool	tu_types_validate();

#endif // TU_TYPES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
