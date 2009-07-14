// IBitmapDrawable_as.cpp:  ActionScript "IBitmapDrawable" class, for Gnash.
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

#include "display/IBitmapDrawable_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value ibitmapdrawable_ctor(const fn_call& fn);
    void attachIBitmapDrawableInterface(as_object& o);
    void attachIBitmapDrawableStaticInterface(as_object& o);
    as_object* getIBitmapDrawableInterface();

}

class IBitmapDrawable_as : public as_object
{

public:

    IBitmapDrawable_as()
        :
        as_object(getIBitmapDrawableInterface())
    {}
};

// extern (used by Global.cpp)
void ibitmapdrawable_class_init(as_object& global)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(global);
        cl = gl->createClass(&ibitmapdrawable_ctor, getIBitmapDrawableInterface());
        attachIBitmapDrawableStaticInterface(*cl);
    }

    // Register _global.IBitmapDrawable
    global.init_member("IBitmapDrawable", cl.get());
}

namespace {

void
attachIBitmapDrawableInterface(as_object& o)
{
}

void
attachIBitmapDrawableStaticInterface(as_object& o)
{

}

as_object*
getIBitmapDrawableInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachIBitmapDrawableInterface(*o);
    }
    return o.get();
}

as_value
ibitmapdrawable_ctor(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = new IBitmapDrawable_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

