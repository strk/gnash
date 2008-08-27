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

// TODO: port to new AS architecture
//
#include "as_object.h" // for inheritance
#include "fn_call.h"

// Forward declarations
namespace gnash {
	//class NetStream;
	class AMFQueue;
}

namespace gnash {

/// NetConnection ActionScript class
//
/// Provides interfaces to load data from an URL
///
class NetConnection: public as_object {
public:

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

	/// Register the "NetConnection" constructor to the given global object
	static void registerConstructor(as_object& global);

protected:

	/// Mark responders associated with remoting calls
	void markReachableResources() const;

private:
	friend class AMFQueue;

	std::auto_ptr<AMFQueue> _callQueue;

	/// Extend the URL to be used for playing
	void addToURL(const std::string& url);

	/// the url prefix optionally passed to connect()
	std::string _prefixUrl;

	/// the complete url of the file
	std::string _completeUrl;

	/// Attach ActionScript instance properties
	void attachProperties();

	/// Attach ActionScript class interface
	static void attachNetConnectionInterface(as_object& o);

	/// Get ActionScript class interface
	static as_object* getNetConnectionInterface();

	/// NetConnection.isConnected ActionScript Property
	static as_value isConnected_getset(const fn_call& fn);

	/// NetConnection.uri ActionScript Property
	static as_value uri_getset(const fn_call& fn);

	/// NetConnection.connect() ActionScript Method
	static as_value connect_method(const fn_call& fn);

	/// NetConnection.close() ActionScript Method
	static as_value close_method(const fn_call& fn);

	/// NetConnection.call() ActionScript Method
	static as_value call_method(const fn_call& fn);

	/// NetConnection.addHeader() ActionScript Method
	static as_value addHeader_method(const fn_call& fn);

};

void netconnection_class_init(as_object& global);

} // end of gnash namespace

// __NETCONNECTION_H__
#endif
