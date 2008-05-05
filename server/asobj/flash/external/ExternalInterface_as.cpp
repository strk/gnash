// ExternalInterface_as.cpp:  ActionScript "ExternalInterface" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "ExternalInterface_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance

#include <sstream>

namespace gnash {

static as_value ExternalInterface_addCallback(const fn_call& fn);
static as_value ExternalInterface_call(const fn_call& fn);
static as_value ExternalInterface_available_getset(const fn_call& fn);
as_value ExternalInterface_ctor(const fn_call& fn);

static void
attachExternalInterfaceInterface(as_object& o)
{
    o.init_member("addCallback", new builtin_function(ExternalInterface_addCallback));
    o.init_member("call", new builtin_function(ExternalInterface_call));
    o.init_property("available", ExternalInterface_available_getset, ExternalInterface_available_getset);
}

static void
attachExternalInterfaceStaticProperties(as_object& o)
{
	// TODO: add static properties here
}

static as_object*
getExternalInterfaceInterface()
{
	boost::intrusive_ptr<as_object> o;
	// TODO: check if this class should inherit from Object
	//       or from a different class
	o = new as_object(getObjectInterface());
	attachExternalInterfaceInterface(*o);
	return o.get();
}

class ExternalInterface_as: public as_object
{

public:

	ExternalInterface_as()
		:
		as_object(getExternalInterfaceInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "ExternalInterface"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
ExternalInterface_addCallback(const fn_call& fn)
{
	boost::intrusive_ptr<ExternalInterface_as> ptr = ensureType<ExternalInterface_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ExternalInterface_call(const fn_call& fn)
{
	boost::intrusive_ptr<ExternalInterface_as> ptr = ensureType<ExternalInterface_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ExternalInterface_available_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ExternalInterface_as> ptr = ensureType<ExternalInterface_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
ExternalInterface_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new ExternalInterface_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("ExternalInterface(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void ExternalInterface_class_init(as_object& where)
{
	// This is going to be the ExternalInterface "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&ExternalInterface_ctor, getExternalInterfaceInterface());
	attachExternalInterfaceStaticProperties(*cl);

	// Register _global.ExternalInterface
	where.init_member("ExternalInterface", cl.get());
}

} // end of gnash namespace
