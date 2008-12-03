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
	//class NetStream_as;
	class AMFQueue;
}

namespace gnash {

/// NetConnection ActionScript class
//
/// Provides interfaces to load data from an URL
///
class NetConnection: public as_object
{
public:

    enum StatusCode
    {
        CONNECT_FAILED,
        CONNECT_SUCCESS,
        CALL_FAILED,
        CALL_SUCCESS
    };

	NetConnection();
	~NetConnection();

	/// Open a connection to stream FLV files.
	//
	/// If already connected an error is raised and false
	/// is returned. Otherwise, a connection is attempted
	/// using a separate thread that starts loading data
	/// caching it.
	///
	/// @param url
	///	An url portion to append to the base url (???)
	///
	/// @return true on success, false on error.
	///
	/// @note Older Flash movies can only take a NULL value as
	/// the parameter, which therefor only connects to the localhost using
	/// RTMP. Newer Flash movies have a parameter to connect which is a
	/// URL string like rtmp://foobar.com/videos/bar.flv
	///
	std::string validateURL(const std::string& url);

    void call(as_object* asCallback, const std::string& callNumber, 
            const SimpleBuffer& buf);

    void setConnected(bool b) {
        _isConnected = b;
    }

    bool isConnected() const {
        return _isConnected;
    }

	/// Extend the URL to be used for playing
	void addToURL(const std::string& url);

    void notifyStatus(StatusCode code);

protected:

	/// Mark responders associated with remoting calls
	void markReachableResources() const;
private:
	friend class AMFQueue;

	std::auto_ptr<AMFQueue> _callQueue;

	/// the url prefix optionally passed to connect()
	std::string _prefixUrl;

	/// the complete url of the file
	std::string _completeUrl;

    bool _isConnected;

    typedef std::pair<std::string, std::string> NetConnectionStatus;

    void getStatusCodeInfo(StatusCode code, NetConnectionStatus& info);

};

void netconnection_class_init(as_object& global);

} // end of gnash namespace

#endif
