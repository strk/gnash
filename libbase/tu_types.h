//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef TU_TYPES_H
#define TU_TYPES_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include "tu_config.h"
#include <stdio.h>

#include <SDL.h>

#if SDL_BYTEORDER == SDL_LIL_ENDIAN
#define _TU_LITTLE_ENDIAN_ 1
#else
#undef _TU_LITTLE_ENDIAN_
#endif // SDL_BYTEORDER == SDL_LIL_ENDIAN

typedef Uint8 uint8;
typedef Sint8 sint8;
typedef Sint8 int8;
typedef Uint16 uint16;
typedef Sint16 sint16;
typedef Sint16 int16;
typedef Uint32 uint32;
typedef Sint32 sint32;
typedef Sint32 int32;

#ifndef PROTYPES_H
typedef Uint64 uint64;
typedef Sint64 sint64;
typedef Sint64 int64;
#endif

// A function to run some validation checks.
bool	tu_types_validate();


#endif // TU_TYPES_H


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
