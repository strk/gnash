// BlendMode_as.cpp:  ActionScript "BlendMode" class, for Gnash.
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

#include "display/BlendMode_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value blendmode_ctor(const fn_call& fn);
    void attachBlendModeInterface(as_object& o);
    void attachBlendModeStaticInterface(as_object& o);
    as_object* getBlendModeInterface();

}

class BlendMode_as : public as_object
{

public:

    BlendMode_as()
        :
        as_object(getBlendModeInterface())
    {}
};

// extern (used by Global.cpp)
void blendmode_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        as_object* proto = getBlendModeInterface();
        cl = gl->createClass(&blendmode_ctor, proto);
        attachBlendModeStaticInterface(*cl);
    }

    // Register _global.BlendMode
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachBlendModeInterface(as_object& /*o*/)
{
}

void
attachBlendModeStaticInterface(as_object& /*o*/)
{

}

as_object*
getBlendModeInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachBlendModeInterface(*o);
    }
    return o.get();
}

as_value
blendmode_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BlendMode_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

