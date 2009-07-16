// DisplayObjectContainer_as.cpp:  ActionScript "DisplayObjectContainer" class.
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

#include "DisplayObjectContainer.h"
#include "display/DisplayObjectContainer_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" 
#include "builtin_function.h" 

namespace gnash {

// Forward declarations
namespace {
    as_value displayobjectcontainer_addChildAt(const fn_call& fn);
    as_value displayobjectcontainer_addChild(const fn_call& fn);
    as_value displayobjectcontainer_areInaccessibleObjectsUnderPoint(
            const fn_call& fn);
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
    as_value displayobjectcontainer_numChildren(const fn_call& fn);
    as_value displayobjectcontainer_ctor(const fn_call& fn);
    void attachDisplayObjectContainerInterface(as_object& o);
}

// extern (used by Global.cpp)
void
displayobjectcontainer_class_init(as_object& where, const ObjectURI& uri)
{
    
    // This should never be called during AVM1 execution!
    assert(isAS3(getVM(where)));

    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&displayobjectcontainer_ctor,
                getDisplayObjectContainerInterface());
    }

    // Register _global.DisplayObjectContainer
    where.init_member("DisplayObjectContainer", cl.get());
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

namespace {

void
attachDisplayObjectContainerInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);

    o.init_member("addChildAt", gl->createFunction(
                displayobjectcontainer_addChildAt));
    o.init_member("addChild", gl->createFunction(
                displayobjectcontainer_addChild));
    o.init_member("areInaccessibleObjectsUnderPoint", gl->createFunction(
                displayobjectcontainer_areInaccessibleObjectsUnderPoint));
    o.init_member("contains", gl->createFunction(
                displayobjectcontainer_contains));
    o.init_member("getChildAt", gl->createFunction(
                displayobjectcontainer_getChildAt));
    o.init_member("getChildByName", gl->createFunction(
                displayobjectcontainer_getChildByName));
    o.init_member("getChildIndex", gl->createFunction(
                displayobjectcontainer_getChildIndex));
    o.init_member("getObjectsUnderPoint", gl->createFunction(
                displayobjectcontainer_getObjectsUnderPoint));
    o.init_member("removeChild", gl->createFunction(
                displayobjectcontainer_removeChild));
    o.init_member("removeChildAt", gl->createFunction(
                displayobjectcontainer_removeChildAt));
    o.init_member("setChildIndex", gl->createFunction(
                displayobjectcontainer_setChildIndex));
    o.init_member("swapChildren", gl->createFunction(
                displayobjectcontainer_swapChildren));
    o.init_member("swapChildrenAt", gl->createFunction(
                displayobjectcontainer_swapChildrenAt));
    o.init_readonly_property("numChildren",
            displayobjectcontainer_numChildren);
}


as_value
displayobjectcontainer_addChild(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);

    as_value ret;

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("addChild(): %s", _("missing arguments"));
        );
        return ret;
    }

    if (fn.nargs > 1) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChild(%s): %s", ss.str(), _("ignoring args after "
                "the first"));
        );
    }

    as_object* objArg = fn.arg(0).to_object(*getGlobal(fn)).get();
    if (!objArg) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChild(%s): first arg doesn't cast to an object",
            ss.str());
        );
        return ret;
    }

    DisplayObject* ch = objArg->toDisplayObject();
    if (!ch) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChild(%s): first arg doesn't cast to a "
            "DisplayObject", ss.str());
        );
        return ret;
    }

    return as_value(ptr->addChild(ch));
}

as_value
displayobjectcontainer_addChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);

    as_value ret;

    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
        log_aserror("addChildAt(): %s", _("missing arguments"));
        );
        return ret;
    }

    if (fn.nargs > 2) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChildAt(%s): %s", ss.str(), _("ignoring args after "
                "the second"));
        );
    }

    as_object* objArg = fn.arg(0).to_object(*getGlobal(fn)).get();
    if (!objArg) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChildAt(%s): first arg doesn't cast to an object",
            ss.str());
        );
        return ret;
    }

    DisplayObject* ch = objArg->toDisplayObject();
    if (!ch) {
        IF_VERBOSE_ASCODING_ERRORS(
        std::stringstream ss; fn.dump_args(ss);
        log_aserror("addChildAt(%s): first arg doesn't cast to a "
            "DisplayObject", ss.str());
        );
        return ret;
    }

    int depth = fn.arg(1).to_number();

    std::stringstream ss; fn.dump_args(ss);
    log_debug("TESTING: addChildAt(%s)", ss.str());
    
    return as_value(ptr->addChildAt(ch, depth));

}

as_value
displayobjectcontainer_areInaccessibleObjectsUnderPoint(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_contains(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getChildByName(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_numChildren(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    return as_value(ptr->numChildren());
}

as_value
displayobjectcontainer_getChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_getObjectsUnderPoint(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_removeChild(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_removeChildAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_setChildIndex(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_swapChildren(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_swapChildrenAt(const fn_call& fn)
{
    boost::intrusive_ptr<DisplayObjectContainer> ptr =
        ensureType<DisplayObjectContainer>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
displayobjectcontainer_ctor(const fn_call& fn)
{
    // This should never be called during AS2 execution!
    assert(isAS3(fn));

    log_unimpl("Attempt to construct a DisplayObjectContainer should throw"
            "an exception!");
    return as_value();
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

