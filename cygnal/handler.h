// 
//   Copyright (C) 2008, 2009 Free Software Foundation, Inc.
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

#ifndef __HANDLER_H__
#define __HANDLER_H__ 1

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/cstdint.hpp>
#include <boost/thread/mutex.hpp>
//#include <boost/thread/condition.hpp>
#include <string>
#include <deque>
#include <map>

#ifdef HAVE_POLL
# include <sys/poll.h>
#else 
# ifdef HAVE_EPOLL
#  include <sys/epoll.h>
# endif
#endif

#include "log.h"
#include "network.h"
#include "buffer.h"
#include "element.h"
#include "cque.h"
#include "network.h"
#include "dsodefs.h" //For DSOEXPORT.
#include "proc.h"

#include "diskstream.h"
#include "sharedlib.h"

// _definst_ is the default instance name
namespace cygnal
{

class Handler
{
public:
    /// \enum admin_cmd_e
    ///		The Admin command sent by the client
    typedef enum {
	UNKNOWN,
	STATUS,
	POLL,
	HELP,
	INTERVAL,
	QUIT,
    } admin_cmd_e;
    
     DSOEXPORT Handler();
    ~Handler();

    /// \var sync
    ///     Send the onSync message to all connectec cients
    bool sync() { return sync(_in_fd); };
    bool sync(int in_fd);
    
// Dump internal data.
    void dump();    

    size_t addFile(int x) { _clients.push_back(x); };
	
protected:
    ///	    Each incoming request has one of 4 states the server has to handle
    ///	    to send a response.
    
    /// \var _clients
    ///	is the array of all clients connected to this server for this
    ///	application. This is where all the output goes.
    std::vector<int>			_clients;
    /// \var _remote
    ///	connections are network connections to other processes,
    ///	possibly on another computer.
    boost::shared_ptr<cygnal::Proc>	_remote;
    /// \var _plugins
    ///	is for the dynamically loaded applications
    boost::shared_ptr<gnash::SharedLib>	_plugin;
    /// \var _file
    ///	is for disk based files
    std::vector<boost::shared_ptr<gnash::DiskStream> > _file;
    /// \var _sol
    ///	is for remote SharedObjects
    std::vector<boost::shared_ptr<amf::Element> > _sol;
    /// \var _in_fd
    ///	    The file descriptor of the incoming data
    int _in_fd;
    
// Remote Shared Objects. References are an index into this vector.
//    std::map<std::string, boost::shared_ptr<handler_t> > _handlers;
};

} // end of gnash namespace

#endif // end of __HANDLER_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
