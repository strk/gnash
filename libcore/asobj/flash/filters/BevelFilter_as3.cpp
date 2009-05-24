// BevelFilter_as3.cpp:  ActionScript "BevelFilter" class, for Gnash.
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

#include "filters/BevelFilter_as3.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value bevelfilter_ctor(const fn_call& fn);
    void attachBevelFilterInterface(as_object& o);
    void attachBevelFilterStaticInterface(as_object& o);
    as_object* getBevelFilterInterface();

}

// extern (used by Global.cpp)
void bevelfilter_class_init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&bevelfilter_ctor, getBevelFilterInterface());
        attachBevelFilterStaticInterface(*cl);
    }

    // Register _global.BevelFilter
    global.init_member("BevelFilter", cl.get());
}

namespace {

void
attachBevelFilterInterface(as_object& o)
{
}

void
attachBevelFilterStaticInterface(as_object& o)
{

}

as_object*
getBevelFilterInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachBevelFilterInterface(*o);
    }
    return o.get();
}

as_value
bevelfilter_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new BevelFilter_as3;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

