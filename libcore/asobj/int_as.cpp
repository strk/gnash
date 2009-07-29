// EventDispatcher.cpp:  Implementation of ActionScript int class, for Gnash.
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

#include "smart_ptr.h"
#include "fn_call.h"
#include "Global_as.h"
#include "as_object.h" // for inheritance
#include "builtin_function.h" // need builtin_function
#include "Object.h"

#include "log.h"

#include <string>
#include <sstream>

namespace gnash {
class int_as_object : public as_object
{

public:

	int_as_object()
		:
		as_object()
	{
	}

};

static as_value
int_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new int_as_object();

    if ( fn.nargs )
    {
        LOG_ONCE( log_unimpl("Arguments passed to int() ctor unhandled") );
    }
	
	return as_value(obj.get()); // will keep alive
}

as_object*
getintInterface()
{
	static boost::intrusive_ptr<as_object> o;
	if ( ! o )
	{
		o = new as_object(getObjectInterface());
	}
	return o.get();
}

// extern (used by Global.cpp)
void int_class_init(as_object& global, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

        Global_as* gl = getGlobal(global);
        as_object* proto = getintInterface();
        cl = gl->createClass(&int_ctor, proto);

	// Register _global.DisplayObject
	global.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

std::auto_ptr<as_object>
init_int_instance()
{
	return std::auto_ptr<as_object>(new int_as_object);
}


}
