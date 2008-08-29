// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

// Implementation and helpers for SWF actions.


#ifndef GNASH_ACTION_H
#define GNASH_ACTION_H

#include "dsodefs.h" // for DSOEXPORT

#include "as_object.h"
#include "types.h"
#include "smart_ptr.h"

namespace gnash {
	class sprite_instance;
	class as_environment;
	class as_object;
	class as_value;
	class swf_function;


	class DSOLOCAL as_property_interface
	{
	public:
		virtual ~as_property_interface() {}
		virtual bool set_property(int index, const as_value& val) = 0;
	};

	//
	// Some handy helpers
	//

	// Dispatching methods from C++.
	as_value	call_method0(const as_value& method, as_environment* env, as_object* this_ptr);

	/// Call a method, be it an as_function or a c_function. 
	//
	/// This is a thin wrapper around operator() and fn_call,
	/// probably worth dropping.
	///
	/// first_arg_bottom_index is the stack index, from the bottom,
	/// of the first argument.  Subsequent arguments are at *lower*
	/// indices.  E.g. if first_arg_bottom_index = 7, then arg1 is
	/// at env->bottom(7), arg2 is at env->bottom(6), etc.
	///
	DSOEXPORT as_value call_method(const as_value& method, as_environment* env,
		as_object* this_ptr, // this is ourself
		int nargs, int first_arg_bottom_index, as_object* super=NULL);

	/// Call a method, be it an as_function or a c_function. 
	//
	/// This is a thin wrapper around operator() and fn_call,
	/// probably worth dropping.
	///
	DSOEXPORT as_value call_method(const as_value& method, as_environment* env,
		as_object* this_ptr, // this is ourself
		std::auto_ptr<std::vector<as_value> > args, as_object* super=NULL);


}	// end namespace gnash


#endif // GNASH_ACTION_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
