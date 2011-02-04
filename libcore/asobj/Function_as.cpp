// Function_as.cpp:  ActionScript "Function" class, for Gnash.
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
//

#include "Function_as.h"

#include "as_object.h"
#include "Global_as.h"
#include "as_value.h"
#include "Array_as.h"
#include "NativeFunction.h"
#include "fn_call.h"
#include "VM.h"
#include "log.h"
#include "namedStrings.h"

namespace gnash {

// Forward declarations
namespace {
    as_value function_apply(const fn_call& fn);
    as_value function_call(const fn_call& fn);
    as_value function_ctor(const fn_call& fn);
}

namespace {

/// Utility struct for pushing args to an array.
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

}

void
registerFunctionNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(function_call, 101, 10);
    vm.registerNative(function_apply, 101, 11);
}

void
function_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as& gl = getGlobal(where);

    NativeFunction* func = new NativeFunction(gl, function_ctor);
    as_object* proto = createObject(gl);

    func->init_member(NSV::PROP_PROTOTYPE, proto);
    func->init_member(NSV::PROP_CONSTRUCTOR, func);
    proto->init_member(NSV::PROP_CONSTRUCTOR, func); 

	// Register _global.Function, only visible for SWF6 up
	const int swf6flags = as_object::DefaultFlags | PropFlags::onlySWF6Up;
	func->init_member(NSV::PROP_uuPROTOuu, proto, swf6flags);
	where.init_member(uri, func, swf6flags);
    
    VM& vm = getVM(where);

    // Note: these are the first functions created, and they need the
    // Function class to be registered.
    proto->init_member("call", vm.getNative(101, 10), swf6flags);
    proto->init_member("apply", vm.getNative(101, 11), swf6flags);
}

namespace {

as_value
function_ctor(const fn_call& /*fn*/)
{
	return as_value();
}

as_value
function_apply(const fn_call& fn)
{

	as_object* function_obj = ensure<ValidThis>(fn);

	// Copy new function call from old one, we'll modify 
	// the copy only if needed
	fn_call new_fn_call(fn);
	new_fn_call.resetArgs();

	if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror (_("Function.apply() called with no args"));
        );
        new_fn_call.this_ptr = new as_object(getGlobal(fn));
	}
	else {
		// Get the object to use as 'this' reference
		as_object* obj = toObject(fn.arg(0), getVM(fn));

        if (!obj) obj = new as_object(getGlobal(fn)); 

        new_fn_call.this_ptr = obj;

        // Note: do not override fn_call::super by creating a super
        // object, as it may not be needed. Doing so can have a very
        // detrimental effect on memory usage!
        // Normal supers will be created when needed in the function
        // call.
        new_fn_call.super = 0;

		// Check for second argument ('arguments' array)
		if (fn.nargs > 1) {

			IF_VERBOSE_ASCODING_ERRORS(
				if (fn.nargs > 2) {
					log_aserror(_("Function.apply() got %d args, expected at "
                        "most 2 -- discarding the ones in excess"), fn.nargs);
				}
			);

			as_object* arg1 = toObject(fn.arg(1), getVM(fn));

            if (arg1) {
                PushFunctionArgs pa(new_fn_call);
                foreachArray(*arg1, pa);
            }
		}
	}

	// Call the function 
	return function_obj->call(new_fn_call);
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
    else tp = toObject(fn.arg(0), getVM(fn));

    new_fn_call.this_ptr = tp;
    new_fn_call.super = 0;
    if (fn.nargs) new_fn_call.drop_bottom();

	// Call the function 
	return function_obj->call(new_fn_call);

}

} // anonymous namespace
} // gnash namespace
