// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#ifndef GNASH_GLOBALS_H
#define GNASH_GLOBALS_H

#include <string>
#include "Global_as.h" 
#include "extension.h"
#include "ClassHierarchy.h"

// Forward declarations
namespace gnash {
    namespace abc {
        class Machine;
    }
	class VM;
	class fn_call;
}

namespace gnash {

class AVM1Global : public Global_as
{
public:

	AVM1Global(VM& vm);
	~AVM1Global() {}

    void registerClasses();

    virtual as_object* createString(const std::string& s);

    virtual as_object* createNumber(double d);

    virtual as_object* createBoolean(bool b);
    
    virtual as_object* createObject();
    
    virtual as_object* createArray();

    virtual const ClassHierarchy& classHierarchy() const {
        return _classes;
    }
    
    virtual ClassHierarchy& classHierarchy() {
        return _classes;
    }

    virtual VM& getVM() const {
        return vm();
    }
    
    /// Create an ActionScript function
    virtual builtin_function* createFunction(Global_as::ASFunction function);

    /// Create an ActionScript class
    //
    /// An AS2 class is generally a function (the constructor) with a
    /// prototype.
    virtual as_object* createClass(Global_as::ASFunction ctor,
            as_object* prototype);

protected:
    
    virtual void markReachableResources() const;

private:

    void loadExtensions();
	Extension _et;

    ClassHierarchy _classes;
    
    as_object* _objectProto;

};

} // namespace gnash

#endif 
