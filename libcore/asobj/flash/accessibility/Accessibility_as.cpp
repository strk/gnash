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
    void attachAccessibilityInterface(as_object& o);
    void attachAccessibilityStaticInterface(as_object& o);
    as_object* getAccessibilityInterface();

    as_value Accessibility_isActive(const fn_call& fn);
    as_value Accessibility_active(const fn_call& fn);
    as_value Accessibility_updateProperties(const fn_call& fn);
    as_value Accessibility_sendEvent(const fn_call& fn);
}

class Accessibility_as : public as_object
{

public:

    Accessibility_as()
        :
        as_object(getAccessibilityInterface())
    {}
};

// extern (used by Global.cpp)
void accessibility_class_init(as_object& global, const ObjectURI& uri)
{

    boost::intrusive_ptr<as_object> obj = new as_object(getObjectInterface());
    attachAccessibilityInterface(*obj);
    // Register _global.Accessibility
    global.init_member("Accessibility", obj.get());
}

namespace {

void
attachAccessibilityInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    const int flags = as_prop_flags::dontDelete
                | as_prop_flags::readOnly;

    const VM& vm = getVM(o);
    // For swf v9 or greater, the isActive() method has been changed to a
    // the property "active".
    if ( vm.getSWFVersion() >= 9 ) {
    o.init_member("active", gl->createFunction(Accessibility_active), flags);
    } else {
    o.init_member("isActive", gl->createFunction(Accessibility_isActive), flags);
    o.init_member("sendEvent", gl->createFunction(Accessibility_sendEvent), flags);
    }
    
    o.init_member("updateProperties",
            gl->createFunction(Accessibility_updateProperties), flags);

}

void
attachAccessibilityStaticInterface(as_object& /*o*/)
{
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

as_value
accessibility_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new Accessibility_as;

    return as_value(obj.get()); // will keep alive
}


as_value
Accessibility_isActive(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Accessibility_active(const fn_call& /*fn*/)
{
    GNASH_REPORT_FUNCTION;
    
//     boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
//     UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value(false);
}

as_value
Accessibility_updateProperties(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
Accessibility_sendEvent(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

