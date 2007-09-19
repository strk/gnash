// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// 
//
//

// Implementation and helpers for SWF actions.


#ifndef GNASH_ACTION_H
#define GNASH_ACTION_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"

#include "gnash.h"
#include "as_object.h"
#include "types.h"
#include "container.h"
#include "smart_ptr.h"

#include <cwchar>

#ifdef __sgi
	__SGI_LIBC_USING_FROM_STD(va_list) 
#endif

namespace gnash {
	class sprite_instance;
	class as_environment;
	class as_object;
	class as_value;
	class swf_function;


	extern DSOEXPORT boost::intrusive_ptr<as_object> s_global;

	class DSOLOCAL as_property_interface
	{
	public:
		virtual ~as_property_interface() {}
		virtual bool	set_property(int index, const as_value& val) = 0;
	};

	//
	// Some handy helpers
	//

	// Dispatching methods from C++.
	as_value	call_method0(const as_value& method, as_environment* env, as_object* this_ptr);
	as_value	call_method1(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0);
	as_value	call_method2(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0, const as_value& arg1);
	as_value	call_method3(
		const as_value& method, as_environment* env, as_object* this_ptr,
		const as_value& arg0, const as_value& arg1, const as_value& arg2);

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
		int nargs, int first_arg_bottom_index);

	const char*	call_method_parsed(
		as_environment* env,
		as_object* this_ptr,
		const char* method_name,
		const char* method_arg_fmt,
		va_list args);

	// tulrich: don't use this!  To register a class constructor,
	// just assign the classname to the constructor function.  E.g.:
	//
	// my_movie->set_member("MyClass", as_value(MyClassConstructorFunction));
	// 
	//void register_as_object(const char* object_name, as_c_function_ptr handler);

	// deprecated, use sprite_instance::loadMovie
	//void attach_extern_movie(const char* c_url, const sprite_instance* target, const sprite_instance* root_movie);

}	// end namespace gnash


#endif // GNASH_ACTION_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
