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


//
// This file implements methods of the ActionScript Math class.
//
// They are all static methods; there is no Math class object as such.
//
// TODO:
//	min(), max() and pow(1) return NaN here; they should return
//	Infinity, -Infinity and 1 respectively
//


#include <cmath>
#include <string>
#include <boost/random.hpp>

#include "VM.h" // get random generator
#include "fn_call.h"
#include "Math_as.h"
#include "log.h"
#include "builtin_function.h" 
#include "Object.h" // for getObjectInterface

namespace gnash {

// Forward declarations
static void attachMathInterface(as_object& proto);
static as_value math_fabs(const fn_call& fn);	// Implements AS "abs"
static as_value math_acos(const fn_call& fn);
static as_value math_asin(const fn_call& fn);
static as_value math_atan(const fn_call& fn);
static as_value math_atan2(const fn_call& fn);
static as_value math_ceil(const fn_call& fn);
static as_value math_cos(const fn_call& fn);
static as_value math_exp(const fn_call& fn);
static as_value math_floor(const fn_call& fn);
static as_value math_log(const fn_call& fn);
static as_value math_max(const fn_call& fn);
static as_value math_min(const fn_call& fn);
static as_value math_pow(const fn_call& fn);
static as_value math_random(const fn_call& fn);
static as_value math_round(const fn_call& fn);
static as_value math_sin(const fn_call& fn);
static as_value math_sqrt(const fn_call& fn);
static as_value math_tan(const fn_call& fn);


void
math_class_init(as_object& global)
{
    // Create built-in math object. It is not a class.
	static boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
	attachMathInterface(*obj);
	global.init_member("Math", obj.get());
}

//
// Macros to wrap C math library functions as ActionScript Math methods
//

//
// One-argument simple functions.
//
// All one-argument Math functions called with no args return NaN
//

#define MATH_WRAP_FUNC1(funcname)				\
	as_value math_##funcname(const fn_call& fn)		\
	{							\
		double result;					\
		if (fn.nargs < 1) result = NaN;			\
		else {						\
			double	arg = fn.arg(0).to_number();	\
			result = std::funcname(arg);			\
		}						\
		return as_value(result);			\
	}

// Dirty it is, but what's it for? All the functions used here are
// in the standard cmath header, so there *ought* to be no need for
// a hack.
//#ifndef __GNUC__  //Some hacks are ugly and dirty, we call them 'fulhack'.
//#	undef TU_MATH_H
//#	include "tu_math.h"
//#endif

MATH_WRAP_FUNC1(fabs)
MATH_WRAP_FUNC1(acos)
MATH_WRAP_FUNC1(asin)
MATH_WRAP_FUNC1(atan)
MATH_WRAP_FUNC1(ceil)
MATH_WRAP_FUNC1(cos)
MATH_WRAP_FUNC1(exp)
MATH_WRAP_FUNC1(floor)
MATH_WRAP_FUNC1(log)
MATH_WRAP_FUNC1(sin)
MATH_WRAP_FUNC1(sqrt)
MATH_WRAP_FUNC1(tan)

// Two-argument functions.
//
// In general, two-argument functions called with no or one argument
// return NaN.
// This is always true for atan2, but there are the following exceptions:
// pow(1) == 1, max() == -Infinity and min() == Infinity
//
// Flash's pow() is clever cos it copes with negative numbers to an integral
// power, and can do pow(-2, -1) == -0.5 and pow(-2, -2) == 0.25.
// Fortunately, pow() in the cmath library works the same way.

#define MATH_WRAP_FUNC2_EXP(funcname, expr)			\
	as_value math_##funcname(const fn_call& fn)		\
	{							\
		double result;					\
		if (fn.nargs < 2) result = NaN;			\
		else {						\
			double	arg0 = fn.arg(0).to_number();	\
			double	arg1 = fn.arg(1).to_number();	\
			result = (expr);			\
		}						\
		return as_value(result);			\
	}

MATH_WRAP_FUNC2_EXP(atan2, (std::atan2(arg0, arg1)))
MATH_WRAP_FUNC2_EXP(max, (arg0 > arg1 ? arg0 : arg1))
MATH_WRAP_FUNC2_EXP(min, (arg0 < arg1 ? arg0 : arg1))
MATH_WRAP_FUNC2_EXP(pow, (std::pow(arg0, arg1)))


// A couple of oddballs.
as_value
math_random(const fn_call& /* fn */)
{

	VM::RNG& rnd = VM::get().randomNumberGenerator();

	// Produces double ( 0 <= n < 1)
	boost::uniform_real<> uni_dist(0, 1);
	boost::variate_generator<VM::RNG&, boost::uniform_real<> > uni(rnd, uni_dist);

	return as_value(uni());

}


as_value
math_round(const fn_call& fn)
{
	// round argument to nearest int. 0.5 goes to 1 and -0.5 goes to 0
	double result;

	if (fn.nargs < 1) result = NaN;
	else {
		double arg0 = fn.arg(0).to_number();
		result = std::floor(arg0 + 0.5);
	}
	return as_value(result);
}


void registerMathNative(as_object& proto)
{
    VM& vm = proto.getVM();
    
    vm.registerNative(math_fabs, 200, 0);
    vm.registerNative(math_min, 200, 1);
    vm.registerNative(math_max, 200, 2);
    vm.registerNative(math_sin, 200, 3);
    vm.registerNative(math_cos, 200, 4);
    vm.registerNative(math_atan2, 200, 5);
    vm.registerNative(math_tan, 200, 6);
    vm.registerNative(math_exp, 200, 7);
    vm.registerNative(math_log, 200, 8);
    vm.registerNative(math_sqrt, 200, 9);
    vm.registerNative(math_round, 200, 10);
    vm.registerNative(math_random, 200, 11);
    vm.registerNative(math_floor, 200, 12);
    vm.registerNative(math_ceil, 200, 13);
    vm.registerNative(math_atan, 200, 14);
    vm.registerNative(math_asin, 200, 15);
    vm.registerNative(math_acos, 200, 16);
    vm.registerNative(math_pow, 200, 17);
}

void
attachMathInterface(as_object& proto)
{

	// TODO: rely on inheritance, use init_property ?
	// All Math members are constant and non-enumerable.

    const int flags = as_prop_flags::dontDelete
                | as_prop_flags::dontEnum
                | as_prop_flags::readOnly;

	// constant
	proto.init_member("E", 2.7182818284590452354, flags);
	proto.init_member("LN2", 0.69314718055994530942, flags);
	proto.init_member("LOG2E", 1.4426950408889634074, flags);
	proto.init_member("LN10", 2.30258509299404568402, flags);
	proto.init_member("LOG10E", 0.43429448190325182765, flags);
	proto.init_member("PI", 3.14159265358979323846, flags);
	proto.init_member("SQRT1_2", 0.7071067811865475244, flags);
	proto.init_member("SQRT2", 1.4142135623730950488, flags);

    VM& vm = proto.getVM();
    
    proto.init_member("abs", vm.getNative(200, 0), flags);
    proto.init_member("min", vm.getNative(200, 1), flags);
    proto.init_member("max", vm.getNative(200, 2), flags);
    proto.init_member("sin", vm.getNative(200, 3), flags);
    proto.init_member("cos", vm.getNative(200, 4), flags);
    proto.init_member("atan2", vm.getNative(200, 5), flags);
    proto.init_member("tan", vm.getNative(200, 6), flags);
    proto.init_member("exp", vm.getNative(200, 7), flags);
    proto.init_member("log", vm.getNative(200, 8), flags);
    proto.init_member("sqrt", vm.getNative(200, 9), flags);
    proto.init_member("round", vm.getNative(200, 10), flags);
    proto.init_member("random", vm.getNative(200, 11), flags);
    proto.init_member("floor", vm.getNative(200, 12), flags);
    proto.init_member("ceil", vm.getNative(200, 13), flags);
    proto.init_member("atan", vm.getNative(200, 14), flags);
    proto.init_member("asin", vm.getNative(200, 15), flags);
	proto.init_member("acos", vm.getNative(200, 16), flags);
    proto.init_member("pow", vm.getNative(200, 17), flags);
}


} // end of gnash namespace
