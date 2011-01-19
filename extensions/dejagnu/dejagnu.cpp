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
#include "as_function.h"
#include "Global_as.h"

using namespace std;

namespace gnash
{

as_value dejagnu_pass(const fn_call& fn);
as_value dejagnu_fail(const fn_call& fn);
as_value dejagnu_totals(const fn_call& fn);

class DejaGnu : public Relay
{
public:
    DejaGnu();
    ~DejaGnu();
    const char *pass (const char *msg);
    const char *fail (const char *msg);
    const char *xpass (const char *msg);
    const char *xfail (const char *msg);
    void totals ();
private:
    int passed;
    int failed;
    int xpassed;
    int xfailed;
};

static void
attachInterface(as_object& obj)
{
    Global_as& gl = getGlobal(obj);
    
    obj.init_member("pass", gl.createFunction(dejagnu_pass));
    obj.init_member("fail", gl.createFunction(dejagnu_fail));
    obj.init_member("totals", gl.createFunction(dejagnu_totals));
}

static as_value
dejagnu_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new DejaGnu());
    return as_value(); 
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
    passed++;
    log_debug("PASSED: %s\n", msg);
    return NULL;
}

const char *
DejaGnu::fail (const char *msg)
{
    failed++;
    log_debug("FAILED: %s\n", msg);
    return NULL;
}

as_value
dejagnu_pass(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    DejaGnu* ptr = ensure<ThisIsNative<DejaGnu> >(fn);
    
    if (fn.nargs > 0) {
	string text = fn.arg(0).to_string();
	return as_value(ptr->pass(text.c_str()));
    }

    return as_value();
}

as_value
dejagnu_fail(const fn_call& fn)
{
//    GNASH_REPORT_FUNCTION;
    DejaGnu* ptr = ensure<ThisIsNative<DejaGnu> >(fn);
    
    if (fn.nargs > 0) {
        string text = fn.arg(0).to_string();
        return as_value(ptr->fail(text.c_str()));
    }

    return as_value();
}

as_value
dejagnu_totals(const fn_call& fn)
{
    DejaGnu* ptr = ensure<ThisIsNative<DejaGnu> >(fn);
    
    ptr->totals();
    return as_value(true);
}

    
extern "C" {
void
dejagnu_class_init(as_object &obj)
{
    Global_as& gl = getGlobal(obj);
    as_object* proto = createObject(gl);
    attachInterface(*proto);

    as_object* cl = gl.createClass(&dejagnu_ctor, proto);
	
	obj.init_member("DejaGnu", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
