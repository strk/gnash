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


#ifndef GNASH_NETCONNECTION_H
#define GNASH_NETCONNECTION_H

#include "IOChannel.h"
#include <string>
#include "as_object.h" // for inheritance
#include "fn_call.h"

// Forward declarations
namespace gnash {
	class AMFQueue;
}

namespace gnash {

/// NetConnection ActionScript class
//
/// Provides interfaces to load data from an URL
///
class NetConnection_as: public as_object
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

	NetConnection_as();
	~NetConnection_as();

    /// Make the stored URI into a valid and checked URL.
	std::string validateURL() const;

    void call(as_object* asCallback, const std::string& callNumber, 
            const SimpleBuffer& buf);

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

    const std::string& getURI() const {
        return _uri;
    }

    /// Notify the NetConnection onStatus handler of a change.
    void notifyStatus(StatusCode code);

    /// Get an stream by name
    std::auto_ptr<IOChannel> getStream(const std::string& name);

protected:

	/// Mark responders associated with remoting calls
	void markReachableResources() const;

private:

	friend class AMFQueue;

    typedef std::pair<std::string, std::string> NetConnectionStatus;

    void getStatusCodeInfo(StatusCode code, NetConnectionStatus& info);

	/// Extend the URL to be used for playing
	void addToURL(const std::string& url);

	std::auto_ptr<AMFQueue> _callQueue;

	/// the url prefix optionally passed to connect()
	std::string _uri;

    bool _isConnected;

    bool _inError;


};

void netconnection_class_init(as_object& global);

} // end of gnash namespace

#endif
