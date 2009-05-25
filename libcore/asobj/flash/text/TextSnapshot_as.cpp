// TextSnapshot_as.cpp:  ActionScript "TextSnapshot" class, for Gnash.
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

#include "text/TextSnapshot_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value textsnapshot_getSelected(const fn_call& fn);
    as_value textsnapshot_getSelectedText(const fn_call& fn);
    as_value textsnapshot_getText(const fn_call& fn);
    as_value textsnapshot_getTextRunInfo(const fn_call& fn);
    as_value textsnapshot_hitTestTextNearPos(const fn_call& fn);
    as_value textsnapshot_setSelectColor(const fn_call& fn);
    as_value textsnapshot_setSelected(const fn_call& fn);
    as_value textsnapshot_ctor(const fn_call& fn);
    void attachTextSnapshotInterface(as_object& o);
    void attachTextSnapshotStaticInterface(as_object& o);
    as_object* getTextSnapshotInterface();

}

// extern (used by Global.cpp)
void textsnapshot_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&textsnapshot_ctor, getTextSnapshotInterface());
        attachTextSnapshotStaticInterface(*cl);
    }

    // Register _global.TextSnapshot
    global.init_member("TextSnapshot", cl.get());
}

namespace {

void
attachTextSnapshotInterface(as_object& o)
{
    o.init_member("getSelected", new builtin_function(textsnapshot_getSelected));
    o.init_member("getSelectedText", new builtin_function(textsnapshot_getSelectedText));
    o.init_member("getText", new builtin_function(textsnapshot_getText));
    o.init_member("getTextRunInfo", new builtin_function(textsnapshot_getTextRunInfo));
    o.init_member("hitTestTextNearPos", new builtin_function(textsnapshot_hitTestTextNearPos));
    o.init_member("setSelectColor", new builtin_function(textsnapshot_setSelectColor));
    o.init_member("setSelected", new builtin_function(textsnapshot_setSelected));
}

void
attachTextSnapshotStaticInterface(as_object& o)
{

}

as_object*
getTextSnapshotInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachTextSnapshotInterface(*o);
    }
    return o.get();
}

as_value
textsnapshot_getSelected(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_getSelectedText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_getText(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_getTextRunInfo(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_hitTestTextNearPos(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_setSelectColor(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_setSelected(const fn_call& fn)
{
    boost::intrusive_ptr<TextSnapshot_as> ptr =
        ensureType<TextSnapshot_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
textsnapshot_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new TextSnapshot_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

