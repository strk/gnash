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
    as_object* getBuiltInItemsObject();

}

class ContextMenu_as : public as_object
{

public:

    ContextMenu_as()
        :
        as_object(getContextMenuInterface())
    {}

private:

};


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

as_object*
getBuiltInItemsObject()
{
    as_object* o = new as_object();
    const int flags = 0;
    o->init_member("print", true, flags);
    o->init_member("forward_back", true, flags);
    o->init_member("rewind", true, flags);
    o->init_member("loop", true, flags);
    o->init_member("play", true, flags);
    o->init_member("quality", true, flags);
    o->init_member("zoom", true, flags);
    o->init_member("save", true, flags);
    return o;
}


void
attachContextMenuInterface(as_object& o)
{
    o.init_member("hideBuiltInItems",
            new builtin_function(contextmenu_hideBuiltInItems));
    o.init_member("copy", new builtin_function(contextmenu_copy));
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
    boost::intrusive_ptr<ContextMenu_as> ptr =
        ensureType<ContextMenu_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
contextmenu_copy(const fn_call& fn)
{
    boost::intrusive_ptr<ContextMenu_as> ptr =
        ensureType<ContextMenu_as>(fn.this_ptr);
    UNUSED(ptr);
    LOG_ONCE( log_unimpl (__FUNCTION__) );
    return as_value();
}

as_value
contextmenu_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new ContextMenu_as;

    // There is always an onSelect member, but it may be undefined.
    const as_value& callback = fn.nargs ? fn.arg(0) : as_value();
    obj->set_member(NSV::PROP_ON_SELECT, callback);
    
    string_table& st = fn.getVM().getStringTable();
    obj->set_member(st.find("builtInItems"), getBuiltInItemsObject());

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

