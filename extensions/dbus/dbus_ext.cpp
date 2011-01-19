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
#include "dbus_ext.h"
#include "fn_call.h"
#include "as_object.h"
#include "Global_as.h"

using namespace std;

namespace gnash
{

as_value dbus_ext_setsockname(const fn_call& fn);

class Dbus : public Relay
{
public:
    Dbus();
    ~Dbus();
    void setSocketName(const char *sock);
private:
    std::string _name;
};

Dbus::Dbus() 
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

static void
attachInterface(as_object& obj)
{
    GNASH_REPORT_FUNCTION;
    Global_as& gl = getGlobal(obj);
    obj.init_member("setSocketName", gl.createFunction(dbus_ext_setsockname));
}

static as_value
dbus_ctor(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new Dbus());

    return as_value(); 
}

as_value
dbus_ext_setsockname(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    Dbus* ptr = ensure<ThisIsNative<Dbus> >(fn);
    
    if (fn.nargs > 0) {
        const std::string& text = fn.arg(0).to_string();
        ptr->setSocketName(text.c_str());
        return as_value(true);
    }
    return as_value(false);
}

// const char *dbus_setmode(struct dbus_config *config, const char *mode);
extern "C" {

void
dbus_class_init(as_object &obj)
{
    Global_as& gl = getGlobal(obj);
    as_object* proto = createObject(gl);
    attachInterface(*proto);
    as_object* cl = gl.createClass(&dbus_ctor, proto);
	
	obj.init_member("Dbus", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
