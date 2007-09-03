// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: tu_types.h,v 1.40 2007/09/03 22:49:58 nihilus Exp $ */

#ifndef TU_TYPES_H
#define TU_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "tu_config.h"

#if defined(_WIN32) || defined(WIN32)
//#include <sdl_stdinc.h>	
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#else
# include <boost/cstdint.hpp>

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

// A function to run some validation checks.
DSOEXPORT bool	tu_types_validate();

#endif // TU_TYPES_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
