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
#include "metome_ext.h"
#include "fn_call.h"
#include "as_object.h"
#include "Globals.h"
#include "builtin_function.h" // need builtin_function

using namespace std;

namespace gnash
{

as_value metome_ext_connect(const fn_call& fn);

class Metome : public Relay
{
public:
    Metome();
    ~Metome();
    void connect(const char *sock);
private:
    const char *_name;
};


Metome::Metome() 
    : _name(0)
{
    GNASH_REPORT_FUNCTION;
}


Metome::~Metome() 
{
    GNASH_REPORT_FUNCTION;
}

void
Metome::connect(const char *sock)
{
    GNASH_REPORT_FUNCTION;
    _name = sock;
}

static void
attachInterface(as_object& obj)
{
    GNASH_REPORT_FUNCTION;
    Global_as& gl = getGlobal(obj);
    obj.init_member("connect", gl.createFunction(metome_ext_connect));
}

static as_value
metome_ctor(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new Metome);

    return as_value(); 
}

as_value
metome_ext_setsockname(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    Metome* ptr = ensure<ThisIsNative<Metome> >(fn);
    
    if (fn.nargs > 0) {
        string text = fn.arg(0).to_string();
        ptr->connect(text.c_str());
        return as_value(true);
    }
    return as_value(false);
}

// const char *metome_setmode(struct metome_config *config, const char *mode);
extern "C" {
void
metome_class_init(as_object &obj)
{
	// This is going to be the global "class"/"function"
    Global_as& gl = getGlobal(obj);
    as_object* proto = gl.createObject();
    attachInterface(*proto);
    as_object* cl = gl.createClass(&metome_ctor, proto);
	
	obj.init_member("Metome", cl);
}
} // end of extern C


} // end of gnash namespace

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
