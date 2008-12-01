// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "VM.h"
#include "URLAccessManager.h"
#include "URL.h"
#include "log.h"
#include "LocalConnection.h"
#include "network.h"
#include "fn_call.h"
#include "builtin_function.h"
#include "amf.h"
#include "lcshm.h"
#include "Object.h" // for getObjectInterface
#include "namedStrings.h"

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


// Anonymous namespace for module-statics
namespace {

/// Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& /* fn */)
{
    GNASH_REPORT_FUNCTION;
    LocalConnection *localconnection_obj = new LocalConnection;

    return as_value(localconnection_obj);
}

/// The callback for LocalConnection::close()
as_value
localconnection_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);
    
    ptr->close();
    return as_value();
}

/// The callback for LocalConnection::connect()
as_value
localconnection_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
//    log_debug("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);

    std::string name = fn.arg(0).to_string();
    bool ret;

    if (fn.nargs != 0) {
        ret = ptr->connect(name);
        name = "localhost";
    } else {
        log_error(_("No connection name specified to LocalConnection.connect()"));
        ret = ptr->connect(name); // FIXME: This should probably
                                       // fail instead
    }
    return as_value(ret);
}

/// The callback for LocalConnection::domain()
as_value
localconnection_domain(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);

    VM& vm = ptr->getVM();
    const int swfVersion = vm.getSWFVersion();

    return as_value(ptr->domain(swfVersion));
}

/// The callback for LocalConnection::send()
as_value
localconnection_send(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);

    std::ostringstream os;
    fn.dump_args(os);

    // It is useful to see what's supposed being sent, so we log this every time.
    log_unimpl(_("LocalConnection.send unimplemented %s"), os.str());

    if (!ptr->connected()) {
        ptr->connect();
    }
    
    if (rcfile.getLocalConnection() ) {
        log_security("Attempting to write to disabled LocalConnection!");
        return as_value(false);
    }

    // FIXME: send something
    return as_value();
}


void
attachLocalConnectionInterface(as_object& o)
{
    GNASH_REPORT_FUNCTION;

    o.init_member("close", new builtin_function(localconnection_close));
    o.init_member("connect", new builtin_function(localconnection_connect));
    o.init_member("domain", new builtin_function(localconnection_domain));
    o.init_member("send", new builtin_function(localconnection_send));
}

as_object*
getLocalConnectionInterface()
{
    GNASH_REPORT_FUNCTION;

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
		cl=new builtin_function(&localconnection_new, getLocalConnectionInterface());
        //attachLocalConnectionStaticInterface(*cl);
		VM::get().addStatic(cl); // FIXME: why do we need to register ourself here ?
	}

	return cl;
}

} // anonymous namespace

// This doesn't exist on all systems, but here's the vaue used on Unix.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

// \class LocalConnection
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
LocalConnection::LocalConnection()
    :
    as_object(getLocalConnectionInterface()),
    _connected(false)
{
    GNASH_REPORT_FUNCTION;
}

LocalConnection::~LocalConnection()
{
    GNASH_REPORT_FUNCTION;
}

/// \brief Closes (disconnects) the LocalConnection object.
void
LocalConnection::close()
{
    GNASH_REPORT_FUNCTION;
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
bool
LocalConnection::connect()
{
    return connect("");
}

bool
LocalConnection::connect(const std::string& name)
{
    GNASH_REPORT_FUNCTION;

    if (name.empty()) {
        _name = "none, sysv segment type";
    } else {
        _name = name;
    }
    
    log_debug("trying to open shared memory segment: \"%s\"", _name);
    
    if (Shm::attach(_name.c_str(), true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: \"%s\"", _name);
        return false; 
    }
    
    _connected = true;
    
    return true;
}

/// \brief Returns a string representing the superdomain of the
/// location of the current SWF file.
///
/// The domain is either the "localhost", or the hostname from the
/// network connection. This behaviour changed for SWF v7. Prior to v7
/// only the domain was returned, ie dropping off node names like
/// "www". As of v7, the behaviour is to return the full host
/// name. Gnash supports both behaviours based on the version.
std::string
LocalConnection::domain(int version)
{
//    GNASH_REPORT_FUNCTION;
    // We already figured out the name
    if (_name.size()) {
        return _name;
    }
    
    URL url(_vm.getRoot().getOriginalURL());
//    log_debug(_("ORIG URL=%s (%s)"), url.str(), url.hostname());
    if (url.hostname().empty()) {
        _name = "localhost";
    } else {
        _name = url.hostname();
    }

    // Adjust the name based on the swf version. Prior to v7, the nodename part
    // was removed. For v7 or later. the full hostname is returned. The localhost
    // is always just the localhost.
    if (version <= 6) {
        std::string::size_type pos;
        pos = _name.rfind(".", _name.size());
        if (pos != std::string::npos) {
            pos = _name.rfind(".", pos-1);
            if (pos != std::string::npos) {
                _name = _name.substr(pos+1, _name.size());
            }
        }
    }

    // If unset, pick the default. Yes, we're paranoid.
    if (_name.empty()) {
        _name =  "localhost";
    }
    
    log_debug("The domain for this host is: %s", _name);

    return _name;
}

void
localconnection_class_init(as_object& glob)
{
	builtin_function* ctor=getLocalConnectionConstructor();

	int swf6flags = as_prop_flags::dontEnum|as_prop_flags::dontDelete|as_prop_flags::onlySWF6Up;
    glob.init_member(NSV::CLASS_LOCAL_CONNECTION, ctor, swf6flags);
}

} // end of gnash namespace
