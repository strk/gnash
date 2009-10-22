// Security_as.cpp:  ActionScript "Security" class, for Gnash.
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

#include "system/Security_as.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

namespace gnash {

// Forward declarations
namespace {
    as_value security_allowInsecureDomain(const fn_call& fn);
    as_value security_loadPolicyFile(const fn_call& fn);
    as_value security_showSettings(const fn_call& fn);
    as_value security_LOCAL_TRUSTED(const fn_call& fn);
    as_value security_LOCAL_WITH_FILE(const fn_call& fn);
    as_value security_LOCAL_WITH_NETWORK(const fn_call& fn);
    as_value security_REMOTE(const fn_call& fn);
    as_value security_ctor(const fn_call& fn);
    void attachSecurityInterface(as_object& o);
    void attachSecurityStaticInterface(as_object& o);
    as_object* getSecurityInterface();

}

// extern (used by Global.cpp)
void
security_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as* gl = getGlobal(where);
    as_object* proto = gl->createObject();
    as_object* cl = gl->createClass(&security_ctor, proto);
    attachSecurityStaticInterface(*cl);
    attachSecurityInterface(*proto);

    // Register _global.Security
    where.init_member(getName(uri), cl, as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachSecurityInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("allowInsecureDomain", gl->createFunction(security_allowInsecureDomain));
    o.init_member("loadPolicyFile", gl->createFunction(security_loadPolicyFile));
    o.init_member("showSettings", gl->createFunction(security_showSettings));
    o.init_member("LOCAL_TRUSTED", gl->createFunction(security_LOCAL_TRUSTED));
    o.init_member("LOCAL_WITH_FILE", gl->createFunction(security_LOCAL_WITH_FILE));
    o.init_member("LOCAL_WITH_NETWORK", gl->createFunction(security_LOCAL_WITH_NETWORK));
    o.init_member("REMOTE", gl->createFunction(security_REMOTE));
}

void
attachSecurityStaticInterface(as_object& /*o*/)
{
}

as_value
security_allowInsecureDomain(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_loadPolicyFile(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_showSettings(const fn_call&)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_LOCAL_TRUSTED(const fn_call&)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_LOCAL_WITH_FILE(const fn_call&)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_LOCAL_WITH_NETWORK(const fn_call&)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_REMOTE(const fn_call&)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
security_ctor(const fn_call& /*fn*/)
{
    return as_value(); 
}

} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

