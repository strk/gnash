// Stage_as.cpp:  ActionScript "Stage" class, for Gnash.
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

#include "display/Stage_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value stage_addChildAt(const fn_call& fn);
    as_value stage_addEventListener(const fn_call& fn);
    as_value stage_dispatchEvent(const fn_call& fn);
    as_value stage_hasEventListener(const fn_call& fn);
    as_value stage_invalidate(const fn_call& fn);
    as_value stage_isFocusInaccessible(const fn_call& fn);
    as_value stage_removeChildAt(const fn_call& fn);
    as_value stage_setChildIndex(const fn_call& fn);
    as_value stage_swapChildrenAt(const fn_call& fn);
    as_value stage_willTrigger(const fn_call& fn);
    as_value stage_fullScreen(const fn_call& fn);
    as_value stage_mouseLeave(const fn_call& fn);
    as_value stage_resize(const fn_call& fn);
    as_value stage_ctor(const fn_call& fn);
    void attachStageInterface(as_object& o);
    void attachStageStaticInterface(as_object& o);
    as_object* getStageInterface();

}

// extern (used by Global.cpp)
void stage_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&stage_ctor, getStageInterface());
        attachStageStaticInterface(*cl);
    }

    // Register _global.Stage
    global.init_member("Stage", cl.get());
}

namespace {

void
attachStageInterface(as_object& o)
{
    o.init_member("addChildAt", new builtin_function(stage_addChildAt));
    o.init_member("addEventListener", new builtin_function(stage_addEventListener));
    o.init_member("dispatchEvent", new builtin_function(stage_dispatchEvent));
    o.init_member("hasEventListener", new builtin_function(stage_hasEventListener));
    o.init_member("invalidate", new builtin_function(stage_invalidate));
    o.init_member("isFocusInaccessible", new builtin_function(stage_isFocusInaccessible));
    o.init_member("removeChildAt", new builtin_function(stage_removeChildAt));
    o.init_member("setChildIndex", new builtin_function(stage_setChildIndex));
    o.init_member("swapChildrenAt", new builtin_function(stage_swapChildrenAt));
    o.init_member("willTrigger", new builtin_function(stage_willTrigger));
    o.init_member("fullScreen", new builtin_function(stage_fullScreen));
    o.init_member("mouseLeave", new builtin_function(stage_mouseLeave));
    o.init_member("resize", new builtin_function(stage_resize));
}

void
attachStageStaticInterface(as_object& o)
{

}

as_object*
getStageInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachStageInterface(*o);
    }
    return o.get();
}

as_value
stage_addChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_addEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_dispatchEvent(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_hasEventListener(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_invalidate(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_isFocusInaccessible(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_removeChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_setChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_swapChildrenAt(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_willTrigger(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_fullScreen(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_mouseLeave(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_resize(const fn_call& fn)
{
    boost::intrusive_ptr<Stage_as> ptr =
        ensureType<Stage_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
stage_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new Stage_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

