// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#include <vector>
#include <string>
#include <list>
#include <memory>
#include <boost/shared_ptr.hpp>

#include "Relay.h"

// Forward declarations
namespace gnash {
    class Connection;
    class as_object;
    class as_value;
    class IOChannel;
    struct ObjectURI;
}

namespace gnash {

/// NetConnection ActionScript class
//
/// Provides interfaces to load data from an URL
///
class NetConnection_as : public ActiveRelay
{
public:

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

    NetConnection_as(as_object* owner);

    virtual ~NetConnection_as();

    /// Process connection stuff
    virtual void update();

    /// Make the stored URI into a valid and checked URL.
	std::string validateURL() const;

    void call(as_object* asCallback, const std::string& methodName,
            const std::vector<as_value>& args);

    /// Process the close() method.
    void close();

    /// Process the connect(uri) method.
    //
    /// Return false if the connection is disallowed or invalid,
    /// true if a connection will be attempted.
    bool connect(const std::string& uri);

    /// Carry out the connect(null) method.
    //
    /// There is no return because this attempt is always considered
    /// be be successful.
    void connect();

    void setConnected() {
        _isConnected = true;
    }

    bool isConnected() const {
        return _isConnected;
    }

    void setURI(const std::string& uri);

    const std::string& getURI() const {
        return _uri;
    }

    /// Notify the NetConnection onStatus handler of a change.
    void notifyStatus(StatusCode code);

    /// Get an stream by name
    std::auto_ptr<IOChannel> getStream(const std::string& name);

    /// Mark responders associated with remoting calls
    void markReachableResources() const;

private:

    bool isRTMP();

    void createStream(as_object* asCallback);

    /// Extend the URL to be used for playing
    void addToURL(const std::string& url);

    typedef std::list<boost::shared_ptr<Connection> > Connections;

    /// Queue of call groups
    //
    /// A queue of persisting closed connections. Because HTTP remoting
    /// requests in particular can take time to send and receive data, they
    /// can still be active after the connection was conceptually closed. This
    /// stores those connections until they are finished.
    Connections _oldConnections;

    /// The current conceptual network connection.
    std::auto_ptr<Connection> _currentConnection; 

    /// the url prefix optionally passed to connect()
    std::string _uri;

    bool _isConnected;

    void startAdvanceTimer();

    void stopAdvanceTimer();
};

void netconnection_class_init(as_object& global, const ObjectURI& uri);

} // end of gnash namespace

#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
