// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "VM.h" // get random generator
#include "fn_call.h"
#include "Global_as.h"
#include "Math_as.h"
#include "log.h"
#include "NativeFunction.h" 
#include "GnashNumeric.h"

#include <cmath>
#include <string>
#include <boost/random.hpp>

namespace gnash {

// Forward declarations
namespace {
    typedef double (*UnaryMathFunc) (double);
    typedef double (*BinaryMathFunc) (double, double);

    void attachMathInterface(as_object& proto);
    as_value math_max(const fn_call& fn);
    as_value math_min(const fn_call& fn);
    as_value math_random(const fn_call& fn);

    // There's no std::round, but Math.round behaves like all the other
    // unary functions.
    inline double round(double d);

    template<UnaryMathFunc Func> as_value unaryFunction(const fn_call& fn);
    template<BinaryMathFunc Func> as_value binaryFunction(const fn_call& fn);
    template<> as_value binaryFunction<std::pow>(const fn_call& fn);

}


void
registerMathNative(as_object& proto)
{
    VM& vm = getVM(proto);
    
    vm.registerNative(unaryFunction<std::abs>, 200, 0);
    vm.registerNative(math_min, 200, 1);
    vm.registerNative(math_max, 200, 2);
    vm.registerNative(unaryFunction<std::sin>, 200, 3);
    vm.registerNative(unaryFunction<std::cos>, 200, 4);
    vm.registerNative(binaryFunction<std::atan2>, 200, 5);
    vm.registerNative(unaryFunction<std::tan>, 200, 6);
    vm.registerNative(unaryFunction<std::exp>, 200, 7);
    vm.registerNative(unaryFunction<std::log>, 200, 8);
    vm.registerNative(unaryFunction<std::sqrt>, 200, 9);
    vm.registerNative(unaryFunction<round>, 200, 10);
    vm.registerNative(math_random, 200, 11);
    vm.registerNative(unaryFunction<std::floor>, 200, 12);
    vm.registerNative(unaryFunction<std::ceil>, 200, 13);
    vm.registerNative(unaryFunction<std::atan>, 200, 14);
    vm.registerNative(unaryFunction<std::asin>, 200, 15);
    vm.registerNative(unaryFunction<std::acos>, 200, 16);
    vm.registerNative(binaryFunction<std::pow>, 200, 17);
}


void
math_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinObject(where, attachMathInterface, uri);
}

namespace {

//
// One-argument simple functions.
//
// All one-argument Math functions called with no args return NaN
//

// If it is called with two arguments, the valueOf method
// of the second method is called, but not used. Strange, but true.
template<UnaryMathFunc Func>
as_value
unaryFunction(const fn_call& fn)
{
    if (fn.nargs < 1) return as_value(NaN);
    double arg = toNumber(fn.arg(0), getVM(fn));	
    if (fn.nargs > 1) toNumber(fn.arg(1), getVM(fn));
    return as_value(Func(arg));
}

/// Two-argument functions.
//
/// As a rule, two-argument functions called with no or one argument
/// return NaN. However, three of the four functions break this rule in
/// different ways. Exceptions are described below.
//
/// There is no real need for this template at present, as it only handles
/// Math.atan2. But it might be useful if other Math functions are added.
template<BinaryMathFunc Func>
as_value
binaryFunction(const fn_call& fn)
{
    if (fn.nargs < 2) return as_value(NaN);
    double arg0 = toNumber(fn.arg(0), getVM(fn));	
    double arg1 = toNumber(fn.arg(1), getVM(fn));
    return as_value(Func(arg0, arg1));
}

/// Math.pow
//
/// Math.pow is odd in that Math.pow(1) returns 1. All other single-argument
/// calls return NaN.
template<>
as_value
binaryFunction<std::pow>(const fn_call& fn)
{
    if (!fn.nargs) return as_value(NaN);
    
    double arg0 = toNumber(fn.arg(0), getVM(fn));

    if (fn.nargs < 2) {
        if (arg0 == 1) return as_value(1);
        return as_value(NaN);
    }

    double arg1 = toNumber(fn.arg(1), getVM(fn));
    return as_value(isFinite(arg0) ? std::pow(arg0, arg1) : NaN );
}

/// Math.min
//
/// Math.min() returns Infinity
as_value
math_min(const fn_call& fn)
{
    if (!fn.nargs) return as_value(std::numeric_limits<double>::infinity());

	if (fn.nargs < 2) return as_value(NaN);

	double arg0 = toNumber(fn.arg(0), getVM(fn));
	double arg1 = toNumber(fn.arg(1), getVM(fn));

	if (isNaN(arg0) || isNaN(arg1))
	{
		return as_value(NaN);
	}

	return as_value(std::min(arg0, arg1));

}

/// Math.min
//
/// Math.max() returns -Infinity
as_value
math_max(const fn_call& fn)
{
    if (!fn.nargs) return as_value(-std::numeric_limits<double>::infinity());
	
    if (fn.nargs < 2) return as_value(NaN);

	double arg0 = toNumber(fn.arg(0), getVM(fn));
	double arg1 = toNumber(fn.arg(1), getVM(fn));

	if (isNaN(arg0) || isNaN(arg1))
	{
		return as_value(NaN);
	}

	return as_value(std::max(arg0, arg1));
}

/// Math.random()
//
/// The first two arguments are converted to numbers, even though
/// neither is used.
as_value
math_random(const fn_call& fn)
{

    if (fn.nargs) toNumber(fn.arg(0), getVM(fn));
    if (fn.nargs > 1) toNumber(fn.arg(1), getVM(fn));

	VM::RNG& rnd = getVM(fn).randomNumberGenerator();

	// Produces double ( 0 <= n < 1)
	boost::uniform_real<> uni_dist(0, 1);
	boost::variate_generator<VM::RNG&, boost::uniform_real<> > uni(
            rnd, uni_dist);

	return as_value(uni());

}

inline double
round(double d)
{
    return std::floor(d + 0.5);
}

void
attachMathInterface(as_object& proto)
{
	// TODO: rely on inheritance, use init_property ?
	// All Math members are constant and non-enumerable.

    const int flags = PropFlags::dontDelete
                | PropFlags::dontEnum
                | PropFlags::readOnly;

	// constant
    proto.init_member("E", 2.7182818284590452354, flags);
    proto.init_member("LN2", 0.69314718055994530942, flags);
    proto.init_member("LOG2E", 1.4426950408889634074, flags);
    proto.init_member("LN10", 2.30258509299404568402, flags);
    proto.init_member("LOG10E", 0.43429448190325182765, flags);
    proto.init_member("PI", 3.14159265358979323846, flags);
    proto.init_member("SQRT1_2", 0.7071067811865475244, flags);
    proto.init_member("SQRT2", 1.4142135623730950488, flags);
    
    VM& vm = getVM(proto);
    
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

} // anonymous namespace
} // end of gnash namespace

