// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
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
  
class Math {
public:
    Math();
    ~Math();
   void abs();
   void acos();
   void asin();
   void atan();
   void atan2();
   void ceil();
   void cos();
   void exp();
   void floor();
   void log();
   void max();
   void min();
   void pow();
   void random();
   void round();
   void sin();
   void sqrt();
   void tan();
private:
    bool _E;
    bool _LN2;
    bool _LN10;
    bool _LOG2E;
    bool _LOG10E;
    bool _PI;
    bool _SQRT1_2;
    bool _SQRT2;
};

class math_as_object : public as_object
{
private:
	//Math obj;

public:
	math_as_object();
};

void math_class_init(as_object& global);

} // end of gnash namespace

// __GMATH_H__
#endif

