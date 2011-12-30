// accessibility_as.cpp:  ActionScript "Accessibility" class, for Gnash.
//
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "namedStrings.h"
#include "Accessibility_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"

namespace gnash {

// Forward declarations
namespace {
    void attachAccessibilityStaticInterface(as_object& o);

    as_value accessibility_isActive(const fn_call& fn);
    as_value accessibility_updateProperties(const fn_call& fn);
    as_value accessibility_sendEvent(const fn_call& fn);
}

// extern (used by Global.cpp)
void
accessibility_class_init(as_object& where, const ObjectURI& uri)
{

    Global_as& gl = getGlobal(where);

    const int flags = as_object::DefaultFlags | PropFlags::readOnly;

    // This object has unusual properties.
    as_object* obj = createObject(gl);
    obj->set_member_flags(NSV::PROP_uuPROTOuu, flags);
    obj->init_member(NSV::PROP_CONSTRUCTOR, getMember(gl, NSV::CLASS_OBJECT),
            flags);

    attachAccessibilityStaticInterface(*obj);

    // Register _global.Accessibility
    where.init_member(uri, obj, as_object::DefaultFlags);
}

void
registerAccessibilityNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(accessibility_isActive, 1999, 0);
    vm.registerNative(accessibility_sendEvent, 1999, 1);
    vm.registerNative(accessibility_updateProperties, 1999, 2);
}

namespace {

void
attachAccessibilityStaticInterface(as_object& o)
{
    const int flags = PropFlags::dontDelete |
                      PropFlags::readOnly;

    VM& vm = getVM(o);

    o.init_member("isActive", vm.getNative(1999, 0), flags);
    o.init_member("sendEvent", vm.getNative(1999, 1), flags);
    o.init_member("updateProperties", vm.getNative(1999, 2), flags);
}

as_value
accessibility_isActive(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
accessibility_updateProperties(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

as_value
accessibility_sendEvent(const fn_call& /*fn*/)
{
    LOG_ONCE(log_unimpl(__FUNCTION__) );
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:

