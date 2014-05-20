// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include <string>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <memory>

#include "as_object.h" 
#include "fn_call.h"
#include "log.h"
#include "ClassHierarchy.h"
#include "dsodefs.h" // for DSOTEXPORT

// Forward declarations
namespace gnash {
    class as_value;
    class VM;
    class Extension;
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
    typedef void(*Properties)(as_object&);

	explicit Global_as(VM& vm);
	virtual ~Global_as();
    
    void registerClasses();

    as_object* createArray();

    VM& getVM() const {
        return vm();
    }
    
    /// Create an ActionScript function
    as_function* createFunction(Global_as::ASFunction function);

    /// Create an ActionScript class
    //
    /// An AS2 class is generally a function (the constructor) with a
    /// prototype.
    as_object* createClass(Global_as::ASFunction ctor,
            as_object* prototype);

    void makeObject(as_object& o) const;

protected:
    
    virtual void markReachableResources() const;

private:

    void loadExtensions();
    std::unique_ptr<Extension> _et;

    ClassHierarchy _classes;
    
    as_object* _objectProto;

};

DSOTEXPORT as_object* createObject(const Global_as& gl);
    

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
registerBuiltinObject(as_object& where, Global_as::Properties p,
        const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* obj = createObject(gl);
    if (p) p(*obj);
    
    where.init_member(uri, obj, as_object::DefaultFlags);

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
        Global_as::Properties p, Global_as::Properties c, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(ctor, proto);
 
    // Attach class properties to class
    if (c) c(*cl);

    // Attach prototype properties to prototype
    if (p) p(*proto);

    // Register class with specified object.
    where.init_member(uri, cl, as_object::DefaultFlags);
    return cl;
}

/// Call an as_value on an as_object.
//
/// The call will fail harmlessly if the as_value is not callable.
inline DSOEXPORT as_value
invoke(const as_value& method, const as_environment& env, as_object* this_ptr,
        fn_call::Args& args, as_object* super = 0,
        const movie_definition* callerDef = 0)
{

	as_value val;
	fn_call call(this_ptr, env, args);
	call.super = super;
    call.callerDef = callerDef;

	try {
		if (as_object* func = toObject(method, getVM(env))) {
            // Call function.
		    val = func->call(call);
		}
		else {
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror("Attempt to call a value which is not "
                    "a function (%s)", method);
            );
            return val;
		}
	}
	catch (ActionTypeError& e) {
		assert(val.is_undefined());
		IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("%s", e.what());
		);
	}
	return val;
}

/// Helper macros for callMethod arguments.
#define FUNC_PARAM(z, n, t) BOOST_PP_COMMA_IF(n) t arg##n
#define VALUE_ARG(z, n, t) BOOST_PP_COMMA_IF(n) arg##n

/// Call a member function of this object in an AS-compatible way
//
/// This is a macro to cope with a varying number of arguments. The function
/// signature is as follows:
//
/// as_value callMethod(as_object* obj, const ObjectURI& uri,
///     const as_value& arg1, ..., const as_value& argN);
//
/// If the member function exists and is a function, invoke() is called on
/// the member with the object as the this pointer.
//
/// @param obj          The object to call the method on. This may be null, in
///                     which case the call is a no-op. This is because calling
///                     methods on null or non-objects in AS is harmless.
/// @param name         The name of the method. 
///
/// @param arg0..argN   The arguments to pass
///
/// @return             The return value of the call (possibly undefined).
#define CALL_METHOD(x, n, t) \
inline as_value \
callMethod(as_object* obj, const ObjectURI& uri BOOST_PP_COMMA_IF(n)\
        BOOST_PP_REPEAT(n, FUNC_PARAM, const as_value&)) {\
    if (!obj) return as_value();\
    as_value func;\
    if (!obj->get_member(uri, &func)) return as_value();\
    fn_call::Args args;\
    BOOST_PP_EXPR_IF(n, (args += BOOST_PP_REPEAT(n, VALUE_ARG, BOOST_PP_EMPTY));)\
    return invoke(func, as_environment(getVM(*obj)), obj, args);\
}

/// The maximum number of as_value arguments allowed in callMethod functions.
#define MAX_ARGS 4
BOOST_PP_REPEAT(BOOST_PP_INC(MAX_ARGS), CALL_METHOD, BOOST_PP_EMPTY)

#undef VALUE_ARG
#undef FUNC_PARAM
#undef MAX_ARGS
#undef CALL_METHOD

/// Convenience function for finding a class constructor.
//
/// Only currently useful in AS2.
inline as_function*
getClassConstructor(const fn_call& fn, const std::string& s)
{
    const as_value ctor(findObject(fn.env(), s));
    return ctor.to_function();
}

inline as_value
emptyFunction(const fn_call&)
{
    return as_value();
}

} // namespace gnash

#endif 
