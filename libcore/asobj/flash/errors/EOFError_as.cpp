// EOFError_as.cpp:  ActionScript "EOFError" class, for Gnash.
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

#include "errors/EOFError_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value eoferror_ctor(const fn_call& fn);
    void attachEOFErrorInterface(as_object& o);
    void attachEOFErrorStaticInterface(as_object& o);
    as_object* getEOFErrorInterface();

}

class EOFError_as : public as_object
{

public:

    EOFError_as()
        :
        as_object(getEOFErrorInterface())
    {}
};

// extern (used by Global.cpp)
void eoferror_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&eoferror_ctor, getEOFErrorInterface());
        attachEOFErrorStaticInterface(*cl);
    }

    // Register _global.EOFError
    global.init_member("EOFError", cl.get());
}

namespace {

void
attachEOFErrorInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
}

void
attachEOFErrorStaticInterface(as_object& o)
{

}

as_object*
getEOFErrorInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachEOFErrorInterface(*o);
    }
    return o.get();
}

as_value
eoferror_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new EOFError_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

