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
#include "Object.h"

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

    /// Create a String object
    //
    /// This calls the String constructor. If that has been changed, this
    /// function may not produce a String object. This is generally
    /// expected behaviour.
    virtual as_object* createString(const std::string& s) = 0;

    /// Create a Number object
    //
    /// This calls the Number constructor. If that has been changed, this
    /// function may not produce a Number object. This is generally
    /// expected behaviour.
    virtual as_object* createNumber(double d) = 0;

    /// Create a Boolean object
    //
    /// This calls the Boolean constructor. If that has been changed, this
    /// function may not produce a Boolean object. This is generally
    /// expected behaviour.
    virtual as_object* createBoolean(bool b) = 0;

    /// Create an Object
    //
    /// This function returns an Object with Object.prototype as its
    /// __proto__ member. It should probably call the Object constructor,
    /// but Gnash creates some of its classes on demand. If the Object class
    /// has changed before this happens, Gnash's behaviour would differ from
    /// the reference player's.
    //
    /// TODO: think whether it's better to return the original Object class,
    /// a possibly altered one, or allow both.
    virtual as_object* createObject() = 0;

    virtual Global_as& global() {
        return *this;
    }

    virtual VM& getVM() const = 0;
};

typedef void(*Properties)(as_object&);

/// Register a built-in object
//
/// This is used for simple objects that are part of the player API.
//
/// In the reference player these objects are always constructed in
/// ActionScript, though their functions may be native.
//
/// They include (AS2) Mouse, Selection and Stage, and (AS3) all constant
/// enumeration objects.
//
/// @param p        a pointer to a function that will attach properties to the
///                 object
/// @param where    the object to which the created object will be attached
/// @param uri      an ObjectURI describing the name and namespace of the
///                 created object.
/// @return         the built-in object with properties attached.
inline as_object*
registerBuiltinObject(as_object& where, Properties p, const ObjectURI& uri)
{

    // This is going to be the global Mouse "class"/"function"
    Global_as* gl = getGlobal(where);
    as_object* obj = gl->createObject();
    if (p) p(*obj);
    
    where.init_member(getName(uri), obj, as_object::DefaultFlags,
            getNamespace(uri));

    return obj;
}

/// Register a built-in class
//
/// This is used for classes that are part of the player API.
//
/// In the reference player these classes are always constructed in
/// ActionScript, though their functions may be native, and the constructor
/// may also call native functions.
//
/// @param c        a pointer to a function that will attach properties to the
///                 class itself. These are known as static properties.
/// @param p        a pointer to a function that will attach properties to the
///                 class prototype. These are instance properties.
/// @param ctor     the constructor function for the new class.
/// @param where    the object to which the created object will be attached
/// @param uri      an ObjectURI describing the name and namespace of the
///                 created object.
/// @return         the built-in class with prototype and properties attached.
inline as_object*
registerBuiltinClass(as_object& where, Global_as::ASFunction ctor,
        Properties p, Properties c, const ObjectURI& uri)
{
    Global_as* gl = getGlobal(where);
    as_object* proto = gl->createObject();
    as_object* cl = gl->createClass(ctor, proto);
 
    // Attach class properties to class
    if (c) c(*cl);

    // Attach prototype properties to prototype
    if (p) p(*proto);

    // Register class with specified object.
    where.init_member(getName(uri), cl, as_object::DefaultFlags,
            getNamespace(uri));
    return cl;
}

} // namespace gnash

#endif 
