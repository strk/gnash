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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "Math.h"
#include "tu_random.h"

namespace gnash {

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

void math_init(as_object* s_global)
{
    // Create built-in math object.
    as_object*	math_obj = new math_as_object;

    s_global->set_member("math", math_obj);
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

MATH_WRAP_FUNC1(fabs);
MATH_WRAP_FUNC1(acos);
MATH_WRAP_FUNC1(asin);
MATH_WRAP_FUNC1(atan);
MATH_WRAP_FUNC1(ceil);
MATH_WRAP_FUNC1(cos);
MATH_WRAP_FUNC1(exp);
MATH_WRAP_FUNC1(floor);
MATH_WRAP_FUNC1(log);
MATH_WRAP_FUNC1(sin);
MATH_WRAP_FUNC1(sqrt);
MATH_WRAP_FUNC1(tan);

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
MATH_WRAP_FUNC2_EXP(atan2, (atan2(arg0, arg1)));
MATH_WRAP_FUNC2_EXP(max, (arg0 > arg1 ? arg0 : arg1));
MATH_WRAP_FUNC2_EXP(min, (arg0 < arg1 ? arg0 : arg1));
MATH_WRAP_FUNC2_EXP(pow, (pow(arg0, arg1)));

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

	// constant
	set_member("e", 2.7182818284590452354);
	set_member("ln2", 0.69314718055994530942);
	set_member("log2e", 1.4426950408889634074);
	set_member("ln10", 2.30258509299404568402);
	set_member("log10e", 0.43429448190325182765);
	set_member("pi", 3.14159265358979323846);
	set_member("sqrt1_2", 0.7071067811865475244);
	set_member("sqrt2", 1.4142135623730950488);

	// math methods
	set_member("abs", &math_abs);
	set_member("acos", &math_acos);
	set_member("asin", &math_asin);
	set_member("atan", &math_atan);
	set_member("ceil", &math_ceil);
	set_member("cos", &math_cos);
	set_member("exp", &math_exp);
	set_member("floor", &math_floor);
	set_member("log", &math_log);
	set_member("random", &math_random);
	set_member("round", &math_round);
	set_member("sin", &math_sin);
	set_member("sqrt", &math_sqrt);
	set_member("tan", &math_tan);

	set_member("atan2", &math_atan2);
	set_member("max", &math_max);
	set_member("min", &math_min);
	set_member("pow", &math_pow);
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

