// DisplayObjectContainer_as.cpp:  ActionScript "DisplayObjectContainer" class, for Gnash.
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

#include "display/DisplayObjectContainer_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value displayobjectcontainer_addChildAt(const fn_call& fn);
    as_value displayobjectcontainer_areInaccessibleObjectsUnderPoint(const fn_call& fn);
    as_value displayobjectcontainer_contains(const fn_call& fn);
    as_value displayobjectcontainer_getChildAt(const fn_call& fn);
    as_value displayobjectcontainer_getChildByName(const fn_call& fn);
    as_value displayobjectcontainer_getChildIndex(const fn_call& fn);
    as_value displayobjectcontainer_getObjectsUnderPoint(const fn_call& fn);
    as_value displayobjectcontainer_removeChild(const fn_call& fn);
    as_value displayobjectcontainer_removeChildAt(const fn_call& fn);
    as_value displayobjectcontainer_setChildIndex(const fn_call& fn);
    as_value displayobjectcontainer_swapChildren(const fn_call& fn);
    as_value displayobjectcontainer_swapChildrenAt(const fn_call& fn);
    as_value displayobjectcontainer_ctor(const fn_call& fn);
    void attachDisplayObjectContainerInterface(as_object& o);
    void attachDisplayObjectContainerStaticInterface(as_object& o);
    as_object* getDisplayObjectContainerInterface();

}

// extern (used by Global.cpp)
void displayobjectcontainer_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&displayobjectcontainer_ctor, getDisplayObjectContainerInterface());
        attachDisplayObjectContainerStaticInterface(*cl);
    }

    // Register _global.DisplayObjectContainer
    global.init_member("DisplayObjectContainer", cl.get());
}

namespace {

void
attachDisplayObjectContainerInterface(as_object& o)
{
    o.init_member("addChildAt", new builtin_function(displayobjectcontainer_addChildAt));
    o.init_member("areInaccessibleObjectsUnderPoint", new builtin_function(displayobjectcontainer_areInaccessibleObjectsUnderPoint));
    o.init_member("contains", new builtin_function(displayobjectcontainer_contains));
    o.init_member("getChildAt", new builtin_function(displayobjectcontainer_getChildAt));
    o.init_member("getChildByName", new builtin_function(displayobjectcontainer_getChildByName));
    o.init_member("getChildIndex", new builtin_function(displayobjectcontainer_getChildIndex));
    o.init_member("getObjectsUnderPoint", new builtin_function(displayobjectcontainer_getObjectsUnderPoint));
    o.init_member("removeChild", new builtin_function(displayobjectcontainer_removeChild));
    o.init_member("removeChildAt", new builtin_function(displayobjectcontainer_removeChildAt));
    o.init_member("setChildIndex", new builtin_function(displayobjectcontainer_setChildIndex));
    o.init_member("swapChildren", new builtin_function(displayobjectcontainer_swapChildren));
    o.init_member("swapChildrenAt", new builtin_function(displayobjectcontainer_swapChildrenAt));
}

void
attachDisplayObjectContainerStaticInterface(as_object& o)
{

}

as_object*
getDisplayObjectContainerInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachDisplayObjectContainerInterface(*o);
    }
    return o.get();
}

as_value
displayobjectcontainer_addChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_areInaccessibleObjectsUnderPoint(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_contains(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getChildByName(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getObjectsUnderPoint(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_removeChild(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_removeChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_setChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_swapChildren(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_swapChildrenAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer_as> ptr =
        ensureType<DisplayObjectContainer_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new DisplayObjectContainer_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

