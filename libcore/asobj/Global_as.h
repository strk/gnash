// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// This file provides implementations of two different ActionScript global
/// objects: one for AVM1, one for AVM2.
//
/// The AVM1 global object has more (known) global functions. All AS2 classes
/// are initialized as object prototypes and functions attached to the 
/// global object. From SWF8, the 'flash' package is attached as follows:
///
///                             _global
///                                 |
///                               flash
///                                 |
///            ---------------------------------------------------
///            |         |          |          |         |       |
///         display     net     external    filters     geom    text
///
/// where each item is an object.
///
/// The AVM2 global object has functions such as trace(), escape(),
/// parseFloat(), parseInt() in common with AVM1. Some classes, such as
/// Array, Boolean, Date, String, and Object, are also directly attached.
/// Other classes, however, are different. The flash package in AVM2 is a 
/// namespace, not an object, and all members of the flash package are
/// attached with a namespace to the global object. As we do this on
/// demand, the AVM2 global object is much emptier than the AVM1 equivalent
/// to start with.
#ifndef GNASH_GLOBAL_H
#define GNASH_GLOBAL_H

#include "as_object.h" // for inheritance
#include "extension.h"
#include "ClassHierarchy.h"

// Forward declarations
namespace gnash {
	class Machine;
	class as_value;
	class VM;
	class fn_call;
	class ClassHierarchy;
}

namespace gnash {

/// The Global object ultimately contains all objects in an ActionScript run
//
/// An ActionScript run is a single version (AS1/2 or AS3) and includes all
/// resources parsed from the SWF, created dynamically, loaded, or imported
/// that are available to ActionScript code.
//
/// Each VM (VM for AS1/2, Machine for AS3) has different resources in its
/// Global object. The two objects should be entirely separate.
class Global_as : public as_object
{
public:

    typedef as_value(*ASFunction)(const fn_call& fn);

    virtual const ClassHierarchy& classHierarchy() const = 0;
    virtual ClassHierarchy& classHierarchy() = 0;

    /// Create an ActionScript function
    virtual as_object* createFunction(ASFunction function) = 0;

    /// Create an ActionScript class
    //
    /// The type of a class is different in AS2 and AS3. In AS2 it is generally
    /// a function (the constructor) with a prototype. In AS3 it is generally
    /// an object (the prototype) with a constructor.
    virtual as_object* createClass(ASFunction ctor, as_object* prototype) = 0;

    virtual Global_as& global() {
        return *this;
    }

    virtual VM& getVM() const = 0;
};

} // namespace gnash

#endif 
