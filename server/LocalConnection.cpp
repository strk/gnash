// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>

#include "log.h"
#include "LocalConnection.h"
#include "network.h"
#include "fn_call.h"

namespace gnash {

// \class LocalConnection
/// \brief Open a connection between two SWF movies so they can send
/// each other Flash Objects to be executed.
///
LocalConnection::LocalConnection() {
}

LocalConnection::~LocalConnection() {
}

/// \brief Closes (disconnects) the LocalConnection object.
void
LocalConnection::close()
{
#ifdef NETWORK_CONN
    closeNet();
#else
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
LocalConnection::connect(const char *name)
{
#ifdef NETWORK_CONN
    short lastport;
    const char *lcname;

    std::map<const char *, short>::const_iterator it;
    for (it = _allocated.begin(); it != _allocated.end(); it++) {
        lcname = it->first;
        lastport  = it->second;
        if (strcmp(name, lcname) == 0) {
            log_msg("ERROR: %s already allocated!\n", name);
            return false;
        }
    }

    // Allocate the tcp/ip port adfter the last allocated one.
    if (lastport != 0) {
        _allocated[name] = lastport+1;
    }

    // Create the socket
    if (createServer(lastport+1)) {
        log_msg("New server started for \"%s\" connections.\n", name);
    } else {
        log_msg("ERROR: Couldn't create a new server for \"%s\"!\n");
        return false;
    }

    if (newConnection(false)) {
        log_msg("New connection started for \"%s\" connections.\n", name);
//        writeNet(heloCreate(_version));
        return true;
    } else {
        if (errno == EAGAIN) {
            log_msg("No clients tried to connect within the allocated time limit\n");
            return false;
        }
        else {
            log_msg("ERROR: Couldn't create a new connection!\n");
        }
            
        return false;
    }
#else
    if (attach(name, true) == false) {
        return false;
    }
#endif
    _name = name;

    return true;
}

/// \brief Returns a string representing the superdomain of the
/// location of the current SWF file.
///
/// The domain is either the "localhost", or the hostname from the
/// network connection. This behaviour changed for SWF v7. Prior to v7
/// only the domain was returned, ie dropping off node names like
/// "www". As of v7, the behaviour is to return the full host
/// name. Gnash defaults to the v7 behaviour.
/// \note If this becomes a problem, we'll have to implemented the
/// older behaviour based on the version of the flash movie being
/// played.
std::string
LocalConnection::domain(void)
{
    if (_name.size() == 0) {
        return "localhost";
    } else {
        return _name;
    }
}

/// \brief Invokes a method on a specified LocalConnection object.
void
LocalConnection::send()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

/// \brief Instantiate a new LocalConnection object within a flash movie
void
localconnection_new(const fn_call& fn)
{
    localconnection_as_object *localconnection_obj = new localconnection_as_object;

    localconnection_obj->set_member("close", &localconnection_close);
    localconnection_obj->set_member("connect", &localconnection_connect);
    localconnection_obj->set_member("domain", &localconnection_domain);
    localconnection_obj->set_member("send", &localconnection_send);
#ifdef ENABLE_TESTING
#ifdef NETWORK_CONN
    localconnection_obj->set_member("connected",  &network_connected);
    localconnection_obj->set_member("getfilefd",  &network_getfilefd);
    localconnection_obj->set_member("getlistenfd",  &network_getlistenfd);
#else
    localconnection_obj->set_member("getname",  &shm_getname);
    localconnection_obj->set_member("getsize",  &shm_getsize);
    localconnection_obj->set_member("getallocated",  &shm_getallocated);
    localconnection_obj->set_member("exists",  &shm_exists);
#endif
#endif

    fn.result->set_as_object(localconnection_obj);
}

/// \brief The callback for LocalConnection::close()
void localconnection_close(const fn_call& fn)
{
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    
    localconnection_as_object *ptr = (localconnection_as_object*)fn.this_ptr;
    assert(ptr);
    
    ptr->obj.close();
}

/// \brief The callback for LocalConnection::connect()
void localconnection_connect(const fn_call& fn)
{
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    bool ret;
    localconnection_as_object *ptr = (localconnection_as_object*)fn.this_ptr;
    
    assert(ptr);
    if (fn.nargs != 0) {
        ret = ptr->obj.connect(fn.env->bottom(fn.first_arg_bottom_index).to_string());
    } else {
        log_msg("ERROR: No connection name specified to LocalConnection.connect()!\n");
        ret = ptr->obj.connect("localhost"); // FIXME: This should probably
                                       // fail instead
    }
    fn.result->set_bool(ret);
}

/// \brief The callback for LocalConnection::domain()
void localconnection_domain(const fn_call& fn)
{
//    log_msg("%s:\n", __PRETTY_FUNCTION__);
    localconnection_as_object *ptr = (localconnection_as_object*)fn.this_ptr;
    assert(ptr);
    fn.result->set_tu_string(ptr->obj.domain().c_str());
}

// \brief The callback for LocalConnection::send()
void localconnection_send(const fn_call& fn)
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

