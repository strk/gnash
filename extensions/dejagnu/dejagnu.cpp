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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <map>
#include <iostream>
#include <string>
#include "log.h"
#include "dejagnu.h"
#include "fn_call.h"
#include "as_object.h"
#include "builtin_function.h" // need builtin_function

using namespace std;

namespace gnash
{

as_value dejagnu_pass(const fn_call& fn);
as_value dejagnu_fail(const fn_call& fn);
as_value dejagnu_totals(const fn_call& fn);

class dejagnu_as_object : public as_object
{
public:
    DejaGnu obj;
};

static void
attachInterface(as_object *obj)
{
//    GNASH_REPORT_FUNCTION;

    obj->init_member("pass", gl->createFunction(dejagnu_pass));
    obj->init_member("fail", gl->createFunction(dejagnu_fail));
    obj->init_member("totals", gl->createFunction(dejagnu_totals));
}

static as_object*
getInterface()
{
//    GNASH_REPORT_FUNCTION;
    static boost::intrusive_ptr<as_object> o;
    if (o == NULL) {
	o = new as_object();
    }
    return o.get();
}

static as_value
dejagnu_ctor(const fn_call& /* fn */)
{
//    GNASH_REPORT_FUNCTION;
    dejagnu_as_object* obj = new dejagnu_as_object();

    attachInterface(obj);
    return as_value(obj); // will keep alive
//    printf ("Hello World from %s !!!\n", __PRETTY_FUNCTION__);
}


DejaGnu::DejaGnu() 
    : passed(0), failed(0), xpassed(0), xfailed(0)
{
//    GNASH_REPORT_FUNCTION;
}

DejaGnu::~DejaGnu()
{
//    GNASH_REPORT_FUNCTION;
}

const char *
DejaGnu::pass (const char *msg)
{
//    GNASH_REPORT_FUNCTION;

    passed++;
    log_debug("PASSED: %s\n", msg);
    return NULL;
}

const char *
DejaGnu::fail (const char *msg)
{
//    GNASH_REPORT_FUNCTION;

    failed++;
    log_debug("FAILED: %s\n", msg);
    return NULL;
}

as_value
dejagnu_pass(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<dejagnu_as_object> ptr = ensureType<dejagnu_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	return as_value(ptr->obj.pass(text.c_str()));
    }

    return as_value();
}

as_value
dejagnu_fail(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<dejagnu_as_object> ptr = ensureType<dejagnu_as_object>(fn.this_ptr);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	return as_value(ptr->obj.fail(text.c_str()));
    }

    return as_value();
}

as_value
dejagnu_totals(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<dejagnu_as_object> ptr = ensureType<dejagnu_as_object>(fn.this_ptr);
    
    ptr->obj.totals();
    return as_value(true);
}

    
std::auto_ptr<as_object>
init_dejagnu_instance()
{
    return std::auto_ptr<as_object>(new dejagnu_as_object());
}

extern "C" {
    void
    dejagnu_class_init(as_object &obj)
    {
//	GNASH_REPORT_FUNCTION;
	// This is going to be the global "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;
	if (cl == NULL) {
	    cl = new builtin_function(&dejagnu_ctor, getInterface());
// 	    // replicate all interface to class, to be able to access
// 	    // all methods as static functions
 	    attachInterface(cl.get());
	}
	
	obj.init_member("DejaGnu", cl.get());
    }
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
