// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "GnashSystemIOHeaders.h"
#include <cerrno>
#include <cstring>
#include <boost/cstdint.hpp> // for boost::?int??_t
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

#include "VM.h"
#include "movie_root.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "log.h"
#include "LocalConnection_as.h"
#include "network.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "amf.h"
#include "lcshm.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"
#include "StringPredicates.h"

using namespace amf;

// http://www.osflash.org/localconnection
//
// Listening
// To create a listening LocalConnection, you just have to set a thread to:
//
//    1.register the application as a valid LocalConnection listener,
//    2.require the mutex to have exclusive access to the shared memory,
//    3.access the shared memory and check the recipient,
//    4.if you are the recipient, read the message and mark it read,
//    5.release the shared memory and the mutex,
//    6.repeat indefinitely from step 2.
//
// Sending
// To send a message to a LocalConnection apparently works like that:
//    1. require the mutex to have exclusive access to the shared memory,
//    2. access the shared memory and check that the listener is connected,
//    3. if the recipient is registered, write the message,
//    4. release the shared memory and the mutex.
//
// The main thing you have to care about is the timestamp - simply call GetTickCount()
//  and the message size. If your message is correctly encoded, it should be received
// by the listening LocalConnection 
//
// Some facts:
//     * The header is 16 bytes,
//     * The message can be up to 40k,
//     * The listeners block starts at 40k+16 = 40976 bytes,
//     * To add a listener, simply append its name in the listeners list (null terminated strings)
//
namespace {

gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();    

} // anonymous namespace

namespace gnash {

namespace {
    as_value localconnection_connect(const fn_call& fn);
    as_value localconnection_domain(const fn_call& fn);

    bool validFunctionName(const std::string& func);

    builtin_function* getLocalConnectionConstructor();
    as_object* getLocalConnectionInterface();
}

// \class LocalConnection_as
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
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

/// \brief Prepares the LocalConnection object to receive commands from a
/// LocalConnection.send() command.
/// 
/// The name is a symbolic name like "lc_name", that is used by the
/// send() command to signify which local connection to send the
/// object to.
void
LocalConnection_as::connect(const std::string& name)
{

    assert(!name.empty());

    _name = name;
    
    // TODO: does this depend on success?
    _connected = true;
    
    log_debug("trying to open shared memory segment: \"%s\"", _name);
    
    if (Shm::attach(_name.c_str(), true) == false) {
        return;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: \"%s\"", _name);
        return; 
    }
    
    return;
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

void
LocalConnection_as::init(as_object& glob)
{
	builtin_function* ctor=getLocalConnectionConstructor();

	int swf6flags = as_prop_flags::dontEnum | 
                    as_prop_flags::dontDelete | 
                    as_prop_flags::onlySWF6Up;

    glob.init_member(NSV::CLASS_LOCALCONNECTION, ctor, swf6flags);
}


// Anonymous namespace for module-statics
namespace {

/// Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& /* fn */)
{
    LocalConnection_as *obj = new LocalConnection_as;

    return as_value(obj);
}

/// The callback for LocalConnection::close()
as_value
localconnection_close(const fn_call& fn)
{
    
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);
    
    ptr->close();
    return as_value();
}

/// The callback for LocalConnection::connect()
as_value
localconnection_connect(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);

    // If already connected, don't try again until close() is called.
    if (ptr->connected()) return false;

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("LocalConnection.connect() expects exactly "
                    "1 argument"));
        );
        return as_value(false);
    }

    if (!fn.arg(0).is_string()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror(_("LocalConnection.connect(): first argument must "
                    "be a string"));
        );
        return as_value(false);
    }

    std::string name = fn.arg(0).to_string();

    if (name.empty()) {
        return as_value(false);
    }

    ptr->connect(name);

    // We don't care whether connected or not.
    return as_value(true);
}

/// The callback for LocalConnection::domain()
as_value
localconnection_domain(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);

    return as_value(ptr->domain());
}

/// LocalConnection.send()
//
/// Returns false only if the call was syntactically incorrect.
as_value
localconnection_send(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection_as> ptr =
        ensureType<LocalConnection_as>(fn.this_ptr);

    // At least 2 args (connection name, function) required.
    if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
    }

    // Both the first two arguments must be a string
    if (!fn.arg(0).is_string() || !fn.arg(1).is_string()) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
    }

    const std::string& func = fn.arg(1).to_string();

    if (!validFunctionName(func)) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream os;
            fn.dump_args(os);
            log_aserror(_("LocalConnection.send(%s): requires at least 2 "
                    "arguments"), os.str());
        );
        return as_value(false);
    }


    // Now we have a valid call.

    // It is useful to see what's supposed being sent, so we log
    // this every time until implemented.
    std::ostringstream os;
    fn.dump_args(os);
    log_unimpl(_("LocalConnection.send unimplemented %s"), os.str());

    // We'll return true if the LocalConnection is disabled too, as
    // the return value doesn't indicate success of the connection.
    if (rcfile.getLocalConnection() ) {
        log_security("Attempting to write to disabled LocalConnection!");
        return as_value(true);
    }

    return as_value(true);
}


void
attachLocalConnectionInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    o.init_member("close", gl->createFunction(localconnection_close));
    o.init_member("connect", gl->createFunction(localconnection_connect));
    o.init_member("domain", gl->createFunction(localconnection_domain));
    o.init_member("send", gl->createFunction(localconnection_send));
}

as_object*
getLocalConnectionInterface()
{

    static boost::intrusive_ptr<as_object> o;

    if ( o == NULL )
    {
        o = new as_object(getObjectInterface());
	    VM::get().addStatic(o.get());

        attachLocalConnectionInterface(*o);
    }

    return o.get();
}

builtin_function*
getLocalConnectionConstructor()
{
	// This is going to be the global Number "class"/"function"
	static builtin_function* cl=NULL;

	if ( cl == NULL )
	{
		cl = new builtin_function(&localconnection_new,
                getLocalConnectionInterface());

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
        ("allowDomain");

    const ReservedNames::const_iterator it =
        std::find_if(reserved.begin(), reserved.end(),
                boost::bind(StringNoCaseEqual(), _1, func));
        
    return (it == reserved.end());
}

} // anonymous namespace

} // end of gnash namespace
