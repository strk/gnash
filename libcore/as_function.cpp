// as_function.cpp:  ActionScript Functions, for Gnash.
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

#include "as_function.h"

#include <string>

#include "log.h"
#include "as_value.h"
#include "fn_call.h"
#include "GnashException.h"
#include "Global_as.h"
#include "namedStrings.h"

namespace gnash {

as_function::as_function(Global_as& gl)
	:
	as_object(gl)
{
}

std::string
as_function::stringValue() const
{
    return "[type Function]";
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
    catch (const GnashException& ex) {
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
        as_object* fakeobj = toObject(ret, getVM(env));

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

} // namespace gnash
