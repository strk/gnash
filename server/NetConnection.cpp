// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "NetConnection.h"

namespace gnash {

NetConnection::NetConnection() {
}

NetConnection::~NetConnection() {
}


void
NetConnection::connect()
{
    log_msg("%s:unimplemented \n", __FUNCTION__);
}
void
netconnection_new(const fn_call& fn)
{
    netconnection_as_object *netconnection_obj = new netconnection_as_object;

    netconnection_obj->set_member("connect", &netconnection_connect);

    fn.result->set_as_object_interface(netconnection_obj);
}
void netconnection_connect(const fn_call& fn) {
    log_msg("%s:unimplemented \n", __FUNCTION__);
}

} // end of gnaash namespace

