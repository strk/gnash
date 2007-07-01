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

#ifndef _GNASH_AS_FUNCTION_H_
#define _GNASH_AS_FUNCTION_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_object.h" // for inheritance

// Forward declarations
namespace gnash {
	class fn_call;
	class builtin_function;
}

namespace gnash {

/// ActionScript Function, either builtin or SWF-defined
//
/// In ActionScript, every Function is also a class.
/// The *exported interface* of the class is defined
/// as an 'prototype' member of the function object.
///
/// Any instance of the class defined by this function will
/// inherit any member of the class 'prototype'.
/// To have an object inherit from a class you can set it's
/// __proto__ member so to point to the class prototype, ie:
///
///   function MyClass() {}
///   MyClass.prototype.doit = function() { trace("doing it"; }
///
///   var myobj = new Object;
///   myobj.__proto__ = MyClass.prototype;
///
/// The 'prototype' of a class must provide a 'constructor'
/// member, which would point back to the Function object
/// itself, which is used as the constructor, so given the
/// code above you can assert that:
///
///   myobj.__proto__.constructor == MyClass
///
/// This class will automatically setup the 'prototype' member
/// if not explicitly provided (ie: will set 'constructor' so
/// that it points to the instance).
/// 
///
///
class as_function : public as_object
{
public:

	/// Decrement refcount on the exported interface.
	virtual ~as_function() {}

	// Avoid RTTI
	as_function* to_function() { return this; }

	/// Dispatch.
	virtual as_value operator()(const fn_call& fn)=0;

	/// Alias for operator()
	as_value call(const fn_call& fn) { return operator()(fn); }

	/// Construct an instance of this class
	// 
	///  Post-conditions:
	///      - The returned object is an instance of this class.
	///        See as_object::instanceOf
	///  
	/// @param env
	///	The environment to use for stack, local variables,
	///	registers and scope chain.
	/// 
	/// @param nargs
	///	Number of arguments passed to this constructor
	///
	/// @param first_arg_index
	///	Index of the as_environment stack element to use
	///     as first argument.
	/// 
	boost::intrusive_ptr<as_object> constructInstance( as_environment& env,
			unsigned nargs, unsigned first_arg_index);

	/// Get this function's "prototype" member (exported interface).
	//
	/// This is never NULL, and created on purpose if not provided
	/// at construction time. 
	///
	as_object* getPrototype();

	/// Make this function a subclass of the given as_function
	void extends(as_function& superclass);

	/// Return true if this is a built-in class.
	virtual bool isBuiltin()  { return false; }

	/// TODO: check if a user-defined 'toString'
	///       will be used when available.
	std::string get_text_value() const
	{
		return "[type Function]";
	}

	/// Return the built-in Function constructor
	static boost::intrusive_ptr<builtin_function> getFunctionConstructor();

#ifdef GNASH_USE_GC
	/// Mark reachable resources. Override from GcResource
	//
	/// Reachable resources from this object is its prototype
	/// and the default as_object reachables (properties and parent).
	///
	/// The default implementation only marks that. If you
	/// override this function from a derived class remember
	/// to call markAsFunctionReachableResources() as a final step.
	///
	virtual void markReachableResources() const
	{
		markAsFunctionReachable();
	}
#endif // GNASH_USE_GC

protected:

#ifdef GNASH_USE_GC
	/// Mark prototype (properties) as being reachable and invoke
	/// the as_object class marker.
	//
	/// Call this function from an override of markReachableResources
	/// in a derived class
	///
	void markAsFunctionReachable() const
	{
		_properties->setReachable();

		markAsObjectReachable();
	}
#endif // GNASH_USE_GC

	/// Construct a function with given interface
	//
	/// If the given interface is NULL a default one
	/// will be provided, with constructor set as 'this'.
	///
	/// @param iface
	///	The interface exported by this class (ie.
	///	it's 'prototype' member). If NULL a default
	///	prototype will be used, using 'this' as
	///	it's 'constructor' member. 
	///	Refcount on the interface will be incremented.
	///
	as_function(as_object* iface);

	/// The "prototype" member.
	//
	/// Used for class constructor and members
	/// to be inherited by instances of this
	/// "Function" (class)
	///
	boost::intrusive_ptr<as_object>	_properties;

private:

	void setPrototype(as_object* proto);
};

/// Initialize the global Function constructor
void function_class_init(as_object& global);

// To be made statics instead
as_value function_apply(const fn_call& fn);
as_value function_call(const fn_call& fn);


} // end of gnash namespace

// _GNASH_AS_FUNCTION_H_
#endif

