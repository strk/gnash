// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifndef __GMATH_H__
#define __GMATH_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "impl.h"
#include "as_object.h"
#ifdef WIN32
# undef min
# undef max
#endif

#ifndef __GNUC__
	#undef abs
	#undef acos
	#undef asin
	#undef atan
	#undef atan2
	#undef ceil
	#undef cos
	#undef exp
	#undef floor
	#undef log
	#undef max
	#undef min
	#undef pow
	#undef random
	#undef round
	#undef sin
	#undef sqrt
	#undef tan
#endif

namespace gnash {

// Math isn't a proper class, so doesn't need a constructor.

void math_class_init(as_object& global);

} // end of gnash namespace

// __GMATH_H__
#endif

