// LocalConnection.cpp:  Connect two SWF movies & send objects, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
#include "config.h"
#endif

#include <cerrno>

#include "log.h"
#include "LocalConnection.h"
#include "network.h"
#include "fn_call.h"
#include "builtin_function.h" 

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
LocalConnection::connect(const char *name)
{
#ifndef NETWORK_CONN
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
    log_unimpl (__FUNCTION__);
}

/// \brief Instantiate a new LocalConnection object within a flash movie
as_value
localconnection_new(const fn_call& /* fn */)
{
    localconnection_as_object *localconnection_obj = new localconnection_as_object;

    localconnection_obj->init_member("close", new builtin_function(localconnection_close));
    localconnection_obj->init_member("connect", new builtin_function(localconnection_connect));
    localconnection_obj->init_member("domain", new builtin_function(localconnection_domain));
    localconnection_obj->init_member("send", new builtin_function(localconnection_send));
#if 0
    // Apparently these AS methods were added for testing purposes. However,
    // they do not appear to be part of the LocalConnection AS object.
    localconnection_obj->init_member("getname",  new builtin_function(shm_getname));
    localconnection_obj->init_member("getsize",  new builtin_function(shm_getsize));
    localconnection_obj->init_member("getallocated",  new builtin_function(shm_getallocated));
    localconnection_obj->init_member("exists",  new builtin_function(shm_exists));
#endif

    return as_value(localconnection_obj);
}

/// \brief The callback for LocalConnection::close()
as_value localconnection_close(const fn_call& fn)
{
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    
    boost::intrusive_ptr<localconnection_as_object> ptr = ensureType<localconnection_as_object>(fn.this_ptr);
    
    ptr->obj.close();
    return as_value();
}

/// \brief The callback for LocalConnection::connect()
as_value localconnection_connect(const fn_call& fn)
{
//    log_msg("%s: %d args\n", __PRETTY_FUNCTION__, fn.nargs);
    bool ret;
    boost::intrusive_ptr<localconnection_as_object> ptr = ensureType<localconnection_as_object>(fn.this_ptr);
    
    if (fn.nargs != 0) {
        ret = ptr->obj.connect(fn.arg(0).to_string().c_str());
    } else {
        log_error(_("No connection name specified to LocalConnection.connect()"));
        ret = ptr->obj.connect("localhost"); // FIXME: This should probably
                                       // fail instead
    }
    return as_value(ret);
}

/// \brief The callback for LocalConnection::domain()
as_value localconnection_domain(const fn_call& fn)
{
//    log_msg("%s:\n", __PRETTY_FUNCTION__);
    boost::intrusive_ptr<localconnection_as_object> ptr = ensureType<localconnection_as_object>(fn.this_ptr);
    return as_value(ptr->obj.domain().c_str());
}

// \brief The callback for LocalConnection::send()
as_value localconnection_send(const fn_call& /*fn*/)
{
    log_unimpl (__FUNCTION__);
    return as_value();
}

} // end of gnash namespace
