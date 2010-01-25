// Error_as.cpp:  ActionScript "Error" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "Error_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "VM.h"

#include <sstream>

namespace gnash {

namespace {
    as_value error_toString(const fn_call& fn);
    as_value error_ctor(const fn_call& fn);

    void attachErrorInterface(as_object& o);
}


// extern 
void
Error_class_init(as_object& where, const ObjectURI& uri)
{
    registerBuiltinClass(where, error_ctor, attachErrorInterface, 0, uri);   
}


namespace {

void
attachErrorInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    int flags = 0;

    o.init_member("toString", gl.createFunction(error_toString), flags);
    
    o.init_member("message", "Error", flags);
    o.init_member("name", "Error", flags);
}


as_value
error_toString(const fn_call& fn)
{
 	boost::intrusive_ptr<as_object> ptr = ensure<ThisIs<as_object> >(fn);

    string_table& st = getStringTable(*ptr);
    as_value message;
    ptr->get_member(st.find("message"), &message);

	return as_value(message);   
}

/// "e = new Error();" returns an Error, "e = Error"; returns undefined.
as_value
error_ctor(const fn_call& fn)
{

    if (!fn.isInstantiation()) return as_value();

    as_object* err = fn.this_ptr;

    string_table& st = getStringTable(fn);

    if (fn.nargs) {
		err->set_member(st.find("message"), fn.arg(0));
	}

	return as_value();
}

}
} // end of gnash namespace
