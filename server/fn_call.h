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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifndef _GNASH_FN_CALL_H_
#define _GNASH_FN_CALL_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_environment.h" // for inlines (arg)

#include <cassert> // for inlines (arg)

namespace gnash {

// Forward declarations
struct as_environment;
class as_object;
struct as_value;

/// \brief
/// Parameters/environment for builtin or user-defined functions
/// callable from ActionScript.
struct fn_call
{
	as_value* result;
	as_object* this_ptr;
	as_environment* env;
	int nargs;
	int first_arg_bottom_index;

	fn_call(as_value* res_in, as_object* this_in,
			as_environment* env_in,
			int nargs_in, int first_in)
		:
		result(res_in),
		this_ptr(this_in),
		env(env_in),
		nargs(nargs_in),
		first_arg_bottom_index(first_in)
	{
	}

	/// Access a particular argument.
	as_value& arg(int n) const
	{
		assert(n < nargs);
		return env->bottom(first_arg_bottom_index - n);
	}

};

/// Signature of a builtin function callable from ActionScript
typedef void (*as_c_function_ptr)(const fn_call& fn);


} // namespace gnash


#endif // _GNASH_FN_CALL_H_


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
