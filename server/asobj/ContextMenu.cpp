// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
#include "config.h"
#endif

#include "ContextMenu.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function

namespace gnash {

void contextmenu_copy(const fn_call& fn);
void contextmenu_hidebuiltinitems(const fn_call& fn);
void contextmenu_ctor(const fn_call& fn);

static void
attachContextMenuInterface(as_object& o)
{
	o.set_member("copy", &contextmenu_copy);
	o.set_member("hidebuiltinitems", &contextmenu_hidebuiltinitems);
}

static as_object*
getContextMenuInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object();
		attachContextMenuInterface(*o);
	}
	return o.get();
}

class contextmenu_as_object: public as_object
{

public:

	contextmenu_as_object()
		:
		as_object(getContextMenuInterface())
	{}

	// override from as_object ?
	//const char* get_text_value() const { return "ContextMenu"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

void contextmenu_copy(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}
void contextmenu_hidebuiltinitems(const fn_call& /*fn*/) {
    log_warning("%s: unimplemented \n", __FUNCTION__);
}

void
contextmenu_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new contextmenu_as_object;
	
	fn.result->set_as_object(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void contextmenu_class_init(as_object& global)
{
	// This is going to be the global ContextMenu "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&contextmenu_ctor, getContextMenuInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachContextMenuInterface(*cl);
		     
	}

	// Register _global.ContextMenu
	global.set_member("ContextMenu", cl.get());

}


} // end of gnash namespace

