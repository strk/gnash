// as_object.cpp:  ActionScript "ContextMenuItem" class, for Gnash.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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
#include "ContextMenuItem_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

// Forward declarations
namespace {
    as_value contextmenuitem_ctor(const fn_call& fn);
    as_value contextmenuitem_copy(const fn_call& fn);
    void attachContextMenuItemInterface(as_object& o);

}

// extern (used by Global.cpp)
void
contextmenuitem_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, contextmenuitem_ctor,
            attachContextMenuItemInterface, 0, uri);
}

namespace {

void
attachContextMenuItemInterface(as_object& o)
{
    const int flags = PropFlags::dontEnum |
                      PropFlags::dontDelete |
                      PropFlags::onlySWF7Up;

    Global_as& gl = getGlobal(o);
    o.init_member("copy", gl.createFunction(contextmenuitem_copy), flags);
}

as_value
contextmenuitem_copy(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);

    Global_as& gl = getGlobal(fn);
    string_table& st = getStringTable(fn);

    as_function* ctor =
        gl.getMember(st.find("ContextMenuItem")).to_function();

    if (!ctor) return as_value();

    fn_call::Args args;
    args += ptr->getMember(st.find("caption")),
        ptr->getMember(NSV::PROP_ON_SELECT),
        ptr->getMember(st.find("separatorBefore")),
        ptr->getMember(NSV::PROP_ENABLED),
        ptr->getMember(st.find("visible"));

    return constructInstance(*ctor, fn.env(), args);
}


as_value
contextmenuitem_ctor(const fn_call& fn)
{
    as_object* obj = fn.this_ptr;

    string_table& st = getStringTable(fn);

    obj->set_member(st.find("caption"), fn.nargs ? fn.arg(0) : as_value());
    obj->set_member(NSV::PROP_ON_SELECT, fn.nargs > 1 ? fn.arg(1) : as_value());
    obj->set_member(st.find("separatorBefore"), fn.nargs > 2 ?
            fn.arg(2) : false);
    obj->set_member(NSV::PROP_ENABLED, fn.nargs > 3 ? fn.arg(3) : true);
    obj->set_member(st.find("visible"), fn.nargs > 4 ? fn.arg(4) : true);

    return as_value(); 
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

