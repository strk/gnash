// BrowserInvokeEvent_as.cpp:  ActionScript "BrowserInvokeEvent" class, for Gnash.
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

#include "events/BrowserInvokeEvent_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value browserinvokeevent_ctor(const fn_call& fn);
    void attachBrowserInvokeEventInterface(as_object& o);
    void attachBrowserInvokeEventStaticInterface(as_object& o);
    as_object* getBrowserInvokeEventInterface();

}

// extern (used by Global.cpp)
void browserinvokeevent_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&browserinvokeevent_ctor, getBrowserInvokeEventInterface());
        attachBrowserInvokeEventStaticInterface(*cl);
    }

    // Register _global.BrowserInvokeEvent
    global.init_member("BrowserInvokeEvent", cl.get());
}

namespace {

void
attachBrowserInvokeEventInterface(as_object& o)
{
}

void
attachBrowserInvokeEventStaticInterface(as_object& o)
{

}

as_object*
getBrowserInvokeEventInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachBrowserInvokeEventInterface(*o);
    }
    return o.get();
}

as_value
browserinvokeevent_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new BrowserInvokeEvent_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

