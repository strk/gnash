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

#include <unistd.h>
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

using namespace std;
using namespace amf;

// http://www.osflash.org/localconnection
//
// Listening
// To create a listening LocalConnection, you just have to set a thread to:

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
namespace {
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();    
}

namespace gnash {

// The maximum 
const int LC_HEADER_SIZE = 16;
const int MAX_LC_HEADER_SIZE = 40960;
const int LC_LISTENERS_START  = MAX_LC_HEADER_SIZE +  LC_HEADER_SIZE;

// This doesn't exist on all systems, but here's the vaue used on Unix.
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 64
#endif

// \class LocalConnection
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
LocalConnection::LocalConnection()
{
    GNASH_REPORT_FUNCTION;
}

LocalConnection::~LocalConnection()
{
    GNASH_REPORT_FUNCTION;
}

Listener::Listener()
    : _baseaddr(0)
{
    GNASH_REPORT_FUNCTION;
}

Listener::Listener(boost::uint8_t *x)
{
    GNASH_REPORT_FUNCTION;
    _baseaddr = x;
}

Listener::~Listener()
{
    GNASH_REPORT_FUNCTION;
}

bool
Listener::addListener(std::string &name)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
    boost::uint8_t *item = addr;
    // Walk to the end of the list
    while (*item != 0) {
        item += strlen(reinterpret_cast<char *>(item))+1;
    }
    // Add ourselves to the list
    if (memcpy(item, name.c_str(), name.size()) == 0) {
        return false;
    }
    return true;
}

bool
Listener::removeListener(std::string &name)
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;
    boost::uint8_t *item = addr;
    while (*item != 0) {
        if (name.c_str() == reinterpret_cast<char *>(item)) {
            int len = strlen(reinterpret_cast<char *>(item)) + 1;
            while (*item != 0) {
                memcpy(item, item + len, len);
                item += len + 1;
            }
            return true;
        }
        item += strlen(reinterpret_cast<char *>(item)) + 1;
    }
    return false;
}

std::vector<std::string> *
Listener::listListeners()
{
    GNASH_REPORT_FUNCTION;

    boost::uint8_t *addr = _baseaddr + LC_LISTENERS_START;

    vector<string> *listeners = new vector<string>;
    const char *item = reinterpret_cast<char *>(addr);
    while (*item != 0) {
        listeners->push_back(item);
        item += strlen(item) + 1;
    }    

    return listeners;
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
LocalConnection::connect(const std::string& name)
{
    GNASH_REPORT_FUNCTION;
    
    _name = name;

    log_debug("trying to open shared memory segment: \"%s\"", name.c_str());
    
    if (Shm::attach(name.c_str(), true) == false) {
        return false;
    }

    if (Shm::getAddr() <= 0) {
        log_error("Failed to open shared memory segment: \"%s\"", name.c_str());
        return false; 
    }
    
#if 0 // { // what are these Listeners ?

    Listener::setBaseAddress(Shm::getAddr());
        
    string str1 = "HelloWorld";
    addListener(str1);
    str1 = "GutenTag";
    addListener(str1);
    str1 = "Aloha";
    addListener(str1);
    
    vector<string>::const_iterator it;
    vector<string> *listeners = listListeners();
    if (listeners->size() == 0) {
        log_msg("Nobody is listening");
    } else {
        log_msg("There are %d", listeners->size());
        for (it=listeners->begin(); it!=listeners->end(); it++) {
            string str = *it;
            log_debug("Listeners: %s", str.c_str());
        }
    }

    delete listeners;
    
    str1 = "HelloWorld";
    removeListener(str1);
    listeners = listListeners();  
    log_msg("There are %d", listeners->size());
    for (it=listeners->begin(); it != listeners->end(); it++) {
        string str = *it;
        log_debug("Listeners: %s", str.c_str());
    }
    
    delete listeners;
#endif // }
    
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
    
    URL url(getVM().getSWFUrl());
//    log_debug(_("ORIG URL=%s (%s)"), url.str().c_str(), url.hostname().c_str());
    if (url.hostname().empty()) {
        _name = "localhost";
    } else {
        _name = url.hostname();
    }

    // Adjust the name based on the swf version. Prior to v7, the nodename part
    // was removed. For v7 or later. the full hostname is returned. The localhost
    // is always just the localhost.
    if (version <= 6) {
        string::size_type pos;
        pos = _name.rfind(".", _name.size());
        if (pos != string::npos) {
            pos = _name.rfind(".", pos-1);
            if (pos != string::npos) {
                _name = _name.substr(pos+1, _name.size());
            }
        }
    }

    // If unset, pick the default. Yes, we're paranoid.
    if (_name.empty()) {
        _name =  "localhost";
    }
    
    log_msg("The domain for this host is: %s", _name.c_str());

    return _name;
}

/// \brief Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& /* fn */)
{
//    GNASH_REPORT_FUNCTION;
    LocalConnection *localconnection_obj = new LocalConnection;

    localconnection_obj->init_member("close", new builtin_function(localconnection_close));
    localconnection_obj->init_member("connect", new builtin_function(localconnection_connect));
    localconnection_obj->init_member("domain", new builtin_function(localconnection_domain));
    localconnection_obj->init_member("send", new builtin_function(localconnection_send));

    return as_value(localconnection_obj);
}

/// \brief The callback for LocalConnection::close()
as_value
localconnection_close(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);
    
    ptr->close();
    return as_value();
}

/// \brief The callback for LocalConnection::connect()
as_value
localconnection_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    bool ret;
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);
    string name = fn.arg(0).to_string().c_str();
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

/// \brief The callback for LocalConnection::domain()
as_value
localconnection_domain(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    boost::intrusive_ptr<LocalConnection> ptr = ensureType<LocalConnection>(fn.this_ptr);
    VM& vm = ptr->getVM();
    int swfVersion = vm.getSWFVersion();

    return as_value(ptr->domain(swfVersion).c_str());
}

// \brief The callback for LocalConnection::send()
as_value
localconnection_send(const fn_call& fn)
{
    boost::intrusive_ptr<LocalConnection> obj = ensureType<LocalConnection>(fn.this_ptr);
    if (rcfile.getLocalConnection() ) {
        log_security("Attempting to write to disabled LocalConnection!");
        return as_value(false);
    }
    
    return as_value();
}

} // end of gnash namespace
