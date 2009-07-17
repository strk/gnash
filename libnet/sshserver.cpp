// ssh.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
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
SSHServer::authPassword(string &user, string &passwd)
{
}

bool
SSHServer::authPassword(SSH_SESSION *session, string &user, string &passwd)
{
}

// Wait for an incoming network connection
bool
SSHServer::acceptConnections()
{
}

bool
SSHServer::acceptConnections(short port)
{
}

bool
SSHServer::acceptConnections(SSH_SESSION *session)
{
}

bool
SSHServer::acceptConnections(SSH_SESSION *session, short port)
{
}

// Parse an SSH command message and do something
bool
SSHServer::processSSHMessage(SSH_MESSAGE *message)
{
    if (!message) {
	return false;
    }
    switch(ssh_message_type(message)){
    case SSH_AUTH_REQUEST:
	switch(ssh_message_subtype(message)) {
	    // not authenticated, send default message
 	case SSH_AUTH_NONE:
 	    break;
	case SSH_AUTH_PASSWORD:
	    {
		log_debug("User %s wants to auth with pass %s\n",
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
	case SSH_AUTH_HOSTBASED:
	    break;
	case SSH_AUTH_PUBLICKEY:
	    break;
	case SSH_AUTH_KEYBINT:
	    break;
	case SSH_AUTH_UNKNOWN:
	    break;
	default:
	    ssh_message_auth_set_methods(message,SSH_AUTH_PASSWORD);
	    ssh_message_reply_default(message);
	    break;
	}
    case SSH_CHANNEL_REQUEST_OPEN:
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
// indent-tabs-mode: t
// End:
