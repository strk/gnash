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

#include <iostream>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <string>
#include "log.h"
#include "dbus_ext.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function

using namespace std;

namespace gnash
{

as_value dbus_ext_setsockname(const fn_call& fn);

Dbus::Dbus() 
    : _name(0)
{
    GNASH_REPORT_FUNCTION;
}


Dbus::~Dbus() 
{
    GNASH_REPORT_FUNCTION;
}

void
Dbus::setSocketName(const char *sock)
{
    GNASH_REPORT_FUNCTION;
    _name = sock;
}

class dbus_as_object : public as_object
{
public:
    Dbus obj;
};

static void
attachInterface(as_object *obj)
{
    GNASH_REPORT_FUNCTION;
    obj->init_member("setSocketName", gl->createFunction(dbus_ext_setsockname));
}

static as_object*
getInterface()
{
    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
    }
    return o.get();
}

static as_value
dbus_ctor(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    dbus_as_object* obj = new dbus_as_object();

    attachInterface(obj);
    return as_value(obj); // will keep alive
//    printf ("Hello World from %s !!!\n", __PRETTY_FUNCTION__);
}

as_value
dbus_ext_setsockname(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<dbus_as_object> ptr = ensureType<dbus_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	ptr->obj.setSocketName(text.c_str());
	return as_value(true);
    }
    return as_value(false);
}

std::auto_ptr<as_object>
init_dbus_instance()
{
    return std::auto_ptr<as_object>(new dbus_as_object());
}

// const char *dbus_setmode(struct dbus_config *config, const char *mode);
extern "C" {
    void
    dbus_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
	    cl = new builtin_function(&dbus_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	obj.init_member("Dbus", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
