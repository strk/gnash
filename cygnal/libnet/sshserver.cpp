// ssh.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
// 
//   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#include <boost/thread/mutex.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <boost/cstdint.hpp>
#include <boost/array.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <cstdlib> // getenv

#include "GnashSystemIOHeaders.h" // read()
#include "sshserver.h"
#include "amf.h"
#include "element.h"
#include "cque.h"
#include "log.h"
#include "network.h"
#include "utility.h"
#include "buffer.h"
#include "diskstream.h"
#include "cache.h"

extern "C" {
# include <libssh/libssh.h>
# include <libssh/sftp.h>
}
#include "sshclient.h"
#include "sshserver.h"

#if defined(_WIN32) || defined(WIN32)
# define __PRETTY_FUNCTION__ __FUNCDNAME__
# include <winsock2.h>
# include <direct.h>
#else
# include <unistd.h>
# include <sys/param.h>
#endif

using namespace gnash;
using namespace std;

static boost::mutex stl_mutex;

namespace gnash
{

const char *PASSWORD  = "none";

SSHServer::SSHServer()
{
//     GNASH_REPORT_FUNCTION;

    // Set the default user name
    setUser();
}

SSHServer::~SSHServer()
{
//    GNASH_REPORT_FUNCTION;
    
    sshShutdown();
}

// Authenticate the password from the user
bool
SSHServer::authPassword(string & /* user */, string & /* passwd */)
{
    return false;
}

bool
SSHServer::authPassword(ssh_session /* session */, string &/* user */, string & /* passwd */)
{
    return false;
}

// Wait for an incoming network connection
bool
SSHServer::acceptConnections()
{
    return false;
}

bool
SSHServer::acceptConnections(short /* port */)
{
    return false;
}

bool
SSHServer::acceptConnections(ssh_session /* session */)
{
    return false;
}

bool
SSHServer::acceptConnections(ssh_session /* session */, short /* port */)
{
    return false;
}

// Parse an SSH command message and do something
bool
SSHServer::processSSHMessage(ssh_message message)
{
    if (!message) {
	return false;
    }
    switch(ssh_message_type(message)){
    case SSH_REQUEST_AUTH:
	switch(ssh_message_subtype(message)) {
	    // not authenticated, send default message
 	case SSH_AUTH_METHOD_NONE:
 	    break;
	case SSH_AUTH_METHOD_PASSWORD:
	    {
		log_debug(_("User %s wants to auth with pass %s\n"),
			  ssh_message_auth_user(message),
			  ssh_message_auth_password(message));
		string user = ssh_message_auth_user(message);
		string passwd = ssh_message_auth_password(message);
		if (authPassword(user, passwd)){
		    // 		auth=1;
		    ssh_message_auth_reply_success(message,0);
		    break;
		}
		break;
	    }
	case SSH_AUTH_METHOD_HOSTBASED:
	    break;
	case SSH_AUTH_METHOD_PUBLICKEY:
	    break;
	case SSH_AUTH_METHOD_INTERACTIVE:
	    break;
	case SSH_AUTH_METHOD_UNKNOWN:
	    break;
	default:
	    ssh_message_auth_set_methods(message,SSH_AUTH_METHOD_PASSWORD);
	    ssh_message_reply_default(message);
	    break;
	}
    case SSH_REQUEST_CHANNEL_OPEN:
	if(ssh_message_subtype(message)==SSH_CHANNEL_SESSION){
	    _channel = ssh_message_channel_request_open_reply_accept(message);
	    break;
	}
	break;
//     case SSH_CHANNEL_REQUEST_EXEC:
// 	break;
    case SSH_CHANNEL_REQUEST_ENV:
	break;
    case SSH_CHANNEL_REQUEST_SUBSYSTEM:
	break;
    case SSH_CHANNEL_REQUEST_WINDOW_CHANGE:
	break;
    case SSH_CHANNEL_REQUEST_UNKNOWN:
	break;
    default:
	ssh_message_reply_default(message);
    }
    ssh_message_free(message);

    return false;
}

void
SSHServer::dump() {
//    GNASH_REPORT_FUNCTION;
    
    boost::mutex::scoped_lock lock(stl_mutex);
  
    log_debug (_("==== The SSH header breaks down as follows: ===="));

    ssh_version(0);
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
