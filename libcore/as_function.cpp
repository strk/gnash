// as_function.cpp:  ActionScript Functions, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "smart_ptr.h" // GNASH_USE_GC
#include "log.h"
#include "as_function.h"
#include "as_value.h"
#include "Array_as.h"
#include "Global_as.h"
#include "fn_call.h"
#include "GnashException.h"
#include "VM.h"
#include "namedStrings.h"
#include "NativeFunction.h"
#include "Object.h"
#include "DisplayObject.h"

#include <iostream>

namespace gnash {

// Forward declarations
namespace {
    as_value function_apply(const fn_call& fn);
    as_value function_call(const fn_call& fn);
    as_object* getFunctionPrototype();
    as_value function_ctor(const fn_call& fn);
}

// This function returns the singleton
// instance of the ActionScript Function object
// prototype, which is what the AS Function class
// exports, thus what each AS function instance inherit.
// 
// The returned object can be accessed by ActionScript
// code through Function.__proto__.prototype.
// User AS code can add or modify members of this object
// to modify behaviour of all Function AS instances.
// 
// FIXME: do not use a static specifier for the proto
// object, as multiple runs of a single movie should
// each use a 'clean', unmodified, version of the
// prototype. What should really happen is that this
// prototype gets initializated by initialization of
// the Function class itself, which would be a member
// of the _global object for each movie instance.

as_function::as_function(Global_as& gl)
	:
	as_object(gl)
{
	int flags = PropFlags::dontDelete |
	            PropFlags::dontEnum | 
	            PropFlags::onlySWF6Up;
	init_member(NSV::PROP_uuPROTOuu, as_value(getFunctionPrototype()), flags);
}

const std::string&
as_function::stringValue() const
{
    // TODO: find out what AS3 functions return.
    static const std::string str("[type Function]");
    return str;
}


NativeFunction*
as_function::getFunctionConstructor()
{
	static NativeFunction* func = 0;
	if ( ! func )
	{
        Global_as& gl = *VM::get().getGlobal();
		func = new NativeFunction(gl, function_ctor);
        as_object* proto = getFunctionPrototype();

        func->init_member(NSV::PROP_PROTOTYPE, proto);
        func->init_member(NSV::PROP_CONSTRUCTOR, func);
		proto->init_member(NSV::PROP_CONSTRUCTOR, func); 
		VM::get().addStatic(func);
	}
	return func;
}

as_object*
constructInstance(as_function& ctor, const as_environment& env,
        fn_call::Args& args)
{
    Global_as& gl = getGlobal(ctor);

    // Create an empty object, with a ref to the constructor's prototype.
    // The function's prototype property always becomes the new object's
    // __proto__ member, regardless of whether it is an object and regardless
    // of its visibility.
    as_object* newobj = new as_object(gl);
    Property* proto = ctor.getOwnProperty(NSV::PROP_PROTOTYPE);
    if (proto) newobj->set_prototype(proto->getValue(ctor));

    return ctor.construct(*newobj, env, args);
}

as_object*
as_function::construct(as_object& newobj, const as_environment& env,
        fn_call::Args& args)
{
	const int swfversion = getSWFVersion(env);

    // Add a __constructor__ member to the new object visible from version 6.
    const int flags = PropFlags::dontEnum | 
                      PropFlags::onlySWF6Up; 

    newobj.init_member(NSV::PROP_uuCONSTRUCTORuu, this, flags);

    if (swfversion < 7) {
        newobj.init_member(NSV::PROP_CONSTRUCTOR, this, PropFlags::dontEnum);
    }
    
    // Don't set a super so that it will be constructed only if required
    // by the function.
    fn_call fn(&newobj, env, args, 0, true);
    as_value ret;

    try {
        ret = call(fn);
    }
    catch (GnashException& ex) {
        // Catching a std::exception here can mask all sorts of bad 
        // behaviour, as (for instance) a poorly constructed string may
        // smash the stack, throw an exception, but not abort.
        // This is very effective at confusing debugging tools.
        // We only throw GnashExceptions. A std::bad_alloc may also be
        // reasonable, but anything else shouldn't be caught here.
        log_debug("Native function called as constructor threw exception: "
                "%s", ex.what());

        // If a constructor throws an exception, throw it back to the
        // caller. This is the only way to signal that a constructor
        // did not return anything.
        throw;
    }

    // Some built-in constructors do things properly and operate on the
    // 'this' pointer. Others return a new object. This is to handle those
    // cases.
    if (isBuiltin() && ret.is_object()) {
        as_object* fakeobj = ret.to_object(getGlobal(env));

        fakeobj->init_member(NSV::PROP_uuCONSTRUCTORuu, as_value(this),
                flags);

        // Also for SWF5+ only?
        if (swfversion < 7) {
            fakeobj->init_member(NSV::PROP_CONSTRUCTOR, as_value(this),
                    PropFlags::dontEnum);
        }
        return fakeobj;
    }

	return &newobj;
}


void
registerFunctionNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(function_call, 101, 10);
    vm.registerNative(function_apply, 101, 11);
}

void
function_class_init(as_object& global, const ObjectURI& uri)
{
    NativeFunction* func = as_function::getFunctionConstructor();

	// Register _global.Function, only visible for SWF6 up
	int swf6flags = PropFlags::dontEnum | 
                    PropFlags::dontDelete | 
                    PropFlags::onlySWF6Up;
	global.init_member(uri, func, swf6flags);
}

namespace {

as_object*
getFunctionPrototype()
{

	static boost::intrusive_ptr<as_object> proto;

	if (proto.get() == NULL) {

		// Initialize Function prototype
        proto = VM::get().getGlobal()->createObject();
        
		// We initialize the __proto__ member separately, as getObjectInterface
		// will end up calling getFunctionPrototype again and we want that
		// call to return the still-not-completely-constructed prototype rather
		// then create a new one. 

        VM& vm = VM::get();

		vm.addStatic(proto.get());

		const int flags = as_object::DefaultFlags | PropFlags::onlySWF6Up; 

		proto->init_member("call", vm.getNative(101, 10), flags);
		proto->init_member("apply", vm.getNative(101, 11), flags);
	}

	return proto.get();

}

as_value
function_ctor(const fn_call& /*fn*/)
{
	return as_value();
}


class PushFunctionArgs
{
public:
    PushFunctionArgs(fn_call& fn) : _fn(fn) {}
    void operator()(const as_value& val) {
        _fn.pushArg(val);
    }
private:
    fn_call& _fn;
};

as_value
function_apply(const fn_call& fn)
{

	as_object* function_obj = ensure<ValidThis>(fn);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);
	new_fn_call.resetArgs();

	if (!fn.nargs)
	{
            IF_VERBOSE_ASCODING_ERRORS(
                log_aserror (_("Function.apply() called with no args"));
            );
            new_fn_call.this_ptr = new as_object(getGlobal(fn));
	}
	else
	{
		// Get the object to use as 'this' reference
		as_object* obj = fn.arg(0).to_object(getGlobal(fn));

        if (!obj) obj = new as_object(getGlobal(fn)); 

        new_fn_call.this_ptr = obj;

        // Note: do not override fn_call::super by creating a super
        // object, as it may not be needed. Doing so can have a very
        // detrimental effect on memory usage!
        // Normal supers will be created when needed in the function
        // call.
        new_fn_call.super = 0;

		// Check for second argument ('arguments' array)
		if (fn.nargs > 1)
		{
			IF_VERBOSE_ASCODING_ERRORS(
				if (fn.nargs > 2) {
					log_aserror(_("Function.apply() got %d"
						" args, expected at most 2"
						" -- discarding the ones in"
						" excess"),
						fn.nargs);
				}
			);

			boost::intrusive_ptr<as_object> arg1 = 
                fn.arg(1).to_object(getGlobal(fn));

            if (arg1) {
                PushFunctionArgs pa(new_fn_call);
                foreachArray(*arg1, pa);
            }
		}
	}

	// Call the function 
	as_value rv = function_obj->call(new_fn_call);

    return rv;
}

as_value
function_call(const fn_call& fn)
{

	as_object* function_obj = ensure<ValidThis>(fn);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);

    as_object* tp;

    if (!fn.nargs || fn.arg(0).is_undefined() || fn.arg(0).is_null()) {
        tp = new as_object(getGlobal(fn));
    }
    else tp = fn.arg(0).to_object(getGlobal(fn));

    new_fn_call.this_ptr = tp;
    new_fn_call.super = 0;
    if (fn.nargs) new_fn_call.drop_bottom();

	// Call the function 
	return function_obj->call(new_fn_call);

}

} // anonymous namespace
} // gnash namespace

