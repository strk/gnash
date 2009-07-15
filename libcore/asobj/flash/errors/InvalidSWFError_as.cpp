// InvalidSWFError_as.cpp:  ActionScript "InvalidSWFError" class, for Gnash.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "errors/InvalidSWFError_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value invalidswferror_ctor(const fn_call& fn);
    void attachInvalidSWFErrorInterface(as_object& o);
    void attachInvalidSWFErrorStaticInterface(as_object& o);
    as_object* getInvalidSWFErrorInterface();

}

class InvalidSWFError_as : public as_object
{

public:

    InvalidSWFError_as()
        :
        as_object(getInvalidSWFErrorInterface())
    {}
};

// extern (used by Global.cpp)
void invalidswferror_class_init(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&invalidswferror_ctor, getInvalidSWFErrorInterface());
        attachInvalidSWFErrorStaticInterface(*cl);
    }

    // Register _global.InvalidSWFError
    global.init_member("InvalidSWFError", cl.get());
}

namespace {

void
attachInvalidSWFErrorInterface(as_object& /*o*/)
{
}

void
attachInvalidSWFErrorStaticInterface(as_object& /*o*/)
{

}

as_object*
getInvalidSWFErrorInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachInvalidSWFErrorInterface(*o);
    }
    return o.get();
}

as_value
invalidswferror_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new InvalidSWFError_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

