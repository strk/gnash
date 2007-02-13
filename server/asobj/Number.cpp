// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

/* $Id: Number.cpp,v 1.21 2007/02/13 19:36:34 rsavoye Exp $ */

// Implementation of ActionScript Number class.

#include "log.h"
#include "tu_config.h"
#include "Number.h"
#include "smart_ptr.h"
#include "fn_call.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function

#include <sstream>
#include <cmath>

// Why ?
//#undef fpclassify
//#define fpclassify(x) _fpclassify(x)

/* C99: 7.12 6 defines for floating point classification */

#undef FP_ZERO
#define FP_ZERO          1
#undef FP_SUBNORMAL
#define FP_SUBNORMAL     2
#undef FP_NORMAL
#define FP_NORMAL        4
#undef FP_INFINITE
#define FP_INFINITE      8
#undef FP_NAN
#define FP_NAN           16 

using namespace std;

namespace gnash {

// Forward declarations
static void number_val_to_str(double val, char *str);
//static void number_to_string(const fn_call& fn);

static void
attachNumberInterface(as_object& o)
{
	// FIXME: add Number interface here:
	// Number.MAX_VALUE
	// Number.MIN_VALUE
	// Number.NaN
	// Number.NEGATIVE_INFINITY
	// Number.POSITIVE_INFINITY

	// Number.toString()
	o.init_member("toString", &as_object::tostring_method);

	// Number.valueOf()
	o.init_member("valueOf", &as_object::valueof_method);
}

static as_object*
getNumberInterface()
{
	static boost::intrusive_ptr<as_object> o=NULL;
	if ( o == NULL )
	{
		o = new as_object();
		attachNumberInterface(*o);
	}
	return o.get();
}

// FIXME: add some useful methods :)
class number_as_object : public as_object
{
	// the number value
	double _val;

	// string representation of the above number
	mutable char _str[256];

public:

	number_as_object(double val=0.0)
		:
		as_object(getNumberInterface()),
		_val(val)
	{
	}

	// override from as_object
	const char* get_text_value() const
	{
		number_val_to_str(_val, _str);
		return _str;
	}

	// override from as_object
	double get_numeric_value() const
	{
		return _val;
	}

	as_value get_primitive_value() const 
	{
		return _val;
	}

};

// Convert numeric value to string value
static void
number_val_to_str(double _val, char *_str)
{
	// Printing formats:
	//
	// If _val > 1, Print up to 15 significant digits, then switch
	// to scientific notation, rounding at the last place and
	// omitting trailing zeroes.
	// e.g. for 9*.1234567890123456789
	// ...
	// 9999.12345678901
	// 99999.123456789
	// 999999.123456789
	// 9999999.12345679
	// 99999999.1234568
	// 999999999.123457
	// 9999999999.12346
	// 99999999999.1235
	// 999999999999.123
	// 9999999999999.12
	// 99999999999999.1
	// 999999999999999
	// 1e+16
	// 1e+17
	// ...
	// e.g. for 1*.111111111111111111111111111111111111
	// ...
	// 1111111111111.11
	// 11111111111111.1
	// 111111111111111
	// 1.11111111111111e+15
	// 1.11111111111111e+16
	// ...
	// For values < 1, print up to 4 leading zeroes after the
	// deciman point, then switch to scientific notation with up
	// to 15 significant digits, rounding with no trailing zeroes
	// e.g. for 1.234567890123456789 * 10^-i:
	// 1.23456789012346
	// 0.123456789012346
	// 0.0123456789012346
	// 0.00123456789012346
	// 0.000123456789012346
	// 0.0000123456789012346
	// 0.00000123456789012346
	// 1.23456789012346e-6
	// 1.23456789012346e-7
	// ...
	//
	// If the value is negative, just add a '-' to the start; this
	// does not affect the precision of the printed value.
	//
	// This almost corresponds to printf("%.15g") format, except
	// that %.15g switches to scientific notation at e-05 not e-06,
	// and %g always prints at least two digits for the exponent.

	// The following code gives the same results as Adobe player
	// except for
	// 9.99999999999999[39-61] e{-2,-3}. Adobe prints these as
	// 0.0999999999999999 and 0.00999999999999 while we print them
	// as 0.1 and 0.01
	// These values are at the limit of a double's precision,
	// for example, in C,
	// .99999999999999938 printfs as
	// .99999999999999933387 and
	// .99999999999999939 printfs as
	// .99999999999999944489
	// so this behaviour is probably too compiler-dependent to
	// reproduce exactly.
	//
	// There may be some milage in comparing against
	// 0.00009999999999999995 and
	// 0.000009999999999999995 instead.

	// Handle non-numeric values.
	// "printf" gives "nan", "inf", "-inf", so we check explicitly
	if(isnan(_val))
		strcpy(_str, "NaN");
	else if(isinf(_val))
		strcpy(_str, _val < 0 ? "-Infinity" : "Infinity");
	else{	// FP_ZERO, FP_NORMAL and FP_SUBNORMAL
		if (fabs(_val) < 0.0001 &&
		    fabs(_val) >= 0.00001) {
			// This is the range for which %.15g gives scientific
			// notation but for which we must give decimal.
			// We can't easily use %f bcos it prints a fixed number
			// of digits after the point, not the maximum number of
			// significant digits with trailing zeroes removed that
			// we require. So we just get %g to do its non-e stuff
			// by multiplying the value by ten and then stuffing
			// an extra zero into the result after the decimal
			// point. Yuk!
			char *cp;
			
			sprintf(_str, "%.15g", _val * 10.0);
			if ((cp = strchr(_str, '.')) == NULL || cp[1] != '0') {
				log_error("Internal error: Cannot find \".0\" in \%s for %.15g\n", _str, _val);
				// Just give it to them raw instead
				sprintf(_str, "%.15g", _val);
			} else {
#if HAVE_MEMMOVE
				// Shunt the digits right one place after the
				// decimal point.
				memmove(cp+2, cp+1, strlen(cp+1)+1);
#else
				// We can't use strcpy() cos the args overlap.

				char c;	// character being moved forward
				
				// At this point, cp points at the '.'
				//
				// In the loop body it points at where we pick
				// up the next char to move forward and where
				// we drop the one we picked up on its left.
				// We stop when we have just picked up the \0.
				for (c = '0', cp++; c != '\0'; cp++) {
					char tmp = *cp; *cp = c; c = tmp;
				}
				// Store the '\0' we just picked up
				*cp = c;
#endif
			}
		} else {
			// Regular case
			char *cp;

			sprintf(_str, "%.15g", _val);
			// Remove a leading zero from 2-digit exponent if any
			if ((cp = strchr(_str, 'e')) != NULL &&
			    cp[2] == '0') {
				// We can't use strcpy() cos its src&dest can't
				// overlap. However, this can only be "...e+0n"
				// or ...e-0n;  3+digit exponents never have
				// leading 0s.
				cp[2] = cp[3]; cp[3] = '\0';
			}
		}
	}
}

static void
number_ctor(const fn_call& fn)
{
	double val = 0;
	if (fn.nargs > 0)
	{
		val = fn.arg(0).to_number();
	}

	number_as_object* obj = new number_as_object(val);
	
	fn.result->set_as_object(obj); // will keep alive
}

// extern (used by Global.cpp)
void number_class_init(as_object& global)
{
	// This is going to be the global Number "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl=NULL;

	if ( cl == NULL )
	{
		cl=new builtin_function(&number_ctor, getNumberInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachNumberInterface(*cl); 
	}

	// Register _global.Number
	global.init_member("Number", cl.get());

}

auto_ptr<as_object>
init_number_instance(double val)
{
	return auto_ptr<as_object>(new number_as_object(val));
}
  
} // namespace gnash
