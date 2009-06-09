// ContextMenu.cpp:  ActionScript ContextMenu class, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "ContextMenu.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "utility.h" // UNUSED

namespace gnash {

class ContextMenu: public as_object
{

public:

	ContextMenu()
		:
		as_object(getExportedInterface())
	{}

	ContextMenu(const as_value& callback)
		:
		as_object(getExportedInterface())
	{
		setCallback(callback);
	}

	ContextMenu(as_function* callback)
		:
		as_object(getExportedInterface())
	{
		setCallback(callback);
	}

	static void registerConstructor(as_object& global);

	// override from as_object ?
	//std::string get_text_value() const { return "ContextMenu"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }

private:

	/// Get the callback to call when user invokes the context menu.
	//
	/// If NULL, no action will be taken on select.
	///
	as_function* getCallback() 
	{
		as_value tmp;
		if (get_member(NSV::PROP_ON_SELECT, &tmp))
			return tmp.to_as_function();
		else return NULL;
	}

	/// Set the callback to call when user invokes the context menu.
	//
	/// @param callback
	///	The function to call. If the value is not a function, no
	///	action will be taken on select.
	///
	void setCallback(const as_value& callback)
	{
		set_member(NSV::PROP_ON_SELECT, callback);
	}

	/// Attach the exported interface of this ActionScript class
	/// to the given object.
	static void attachExportedInterface(as_object& o);

	/// Get the ContextMenu.prototype ActionScript object
	static as_object* getExportedInterface();

	static as_value ctor_method(const fn_call& fn);

	static as_value hideBuiltInItems_method(const fn_call& fn);

	static as_value copy_method(const fn_call& fn);
};

/* static private */
void
ContextMenu::attachExportedInterface(as_object& o)
{
	o.init_member("copy", new builtin_function(ContextMenu::copy_method));
	o.init_member("hideBuiltInItems", new builtin_function(ContextMenu::hideBuiltInItems_method));
}

/* static private */
as_object*
ContextMenu::getExportedInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachExportedInterface(*o);
	}
	return o.get();
}


/* static private */
as_value
ContextMenu::copy_method(const fn_call& fn)
{
	boost::intrusive_ptr<ContextMenu> ptr = ensureType<ContextMenu>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value();
}

/* static private */
as_value
ContextMenu::hideBuiltInItems_method(const fn_call& fn)
{
	boost::intrusive_ptr<ContextMenu> ptr = ensureType<ContextMenu>(fn.this_ptr);
	UNUSED(ptr);
	log_unimpl (__FUNCTION__);
	return as_value();
}

/* static private */
as_value
ContextMenu::ctor_method(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj;
	if ( fn.nargs > 0 )
       		obj = new ContextMenu(fn.arg(0));
	else
		obj = new ContextMenu();
	
	return as_value(obj.get()); // will keep alive
}

/* static public */
void
ContextMenu::registerConstructor(as_object& global)
{
	// This is going to be the global ContextMenu "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(ContextMenu::ctor_method, ContextMenu::getExportedInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		ContextMenu::attachExportedInterface(*cl);
		     
	}

	// Register _global.ContextMenu
	global.init_member("ContextMenu", cl.get());

}

// extern (used by Global.cpp)
void contextmenu_class_init(as_object& global)
{
	ContextMenu::registerConstructor(global);
}


} // end of gnash namespace
