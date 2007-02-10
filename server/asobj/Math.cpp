// 
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

/* $Id: Math.cpp,v 1.16 2007/02/10 17:05:35 nihilus Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cmath>
#include <string>
#include "tu_random.h"
#include "fn_call.h"
#include "GMath.h"
#include "log.h"

using namespace std;

namespace gnash {

void math_abs(const fn_call& fn);
void math_acos(const fn_call& fn);
void math_asin(const fn_call& fn);
void math_atan(const fn_call& fn);
void math_atan2(const fn_call& fn);
void math_ceil(const fn_call& fn);
void math_cos(const fn_call& fn);
void math_exp(const fn_call& fn);
void math_floor(const fn_call& fn);
void math_log(const fn_call& fn);
void math_max(const fn_call& fn);
void math_min(const fn_call& fn);
void math_pow(const fn_call& fn);
void math_random(const fn_call& fn);
void math_round(const fn_call& fn);
void math_sin(const fn_call& fn);
void math_sqrt(const fn_call& fn);
void math_tan(const fn_call& fn);

Math::Math() {
}

Math::~Math() {
}


void
Math::abs()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::acos()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::asin()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::atan()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::atan2()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::ceil()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::cos()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::exp()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::floor()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::log()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::max()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::min()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::pow()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::random()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::round()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::sin()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::sqrt()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
Math::tan()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

void
math_class_init(as_object& global)
{
    // Create built-in math object.
    as_object*	math_obj = new math_as_object;

    global.init_member("Math", math_obj);
}

//
// math object
//


#if 0
// One-argument simple functions.
#define MATH_WRAP_FUNC1(funcname)							\
	void	math_##funcname(as_value* result, as_object* this_ptr,		\
				as_environment* env, int nargs, int first_arg_bottom_index)	\
	{											\
		double	arg = env->bottom(first_arg_bottom_index).to_number();			\
		result->set_double(funcname(arg));						\
	}
#else
// One-argument simple functions.
#define MATH_WRAP_FUNC1(funcname)							\
	void	math_##funcname(const fn_call& fn)						\
	{											\
		double	arg = fn.arg(0).to_number();						\
		fn.result->set_double(funcname(arg));						\
	}
#endif

#ifndef __GNUC__  //Some hacks are ugly and dirty, we call them 'fulhack'.
	#undef TU_MATH_H
	#include "tu_math.h"
#endif

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

#if 0
// Two-argument functions.
#define MATH_WRAP_FUNC2_EXP(funcname, expr)										\
	void	math_##funcname(as_value* result, as_object* this_ptr, as_environment* env, int nargs, int first_arg_bottom_index)	\
	{															\
		double	arg0 = env->bottom(first_arg_bottom_index).to_number();							\
		double	arg1 = env->bottom(first_arg_bottom_index - 1).to_number();						\
		result->set_double(expr);											\
	}
#else
// Two-argument functions.
#define MATH_WRAP_FUNC2_EXP(funcname, expr)										\
	void	math_##funcname(const fn_call& fn)										\
	{															\
		double	arg0 = fn.arg(0).to_number();										\
		double	arg1 = fn.arg(1).to_number();										\
		fn.result->set_double(expr);											\
	}
#endif
MATH_WRAP_FUNC2_EXP(atan2, (atan2(arg0, arg1)))
MATH_WRAP_FUNC2_EXP(max, (arg0 > arg1 ? arg0 : arg1))
MATH_WRAP_FUNC2_EXP(min, (arg0 < arg1 ? arg0 : arg1))
MATH_WRAP_FUNC2_EXP(pow, (pow(arg0, arg1)))

// A couple of oddballs.
void	math_random(const fn_call& fn)
{
    // Random number between 0 and 1.
    fn.result->set_double(tu_random::next_random() / double(uint32_t(0x0FFFFFFFF)));
}
void	math_round(const fn_call& fn)
{
    // round argument to nearest int.
    double	arg0 = fn.arg(0).to_number();
    fn.result->set_double(floor(arg0 + 0.5));
}
	


math_as_object::math_as_object()
	:
	as_object()
{

	// TODO: rely on ineritance, use init_property ?

	// constant
	init_member("E", 2.7182818284590452354);
	init_member("LN2", 0.69314718055994530942);
	init_member("LOG2E", 1.4426950408889634074);
	init_member("LN10", 2.30258509299404568402);
	init_member("LOG10E", 0.43429448190325182765);
	init_member("PI", 3.14159265358979323846);
	init_member("SQRT1_2", 0.7071067811865475244);
	init_member("SQRT2", 1.4142135623730950488);

	// math methods
	init_member("abs", &math_abs);
	init_member("acos", &math_acos);
	init_member("asin", &math_asin);
	init_member("atan", &math_atan);
	init_member("ceil", &math_ceil);
	init_member("cos", &math_cos);
	init_member("exp", &math_exp);
	init_member("floor", &math_floor);
	init_member("log", &math_log);
	init_member("random", &math_random);
	init_member("round", &math_round);
	init_member("sin", &math_sin);
	init_member("sqrt", &math_sqrt);
	init_member("tan", &math_tan);

	init_member("atan2", &math_atan2);
	init_member("max", &math_max);
	init_member("min", &math_min);
	init_member("pow", &math_pow);
}

void
math_new(const fn_call& fn)
{
    math_as_object *math_obj = new math_as_object;
    fn.result->set_as_object(math_obj);
}

void math_abs(const fn_call& fn) {
	return math_fabs(fn);
    //log_msg("%s:unimplemented \n", __FUNCTION__);
}


} // end of gnash namespace

