// Error.cpp:  ActionScript "Error" class, for Gnash.
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

#include "Error.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "Object.h" // for getObjectInterface

namespace gnash {

as_value error_tostring(const fn_call& fn);
as_value error_ctor(const fn_call& fn);

static void
attachErrorInterface(as_object& o)
{
	// is this really needed ? shouldn't toString be
	// derived from Object inheritance ?
	o.init_member("toString", new builtin_function(error_tostring));
}

static as_object*
getErrorInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		attachErrorInterface(*o);
	}
	return o.get();
}

class error_as_object: public as_object
{

public:

	error_as_object()
		:
		as_object(getErrorInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "Error"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};

as_value error_tostring(const fn_call& /*fn*/) {
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
error_ctor(const fn_call& /* fn */)
{
	boost::intrusive_ptr<as_object> obj = new error_as_object;
	
	return as_value(obj.get()); // will keep alive
}

// extern (used by Global.cpp)
void error_class_init(as_object& global)
{
	// This is going to be the global Error "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&error_ctor, getErrorInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachErrorInterface(*cl);
		     
	}

	// Register _global.Error
	global.init_member("Error", cl.get());

}


} // end of gnash namespace

