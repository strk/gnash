// Error_as.cpp:  ActionScript "Error" class, for Gnash.
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

#include "Error_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value Error_toString(const fn_call& fn);
static as_value Error_message_getset(const fn_call& fn);
static as_value Error_name_getset(const fn_call& fn);


as_value Error_ctor(const fn_call& fn);

static void
attachErrorInterface(as_object& o)
{
    o.init_member("toString", new builtin_function(Error_toString));
    o.init_property("message", Error_message_getset, Error_message_getset);
    o.init_property("name", Error_name_getset, Error_name_getset);
}


static as_object*
getErrorInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachErrorInterface(*o);

	}

	return o.get();
}

class Error_as: public as_object
{

public:

	Error_as()
		:
		as_object(getErrorInterface()),
		_name("Error"),
		_message("Error")
	{}

    void setName(const std::string& n) { _name = n; }
    const std::string& getName() const { return _name; }

    void setMessage(const std::string& m) { _message = m; }
    const std::string& getMessage() const { return _message; }

private:
    std::string _name;
    std::string _message;
};

static as_value
Error_toString(const fn_call& fn)
{
 	boost::intrusive_ptr<Error_as> ptr = ensureType<Error_as>(fn.this_ptr);
	return as_value(ptr->getMessage());   
}

static as_value
Error_message_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Error_as> ptr = ensureType<Error_as>(fn.this_ptr);
	if (fn.nargs == 0)
	{
	    // Getter
	    return as_value(ptr->getMessage());
	}
	else
	{
	    // Setter
	    ptr->setMessage(fn.arg(0).to_string());
	    return as_value();
	}
}

static as_value
Error_name_getset(const fn_call& fn)
{
	boost::intrusive_ptr<Error_as> ptr = ensureType<Error_as>(fn.this_ptr);
	
	if (fn.nargs == 0)
	{
	    // Getter
	    return as_value(ptr->getName());
	}
	else
	{
	    // Setter
	    ptr->setName(fn.arg(0).to_string());
	    return as_value();
	}
	
}



as_value
Error_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<Error_as> err = new Error_as;

	if ( fn.nargs > 0)
	{
		err->setMessage(fn.arg(0).to_string());
	}

	return as_value(err.get()); // will keep alive
}

// extern 
void Error_class_init(as_object& where)
{
	// This is going to be the Error "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl = new builtin_function(&Error_ctor, getErrorInterface());

	// Register _global.Error
	where.init_member("Error", cl.get());
}

} // end of gnash namespace
