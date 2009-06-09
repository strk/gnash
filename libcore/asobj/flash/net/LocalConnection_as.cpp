// LocalConnection_as.cpp:  ActionScript "LocalConnection" class, for Gnash.
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

#include "net/LocalConnection_as.h"
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
//moved to header file
//#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException

//Si 
///Added
#include <cerrno>
#include <cstring>
#include <boost/cstdint.hpp> // for boost::?int??_t
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

//#include "VM.h"
#include "movie_root.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "network.h"
#include "builtin_function.h"
#include "amf.h"
#include "lcshm.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "StringPredicates.h"

using namespace amf;

//Added end

namespace gnash {

// Forward declarations

void attachLocalConnectionInterface(as_object& o);
//si
//moved to header file
//extern as_object* getLocalConnectionInterface();


//namespace {
    as_value localconnection_allowInsecureDomain(const fn_call& fn);

    as_value localconnection_allowDomain(const fn_call& fn);

//Si
//FIX ME
//allowDomain has not been defined !!!!!!!!!!!!!!!!!!!!
	
    as_value localconnection_close(const fn_call& fn);
    as_value localconnection_connect(const fn_call& fn);
    as_value localconnection_send(const fn_call& fn);
    as_value localconnection_asyncError(const fn_call& fn);
    as_value localconnection_securityError(const fn_call& fn);
    as_value localconnection_status(const fn_call& fn);

//	Si
//   moved to header file
//    as_value localconnection_ctor(const fn_call& fn);
    
    void attachLocalConnectionStaticInterface(as_object& o);
//    as_object* getLocalConnectionInterface();

//Si 
//Added
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance(); 
    as_value localconnection_domain(const fn_call& fn);

    bool validFunctionName(const std::string& func);

    builtin_function* getLocalConnectionConstructor();
//Added end


//}

//Si
//Moved into head file
//class LocalConnection_as : public as_object
//{
//public:
//    LocalConnection_as()
//        :
//        as_object(getLocalConnectionInterface())
//    {}
//};

// extern (used by Global.cpp)

LocalConnection_as::LocalConnection_as()
    :
    as_object(getLocalConnectionInterface()),
    _connected(false),
    _domain(getDomain())
{
    log_debug("The domain for this host is: %s", _domain);
}

LocalConnection_as::~LocalConnection_as()
{
}

/// \brief Closes (disconnects) the LocalConnection object.
void
LocalConnection_as::close()
{
    _connected = false;
#ifndef NETWORK_CONN
    closeMem();
#endif
}


void LocalConnection_as::init(as_object& global)
{
    static boost::intrusive_ptr<builtin_function> cl;

    if (!cl) {
        cl = new builtin_function(&localconnection_ctor, getLocalConnectionInterface());
        attachLocalConnectionStaticInterface(*cl);
    }

    // Register _global.LocalConnection
    global.init_member("LocalConnection", cl.get());
}


//namespace {

void
attachLocalConnectionInterface(as_object& o)
{
    o.init_member("allowInsecureDomain", new builtin_function(localconnection_allowInsecureDomain));
    o.init_member("allowDomain", new builtin_function(localconnection_allowDomain));	
    o.init_member("close", new builtin_function(localconnection_close));
    o.init_member("connect", new builtin_function(localconnection_connect));
    o.init_member("send", new builtin_function(localconnection_send));
    o.init_member("asyncError", new builtin_function(localconnection_asyncError));
    o.init_member("securityError", new builtin_function(localconnection_securityError));
    o.init_member("status", new builtin_function(localconnection_status));
    o.init_member("domain", new builtin_function(localconnection_domain));
}

void
attachLocalConnectionStaticInterface(as_object& )
{
}

as_object*
getLocalConnectionInterface()
{
    static boost::intrusive_ptr<as_object> o;
    if ( ! o ) {
        o = new as_object();
        attachLocalConnectionInterface(*o);
    }
    return o.get();
}

as_value
localconnection_allowInsecureDomain(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}



//Si
//FIX ME!
//This function has not been completed.
//The definition here is only used for test!!!!!!!!!!!!!!!!!!!!!!!!!!!

as_value
localconnection_allowDomain(const fn_call& fn)
{
    
    return as_value();
}


as_value
localconnection_close(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_connect(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_send(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_asyncError(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_securityError(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_status(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    UNUSED(ptr);
    log_unimpl (__FUNCTION__);
    return as_value();
}

as_value
localconnection_ctor(const fn_call&)
{
    boost::intrusive_ptr<as_object> obj = new LocalConnection_as;

    return as_value(obj.get()); // will keep alive
}

//Si
//Added


/// Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& /* fn */)
{
    LocalConnection_as *obj = new LocalConnection_as;

    return as_value(obj);
}

/// The callback for LocalConnection::domain()
as_value
localconnection_domain(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);

    return as_value(ptr->domain());
}


builtin_function*
getLocalConnectionConstructor()
{
	// This is going to be the global Number "class"/"function"
	static builtin_function* cl=NULL;

	if ( cl == NULL )
	{
		cl = new builtin_function(&localconnection_new,getLocalConnectionInterface());

        // FIXME: why do we need to register ourself here ?
		VM::get().addStatic(cl);
	}

	return cl;
}

/// These names are invalid as a function name.
bool
validFunctionName(const std::string& func)
{

    if (func.empty()) return false;

    typedef std::vector<std::string> ReservedNames;

    static const ReservedNames reserved = boost::assign::list_of
        ("send")
        ("onStatus")
        ("close")
        ("connect")
        ("domain")
	("allowDomain")
        ("allowInsecureDomain");

    const ReservedNames::const_iterator it =
        std::find_if(reserved.begin(), reserved.end(),
                boost::bind(StringNoCaseEqual(), _1, func));
        
    return (it == reserved.end());
}

/// \brief Returns a string representing the superdomain of the
/// location of the current SWF file.
//
/// This is set on construction, as it should be constant.
/// The domain is either the "localhost", or the hostname from the
/// network connection. This behaviour changed for SWF v7. Prior to v7
/// only the domain was returned, ie dropping off node names like
/// "www". As of v7, the behaviour is to return the full host
/// name. Gnash supports both behaviours based on the version.
std::string
LocalConnection_as::getDomain()
{
    
    URL url(_vm.getRoot().getOriginalURL());

    if (url.hostname().empty()) {
        return "localhost";
    }

    // Adjust the name based on the swf version. Prior to v7, the nodename part
    // was removed. For v7 or later. the full hostname is returned. The
    // localhost is always just the localhost.
    if (_vm.getSWFVersion() > 6) {
        return url.hostname();
    }

    const std::string& domain = url.hostname();

    std::string::size_type pos;
    pos = domain.rfind('.');

    // If there is no '.', return the whole thing.
    if (pos == std::string::npos) {
        return domain;
    }

    pos = domain.rfind(".", pos - 1);
    
    // If there is no second '.', return the whole thing.
    if (pos == std::string::npos) {
        return domain;
    }
    // Return everything after the second-to-last '.'
    // FIXME: this must be wrong, or it would return 'org.uk' for many
    // UK websites, and not even Adobe is that stupid. I think.
    return domain.substr(pos + 1);

}


//} // anonymous namespace 
} // gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

