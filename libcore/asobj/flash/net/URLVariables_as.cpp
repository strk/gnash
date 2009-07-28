/ URLVariables_as.cpp:  ActionScript "URLVariables" class, for Gnash.
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

#include "net/URLVariables_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value urlvariables_toString(const fn_call& fn);
    as_value urlvariables_ctor(const fn_call& fn);
    void attachURLVariablesInterface(as_object& o);
    void attachURLVariablesStaticInterface(as_object& o);
    as_object* getURLVariablesInterface();

}

class URLVariables_as : public as_object
{

public:

    URLVariables_as()
        :
        as_object(getURLVariablesInterface())
    {}
};

// extern (used by Global.cpp)
void urlvariables_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&urlvariables_ctor, getURLVariablesInterface());
        attachURLVariablesStaticInterface(*cl);
    }

    // Register _global.URLVariables
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachURLVariablesInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(urlvariables_toString));
}

void
attachURLVariablesStaticInterface(as_object& o)
{
}

as_object*
getURLVariablesInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachURLVariablesInterface(*o);
    }
    return o.get();
}

as_value
urlvariables_toString(const fn_call& fn)
{
    boost::intrusive_ptr<URLVariables_as> ptr =
        ensureType<URLVariables_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
urlvariables_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new URLVariables_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

