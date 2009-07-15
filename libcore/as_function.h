// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef GNASH_AS_FUNCTION_H
#define GNASH_AS_FUNCTION_H

#include "smart_ptr.h" // GNASH_USE_GC
#include "as_object.h" // for inheritance

// Forward declarations
namespace gnash {
	class fn_call;
	class builtin_function;
	class Global_as;
}

namespace gnash {

/// ActionScript Function, either builtin or SWF-defined
//
/// In ActionScript, every Function is also a class.
/// The *exported interface* of the class is defined
/// as a 'prototype' member of the function object.
///
/// Any instance of the class defined by this function will
/// inherit any member of the class 'prototype'.
/// To have an object inherit from a class you can set its
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
class as_function : public as_object
{
public:

	/// Decrement refcount on the exported interface.
	virtual ~as_function() {}

	// Avoid RTTI
	as_function* to_function() { return this; }

	/// Dispatch.
	virtual as_value operator()(const fn_call& fn) = 0;

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
	/// @param args
	///	Arguments for the constructor invocation
	///
	boost::intrusive_ptr<as_object> constructInstance(const as_environment& env,
			std::auto_ptr<std::vector<as_value> > args);

	/// Get this function's "prototype" member (exported interface).
	///
	boost::intrusive_ptr<as_object> getPrototype();

	/// Make this function a subclass of the given as_function
	void extends(as_function& superclass);

	/// Return true if this is a built-in class.
	virtual bool isBuiltin() { return false; }

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
	
    /// Construct a function with no interface
	//
	/// For SWF>5 the function object will have derive from Function.
	///
	as_function(Global_as& gl);

	/// Construct a function with given interface (possibly none)
	//
	/// @param iface
	///	The interface exported by this class (its 'prototype' member).
	///	If NULL, no prototype will be set (this is used for some
	///	corner cases like TextField in SWF5 or below).
	///	If not NULL, a 'constructor' member will be added to the
	///	prototype, pointing to 'this'.
	///
	as_function(Global_as& gl, as_object* iface);

#ifdef GNASH_USE_GC
	/// Mark prototype (properties) as being reachable and invoke
	/// the as_object class marker.
	//
	/// Call this function from an override of markReachableResources
	/// in a derived class
	///
	void markAsFunctionReachable() const
	{
		//_properties->setReachable();

		markAsObjectReachable();
	}
#endif // GNASH_USE_GC


private:

	void setPrototype(as_object* proto);
};

/// Initialize the global Function constructor
void function_class_init(as_object& global);

} // gnash namespace

#endif

