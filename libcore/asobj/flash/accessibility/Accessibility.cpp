// Accessibility_as3.cpp:  ActionScript "Accessibility" class, for Gnash.
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

#include "accessibility/Accessibility_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

namespace gnash {

as_value accessibility_ctor(const fn_call& fn);
void attachAccessibilityInterface(as_object& o);
// void attachAccessibilityStaticInterface(as_object& o);
as_object* getAccessibilityInterface();

static as_value Accessibility_isActive(const fn_call& fn);
static as_value Accessibility_active(const fn_call& fn);
static as_value Accessibility_updateProperties(const fn_call& fn);
static as_value Accessibility_sendEvent(const fn_call& fn);

void
attachAccessibilityInterface(as_object& o)
{
    const int flags = as_prop_flags::dontDelete
                | as_prop_flags::readOnly;

    const VM& vm = o.getVM();
    // For swf v9 or greater, the isActive() method has been changed to a
    // the property "active".
    if ( vm.getSWFVersion() >= 9 ) {
	o.init_member("active", new builtin_function(Accessibility_active), flags);
    } else {
	o.init_member("isActive", new builtin_function(Accessibility_isActive), flags);
	o.init_member("sendEvent", new builtin_function(Accessibility_sendEvent), flags);
    }
    
    o.init_member("updateProperties", new builtin_function(Accessibility_updateProperties), flags);
}

as_object*
getAccessibilityInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachAccessibilityInterface(*o);
    }
    return o.get();
}

// extern (used by Global.cpp)
void accessibility_class_init(as_object& global)
{
    boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
    attachAccessibilityInterface(*obj);
    global.init_member("Accessibility", obj.get());
}

as_value
accessibility_ctor(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> obj = new Accessibility_as3;

    return as_value(obj.get()); // will keep alive
}

static as_value
Accessibility_isActive(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
// 	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
// 	UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value(false);
}

static as_value
Accessibility_active(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
// 	boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
// 	UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value(false);
}

static as_value
Accessibility_updateProperties(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

static as_value
Accessibility_sendEvent(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

