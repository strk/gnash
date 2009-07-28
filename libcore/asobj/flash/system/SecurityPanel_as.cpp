// SecurityPanel_as.cpp:  ActionScript "SecurityPanel" class, for Gnash.
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

#include "system/SecurityPanel_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value securitypanel_ctor(const fn_call& fn);
    void attachSecurityPanelInterface(as_object& o);
    void attachSecurityPanelStaticInterface(as_object& o);
    as_object* getSecurityPanelInterface();

}

class SecurityPanel_as : public as_object
{

public:

    SecurityPanel_as()
        :
        as_object(getSecurityPanelInterface())
    {}
};

// extern (used by Global.cpp)
void securitypanel_class_init(as_object& where, const ObjectURI& uri)
{
    static boost::intrusive_ptr<as_object> cl;

    if (!cl) {
        Global_as* gl = getGlobal(where);
        cl = gl->createClass(&securitypanel_ctor, getSecurityPanelInterface());
        attachSecurityPanelStaticInterface(*cl);
    }

    // Register _global.SecurityPanel
    where.init_member(getName(uri), cl.get(), as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachSecurityPanelInterface(as_object& /*o*/)
{
}

void
attachSecurityPanelStaticInterface(as_object& /*o*/)
{

}

as_object*
getSecurityPanelInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachSecurityPanelInterface(*o);
    }
    return o.get();
}

as_value
securitypanel_ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new SecurityPanel_as;

    return as_value(obj.get()); // will keep alive
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

