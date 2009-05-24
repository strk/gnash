// MemoryError_as3.cpp:  ActionScript "MemoryError" class, for Gnash.
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

#include "errors/MemoryError_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value memoryerror_ctor(const fn_call& fn);
    void attachMemoryErrorInterface(as_object& o);
    void attachMemoryErrorStaticInterface(as_object& o);
    as_object* getMemoryErrorInterface();

}

// extern (used by Global.cpp)
void memoryerror_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&memoryerror_ctor, getMemoryErrorInterface());
        attachMemoryErrorStaticInterface(*cl);
    }

    // Register _global.MemoryError
    global.init_member("MemoryError", cl.get());
}

namespace {

void
attachMemoryErrorInterface(as_object& o)
{
}

void
attachMemoryErrorStaticInterface(as_object& o)
{

}

as_object*
getMemoryErrorInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachMemoryErrorInterface(*o);
    }
    return o.get();
}

as_value
memoryerror_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new MemoryError_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

