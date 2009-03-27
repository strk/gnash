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


#ifndef GNASH_NETCONNECTION_H
#define GNASH_NETCONNECTION_H


#include <string>
#include <list>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "IOChannel.h"
#include "as_object.h" // for inheritance
#include "fn_call.h"
#include "VM.h"

// Internal headers from libnet
#include "network.h"
#include "http.h"		// RTMPT & HTTP client side support
#include "rtmp_client.h"	// RTMP client side support


namespace gnash {

class NetConnection_as;

/// NetConnection ActionScript class
//
/// Provides interfaces to load data from an URL
///
class NetConnection_as: public as_object
{
public:

   // This is used to pass parameters to a thread using boost::bind
    typedef struct {
	as_object        *callback;
	Network          *network;
	VM		 *vm;
	string_table     *st;
 	NetConnection_as *nas;
    } thread_params_t;

    enum StatusCode
    {
        CONNECT_FAILED,
        CONNECT_SUCCESS,
        CONNECT_CLOSED,
        CONNECT_REJECTED,
        CONNECT_APPSHUTDOWN,
        CALL_FAILED,
        CALL_BADVERSION
    };

	NetConnection_as();
	~NetConnection_as();

    /// Make the stored URI into a valid and checked URL.
    std::string validateURL() const;

    void call(as_object* asCallback, const std::string& methodName,
              const std::vector<as_value>& args, size_t firstArg);

    /// Process the close() method.
    void close();

    /// Process the connect(uri) method.
    void connect(const std::string& uri);

    /// Carry out the connect(null) method.
    void connect();

    bool isConnected() const {
        return _isConnected;
    }

    void setURI(const std::string& uri);
    const std::string& getURI() const { return _uri; }

    /// Notify the NetConnection onStatus handler of a change.
    void notifyStatus(StatusCode code);

    /// Get an stream by name
    std::auto_ptr<IOChannel> getStream(const std::string& name);

protected:

	/// Mark responders associated with remoting calls
	void markReachableResources() const;

private:
    
    typedef std::pair<std::string, std::string> NetConnectionStatus;

    void getStatusCodeInfo(StatusCode code, NetConnectionStatus& info);

    /// Extend the URL to be used for playing
    void addToURL(const std::string& url);

    /// the url prefix optionally passed to connect()
    std::string		          _uri;
    bool		          _isConnected;
    unsigned int		  _numCalls;
    boost::scoped_ptr<HTTP>	  _http_client;
    boost::scoped_ptr<RTMPClient> _rtmp_client;
};

// This thread waits for data from the server, and executes the callback
extern "C" {
bool DSOEXPORT net_handler(NetConnection_as::thread_params_t *args);
}

void netconnection_class_init(as_object& global);

} // end of gnash namespace

#endif  // GNASH_NETCONNECTION_AS_H

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
