// Accessibility_as.cpp:  ActionScript "Accessibility" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "accessibility/Accessibility_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for AS inheritance

namespace gnash {

// Forward declarations
namespace {
    as_value accessibility_ctor(const fn_call& fn);
    void attachAccessibilityStaticInterface(as_object& o);
    void attachAccessibilityAS3StaticInterface(as_object& o);
    as_object* getAccessibilityInterface();

    as_value Accessibility_isActive(const fn_call& fn);
    as_value Accessibility_active(const fn_call& fn);
    as_value Accessibility_updateProperties(const fn_call& fn);
    as_value Accessibility_sendEvent(const fn_call& fn);
}

// extern (used by Global.cpp)
void
accessibility_class_init(as_object& where, const ObjectURI& uri)
{

    Global_as* gl = getGlobal(where);
    as_object* proto = getObjectInterface();

    const int flags = as_object::DefaultFlags | PropFlags::readOnly;

    // This object has unusual properties.
    as_object* obj = gl->createObject(proto);
    obj->set_member_flags(NSV::PROP_uuPROTOuu, flags);
    obj->init_member(NSV::PROP_CONSTRUCTOR, gl->getMember(NSV::CLASS_OBJECT),
            flags);

    attachAccessibilityStaticInterface(*obj);

    // Register _global.Accessibility
    where.init_member(getName(uri), obj, as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachAccessibilityAS3StaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("active", gl->createFunction(Accessibility_active));
}

void
attachAccessibilityStaticInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    
    const int flags = PropFlags::dontDelete |
                      PropFlags::readOnly;

    o.init_member("isActive",
            gl->createFunction(Accessibility_isActive), flags);
    o.init_member("sendEvent",
            gl->createFunction(Accessibility_sendEvent), flags);
    o.init_member("updateProperties",
            gl->createFunction(Accessibility_updateProperties), flags);
}

as_value
Accessibility_isActive(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Accessibility_active(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value(false);
}

as_value
Accessibility_updateProperties(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Accessibility_sendEvent(const fn_call& /*fn*/)
{
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

