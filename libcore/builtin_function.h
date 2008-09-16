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

#ifndef __GNASH_BUILTIN_FUNCTION_H__
#define __GNASH_BUILTIN_FUNCTION_H__

#include "as_function.h" // for inheritance
#include "fn_call.h" // for call operator
// #include "as_environment.h" // for FrameGuard
#include "namedStrings.h"

#include <cassert>

namespace gnash {

typedef as_value (*as_c_function_ptr)(const fn_call& fn);


/// Any built-in function/class should be of this type
class builtin_function : public as_function
{

public:

	/// Construct a builtin function/class with a default interface
	//
	/// The default interface will have a constructor member set as 'this'
	///
	/// @param func
	///	The C function to call when this as_function is invoked.
	/// 	For classes, the function pointer is the constructor.
	///
	builtin_function(as_c_function_ptr func)
		:
		as_function(),
		_func(func)
	{
		init_member(NSV::PROP_CONSTRUCTOR, as_function::getFunctionConstructor().get());
	}

	/// Construct a builtin function/class with the given interface (possibly none)
	//
	/// @param func
	///	The C function to call when this as_function is invoked.
	/// 	For classes, the function pointer is the constructor.
	///
	/// @param iface
	///	The interface of this class (will be inherited by
	///	instances of this class)
	/// 	If the given interface is NULL no interface will be
	/// 	provided. Use the constructor taking a single argument
	///	to get a default interface instead.
	///
	builtin_function(as_c_function_ptr func, as_object* iface, bool useThisAsCtor=false)
		:
		as_function(iface),
		_func(func)
	{
		if ( useThisAsCtor )
		{
			init_member(NSV::PROP_CONSTRUCTOR, this);
		}
		else
		{
			init_member(NSV::PROP_CONSTRUCTOR, as_function::getFunctionConstructor().get());
		}
	}

	/// Invoke this function or this Class constructor
	virtual as_value operator()(const fn_call& fn)
	{
		// Real native functions don't put self on the CallStack
		// (they never end up in an arguments.caller).
		// This is, for example, the case for Array.sort when
		// calling a custom comparator.
		// Unfortunately gnash implements all builtin as natives
		// while the proprietary player doesn't (for example
		// MovieClip.meth does end up in arguments.caller).
		//
		// A possible short-term solution to this could be
		// specifying for a builtin_function whether or not
		// it should be considered 'native'.
		// If not 'native', we'd push a CallFrame on stack...
		//
		//as_environment::FrameGuard guard(fn.env(), this);

		assert(_func);
		return _func(fn);
	}

	bool isBuiltin()  { return true; }

private:

	as_c_function_ptr _func;
};

} // end of gnash namespace

// __GNASH_BUILTIN_FUNCTION_H__
#endif

