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

#include <vector>
#include <string>
#include <deque>

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
    
    typedef enum {
	NONE,
	HTTP,
	RTMP,
	RTMPT,
	RTMPTS,
	RTMPE,
	RTMPS,
	DTN
    } protocols_supported_e;
    
     DSOEXPORT Handler();
    ~Handler();

    /// \var sync
    ///     Send the onSync message to all connected clients
    bool sync() { return sync(_in_fd); };
    bool sync(int in_fd);

    // Access the name field
    void setName(const std::string &x) { _name = x; };
    std::string &getName() { return _name; }

    void addSOL(boost::shared_ptr<amf::Element> x) {
	_sol.push_back(x);
    };

    /// \method addClient
    ///     Add a client to the list for output messages.
    size_t addClient(int x, protocols_supported_e proto);
    /// \method removeClient
    ///     Remove a client from the list for messages.
    void removeClient(int x);
    /// \var getClients
    ///     Get the vector of file descriptors for this handler.
    std::vector<int> &getClients() { return _clients; };

    protocols_supported_e getProtocol(int x) { return _protocol[x]; };
    
    /// \method addRemote
    ///     Add a remote machine to the list for input messages.
    size_t addRemote(int x) { _remote.push_back(x); return _remote.size(); };
    
    // Dump internal data.
    void dump();    

protected:
    /// \var _name
    ///	    The name of the path this handler is supporting.
    std::string				_name;
    ///	    Each incoming request has one of 4 states the server has
    ///     to handle to send a response.

    /// \var _protocol
    ///    this is the map of which protocol is being used by which
    ///    file descriptor.
    std::map<int, protocols_supported_e> _protocol;
    /// \var _clients
    ///	    is the array of all clients connected to this server for
    ///     this application. This is where all the output goes.
    std::vector<int>			_clients;
    /// \var _remote
    ///	    This is network connections to other processes,
    ///	    on other computers.
    std::vector<int>			_remote;

    /// \var _local
    ///    These are local process we're responsible for
    ///    starting and stopping.
    boost::shared_ptr<cygnal::Proc>	_local;
    /// \var _plugins
    ///	    is for the dynamically loaded applications
    boost::shared_ptr<gnash::SharedLib> _plugin;
    /// \var _file
    ///	    is for disk based files
    std::vector<boost::shared_ptr<gnash::DiskStream> > _file;
    /// \var _sol
    ///	    is for remote SharedObjects
    std::vector<boost::shared_ptr<amf::Element> > _sol;
    /// \var _in_fd
    ///	    The file descriptor of the incoming data for an
    ///     Invoke message.
    int _in_fd;

private:    
    boost::mutex _mutex;
    
// Remote Shared Objects. References are an index into this vector.
//    std::map<std::string, boost::shared_ptr<handler_t> > _handlers;
};

} // end of gnash namespace

#endif // end of __HANDLER_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
