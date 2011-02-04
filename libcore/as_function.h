// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include <string>

#include "as_object.h"

// Forward declarations
namespace gnash {
    class NativeFunction;
    class Global_as;
    template <typename T> class FunctionArgs;
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

    /// Destructor
	virtual ~as_function() {}

    /// Return this as_object as an as_function.
	virtual as_function* to_function() { return this; }

	/// Function dispatch.
    //
    /// Override from as_object, although as_objects cannot generally 
    /// be called.
	virtual as_value call(const fn_call& fn) = 0;

    /// Return the string value of this as_object subclass
    //
    /// It's "function".
    virtual std::string stringValue() const;

	/// Run this function as a constructor on an object
    //
    /// This function assigns various constructor properties and runs the
    /// constructor.
    //
    /// NB: This function does not make the object an 'instance of' the
    /// constructor, i.e. it does not assign a __proto__ property. For
    /// ActionScript compatibility, callers should ensure this is already
    /// done. 
    //
    /// @param newobj   The object to construct. This will be used as the
    ///                 'this' object in the constructor.
	/// @param env      The environment to use for stack, local variables,
	///	                registers and scope chain.
	/// @param args     Arguments for the constructor invocation
    /// @return         The constructed object. TODO: return void; currently
    ///                 there is a hack to cope with some remaining bogus
    ///                 constructors, which
    ///                 necessitates returning a different object from the
    ///                 passed 'this' pointer.
    as_object* construct(as_object& newobj, const as_environment& env,
            FunctionArgs<as_value>& args);

	/// Return true if this is a built-in class.
	virtual bool isBuiltin() { return false; }

protected:
	
    /// Construct a function.
	as_function(Global_as& gl);

};


/// Construct a new object from the given constructor
//
/// This function takes care of creating the new object and assigning the
/// __proto__ property. The construct() function is then called with the
/// new object as its 'this' object.
//
/// @param ctor     The constructor to run.
/// @param env      The environment to use for the function call.
/// @param arg      The arguments to pass to the constructor function.
/// @return         A newly-created object constructed by the specified
///                 function.
as_object* constructInstance(as_function& ctor, const as_environment& env,
        FunctionArgs<as_value>& args);

} // gnash namespace

#endif

