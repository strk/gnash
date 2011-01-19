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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>

#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include <string>
#include "log.h"
#include "lirc.h"
#include "lirc_ext.h"
#include "fn_call.h"
#include "as_object.h"
#include "Global_as.h"
#include "function_as.h"

using namespace std;

namespace gnash
{
struct lirc_config *config;
as_value lirc_ext_init(const fn_call& fn);
as_value lirc_ext_getkey(const fn_call& fn);
as_value lirc_ext_getbutton(const fn_call& fn);

class LircRelay : public Relay
{
public:
    Lirc obj;
};

static void
attachInterface(as_object& obj)
{
    GNASH_REPORT_FUNCTION;
    Global_as& gl = getGlobal(obj);

    obj.init_member("lirc_init", gl.createFunction(lirc_ext_init));
    obj.init_member("lirc_getKey", gl.createFunction(lirc_ext_getkey));
    obj.init_member("lirc_getButton", gl.createFunction(lirc_ext_getbutton));
}

static as_value
lirc_ctor(const fn_call&  fn)
{
    as_object* obj = ensure<ValidThis>(fn);

    obj->setRelay(new LircRelay());

    return as_value();
}


Lirc::Lirc() 
{
    GNASH_REPORT_FUNCTION;
}

Lirc::~Lirc()
{
    GNASH_REPORT_FUNCTION;
}

as_value
lirc_ext_init(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    LircRelay* ptr = ensure<ThisIsNative<LircRelay> >(fn);
    
    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
        return as_value(ptr->obj.init(text.c_str()));
    }
    return as_value(false);
}

as_value
lirc_ext_getkey(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    LircRelay* ptr = ensure<ThisIsNative<LircRelay> >(fn);
    
    if (fn.nargs == 0) {
        key::code key = ptr->obj.getKey();
        return as_value(key);
    }
    return as_value(false);
}

as_value
lirc_ext_getbutton(const fn_call& fn)
{
    LircRelay* ptr = ensure<ThisIsNative<LircRelay> >(fn);
    return as_value(ptr->obj.getButton());
}

extern "C" {
void
lirc_class_init(as_object &obj)
{

    Global_as& gl = getGlobal(obj);
    as_object* proto = createObject(gl);
    attachInterface(*proto);
	as_object* cl = gl.createClass(&lirc_ctor, proto);
	obj.init_member("Lirc", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
