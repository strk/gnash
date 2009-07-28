// FocusEvent_as.cpp:  ActionScript "FocusEvent" class, for Gnash.
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

#include "events/FocusEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value focusevent_toString(const fn_call& fn);
    as_value focusevent_FOCUS_IN(const fn_call& fn);
    as_value focusevent_FOCUS_OUT(const fn_call& fn);
    as_value focusevent_KEY_FOCUS_CHANGE(const fn_call& fn);
    as_value focusevent_MOUSE_FOCUS_CHANGE(const fn_call& fn);
    as_value focusevent_ctor(const fn_call& fn);
    void attachFocusEventInterface(as_object& o);
    void attachFocusEventStaticInterface(as_object& o);
    as_object* getFocusEventInterface();

}

class FocusEvent_as : public as_object
{

public:

    FocusEvent_as()
        :
        as_object(getFocusEventInterface())
    {}
};

// extern (used by Global.cpp)
void focusevent_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&focusevent_ctor, getFocusEventInterface());
        attachFocusEventStaticInterface(*cl);
    }

    // Register _global.FocusEvent
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachFocusEventInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(focusevent_toString));
    o.init_member("FOCUS_IN", gl->createFunction(focusevent_FOCUS_IN));
    o.init_member("FOCUS_OUT", gl->createFunction(focusevent_FOCUS_OUT));
    o.init_member("KEY_FOCUS_CHANGE", gl->createFunction(focusevent_KEY_FOCUS_CHANGE));
    o.init_member("MOUSE_FOCUS_CHANGE", gl->createFunction(focusevent_MOUSE_FOCUS_CHANGE));
}

void
attachFocusEventStaticInterface(as_object& /*o*/)
{
}

as_object*
getFocusEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachFocusEventInterface(*o);
    }
    return o.get();
}

as_value
focusevent_toString(const fn_call& fn)
{
    boost::intrusive_ptr<FocusEvent_as> ptr =
        ensureType<FocusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
focusevent_FOCUS_IN(const fn_call& fn)
{
    boost::intrusive_ptr<FocusEvent_as> ptr =
        ensureType<FocusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
focusevent_FOCUS_OUT(const fn_call& fn)
{
    boost::intrusive_ptr<FocusEvent_as> ptr =
        ensureType<FocusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
focusevent_KEY_FOCUS_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<FocusEvent_as> ptr =
        ensureType<FocusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
focusevent_MOUSE_FOCUS_CHANGE(const fn_call& fn)
{
    boost::intrusive_ptr<FocusEvent_as> ptr =
        ensureType<FocusEvent_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
focusevent_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new FocusEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

