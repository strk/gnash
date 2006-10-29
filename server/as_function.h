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

// 
//
//

#ifndef _GNASH_AS_FUNCTION_H_
#define _GNASH_AS_FUNCTION_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_object.h" // for inheritance

// Forward declarations
namespace gnash {
	class fn_call;
}

namespace gnash {

/// ActionScript Function, either builtin or SWF-defined
class as_function : public as_object
{
public:

	virtual ~as_function() {}

	/// Dispatch.
	virtual void operator()(const fn_call& fn)=0;

	/// Get this function's "prototype" member (exported interface).
	//
	/// This is never NULL, and created on purpose if not provided
	/// at construction time. 
	as_object* getPrototype();


	/// Return true if this is a built-in class.
	virtual bool isBuiltin()  { return false; }

protected:

	/// Construct a function with given interface
	//
	/// If the given interface is NULL a default one
	/// will be provided, with constructor set as 'this'.
	as_function(as_object* iface);

	/// The "prototype" member.
	//
	/// Used for class constructor and members
	/// to be inherited by instances of this
	/// "Function" (class)
	///
	as_object*	_properties;
};

/// Initialize the global Function constructor
void function_init(as_object* global);

// To be made statics instead
void function_apply(const fn_call& fn);
void function_call(const fn_call& fn);


} // end of gnash namespace

// _GNASH_AS_FUNCTION_H_
#endif

