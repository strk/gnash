// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#include "launcher_ext.h"
#include "fn_call.h"
#include "as_object.h"

using namespace std;

namespace gnash
{

as_value launcher_create_md5sum(const fn_call& fn);
as_value launcher_verify_md5sum(const fn_call& fn);

Launcher::Launcher() 
    : _name(0)
{
    GNASH_REPORT_FUNCTION;
}


Launcher::~Launcher() 
{
    GNASH_REPORT_FUNCTION;
}


void
Launcher::launch(const char *filespec)
{
    GNASH_REPORT_FUNCTION;
}

const char *
Launcher::create_md5sum(const char *filespec)
{
    GNASH_REPORT_FUNCTION;
}

bool
Launcher::verify_md5sum(const char *filespec, const char *md5)
{
    GNASH_REPORT_FUNCTION;
}

class launcher_as_object : public as_object
{
public:
    Launcher obj;
};

static void
attachInterface(as_object *obj)
{
    GNASH_REPORT_FUNCTION;
    obj->init_member("CreateMD5sum", new builtin_function(launcher_create_md5sum));
    obj->init_member("VerifyMD5sum", new builtin_function(launcher_verify_md5sum));
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
launcher_ctor(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    launcher_as_object* obj = new launcher_as_object();

    attachInterface(obj);
    return as_value(obj); // will keep alive
//    printf ("Hello World from %s !!!\n", __PRETTY_FUNCTION__);
}

as_value
launcher_create_md5sum(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<launcher_as_object> ptr = ensureType<launcher_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string filespec = fn.arg(0).to_string();
	return as_value(ptr->obj.create_md5sum(filespec.c_str()));
    }
    return as_value("");
}

as_value
launcher_verify_md5sum(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<launcher_as_object> ptr = ensureType<launcher_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string filespec = fn.arg(0).to_string();
	string md5 = fn.arg(1).to_string();
	return as_value(ptr->obj.verify_md5sum(filespec.c_str(), md5.c_str()));
    }
    return as_value(true);
}

std::auto_ptr<as_object>
init_launcher_instance()
{
    return std::auto_ptr<as_object>(new launcher_as_object());
}

extern "C" {
    void
    launcher_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
        Global_as* gl = getGlobal(global);
        as_object* proto = getInterface();
        cl = gl->createClass(&launcher_ctor, proto);
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	obj.init_member("Launcher", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
