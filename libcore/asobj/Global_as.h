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

#ifndef GNASH_GLOBAL_H
#define GNASH_GLOBAL_H

#include "as_object.h" // for inheritance

// Forward declarations
namespace gnash {
	class builtin_function;
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
    virtual builtin_function* createFunction(ASFunction function) = 0;

    /// Create an ActionScript class
    //
    /// The type of a class is different in AS2 and AS3. In AS2 it is generally
    /// a function (the constructor) with a prototype. In AS3 it is generally
    /// an object (the prototype) with a constructor.
    virtual as_object* createClass(ASFunction ctor, as_object* prototype) = 0;

    virtual as_object* createString(const std::string& s) = 0;

    virtual as_object* createNumber(double d) = 0;

    virtual as_object* createBoolean(bool b) = 0;

    virtual as_object* createObject() = 0;
    
    virtual as_object* createObject(as_object* prototype) = 0;

    virtual Global_as& global() {
        return *this;
    }

    virtual VM& getVM() const = 0;
};

} // namespace gnash

#endif 
