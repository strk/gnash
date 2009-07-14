// ContextMenu_as.cpp:  ActionScript "ContextMenu" class, for Gnash.
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

#include "ui/ContextMenu_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "Array_as.h"

namespace gnash {

// Forward declarations
namespace {
    as_value contextmenu_hideBuiltInItems(const fn_call& fn);
    as_value contextmenu_copy(const fn_call& fn);
    as_value contextmenu_ctor(const fn_call& fn);

    void attachContextMenuInterface(as_object& o);
    void attachContextMenuStaticInterface(as_object& o);
    as_object* getContextMenuInterface();
    as_object* setBuiltInItems();

}

// extern (used by Global.cpp)
void
contextmenu_class_init(as_object& global)
{
	static boost::intrusive_ptr<builtin_function> cl;

	if (cl == NULL) {
		cl=new builtin_function(contextmenu_ctor, getContextMenuInterface());
	}

	// Register _global.ContextMenu
	global.init_member("ContextMenu", cl.get());
}


namespace {

void
setBuiltInItems(as_object& o, bool setting)
{
    const int flags = 0;
    string_table& st = getStringTable(o);
    o.set_member(st.find("print"), setting, flags);
    o.set_member(st.find("forward_back"), setting, flags);
    o.set_member(st.find("rewind"), setting, flags);
    o.set_member(st.find("loop"), setting, flags);
    o.set_member(st.find("play"), setting, flags);
    o.set_member(st.find("quality"), setting, flags);
    o.set_member(st.find("zoom"), setting, flags);
    o.set_member(st.find("save"), setting, flags);
}


void
attachContextMenuInterface(as_object& o)
{
    const int flags = as_prop_flags::dontDelete |
                      as_prop_flags::dontEnum |
                      as_prop_flags::onlySWF7Up;

    o.init_member("hideBuiltInItems",
            gl->createFunction(contextmenu_hideBuiltInItems), flags);
    o.init_member("copy", gl->createFunction(contextmenu_copy), flags);
}

as_object*
getContextMenuInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object(getObjectInterface());
        attachContextMenuInterface(*o);
        VM::get().addStatic(o.get());
    }
    return o.get();
}

as_value
contextmenu_hideBuiltInItems(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    string_table& st = getStringTable(fn);
    as_object* builtIns = new as_object;
    setBuiltInItems(*builtIns, false);
    ptr->set_member(st.find("builtInItems"), builtIns);
    return as_value();
}

as_value
contextmenu_copy(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> ptr = ensureType<as_object>(fn.this_ptr);
    as_object* o = new as_object(getContextMenuInterface());
    
    string_table& st = getStringTable(fn);
    as_value onSelect, builtInItems;
    as_value customItems = new Array_as;;

    ptr->get_member(NSV::PROP_ON_SELECT, &onSelect);
    ptr->get_member(st.find("builtInItems"), &builtInItems);
    ptr->get_member(st.find("customItems"), &customItems);

    // The onSelect and the builtInItems property are simple copies, which
    // means the new object has a reference to the same object.
    o->set_member(NSV::PROP_ON_SELECT, onSelect);
    o->set_member(st.find("builtInItems"), builtInItems);

    // The customItems object is a deep copy, but only of elements that are
    // instances of ContextMenuItem (have its prototype as a __proto__ member).
    as_object* nc = new Array_as;
    as_object* customs;

    if (customItems.is_object() && (customs = customItems.to_object().get())) {
        // TODO: only copy properties that are ContextMenuItems.
        nc->copyProperties(*customs);
        customItems = nc;
    }

    o->set_member(st.find("customItems"), customItems);

    return as_value(o);
}

as_value
contextmenu_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj =
        new as_object(getContextMenuInterface());

    // There is always an onSelect member, but it may be undefined.
    const as_value& callback = fn.nargs ? fn.arg(0) : as_value();
    obj->set_member(NSV::PROP_ON_SELECT, callback);
    
    string_table& st = getStringTable(fn);
    as_object* builtInItems = new as_object;
    setBuiltInItems(*builtInItems, true);
    obj->set_member(st.find("builtInItems"), builtInItems);

    // There is an empty customItems array.
    Array_as* customItems = new Array_as();
    obj->set_member(st.find("customItems"), customItems);

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

