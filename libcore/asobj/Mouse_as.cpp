// Mouse.cpp:  ActionScript "Mouse" input device class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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
#include "Mouse_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h" 
#include "VM.h" // for registerNative
#include "AsBroadcaster.h" // for initializing self as a broadcaster
#include "movie_root.h" // for GUI callback

namespace gnash {

// Forward declarations
namespace {    
    as_value mouse_hide(const fn_call& fn);
    as_value mouse_show(const fn_call& fn);

    void attachMouseInterface(as_object& o);
}

/// Mouse isn't a proper class in AS
//
/// Gnash's Mouse_as just has static methods.
void
Mouse_as::registerNative(as_object& o)
{
    VM& vm = getVM(o);

    vm.registerNative(mouse_show, 5, 0);
    vm.registerNative(mouse_hide, 5, 1);
}


// extern (used by Global.cpp)
void
mouse_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinObject(where, attachMouseInterface, uri);
}


namespace {

void
attachMouseInterface(as_object& o)
{
    VM& vm = getVM(o);

    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::readOnly;

    o.init_member("show", vm.getNative(5, 0), flags);
    o.init_member("hide", vm.getNative(5, 1), flags);
 
    // Mouse is always initialized as an AsBroadcaster, even for
    // SWF5.   
    AsBroadcaster::initialize(o);

    Global_as& gl = getGlobal(o);
    as_object* null = nullptr;
    callMethod(&gl, NSV::PROP_AS_SET_PROP_FLAGS, &o, null, 7);
}

/// Returns whether the mouse was visible before the call.
//
/// The return is not a boolean, but rather 1 or 0.
as_value
mouse_hide(const fn_call& fn)
{
    movie_root& m = getRoot(fn);
    const int success =
        m.callInterface<bool>(HostMessage(HostMessage::SHOW_MOUSE, false));

    // returns 1 if mouse was visible before call.
    return as_value(success);
}

/// Returns whether the mouse was visible before the call.
//
/// The return is not a boolean, but rather 1 or 0.
as_value
mouse_show(const fn_call& fn)
{
    movie_root& m = getRoot(fn);
    const int success = 
        m.callInterface<bool>(HostMessage(HostMessage::SHOW_MOUSE, true));

    // returns 1 if Mouse was visible before call.
    return as_value(success);
}

} // anonymous namespace
} // end of gnash namespace
