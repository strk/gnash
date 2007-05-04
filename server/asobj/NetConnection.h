// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

/* $Id: NetConnection.h,v 1.27 2007/05/04 20:42:42 strk Exp $ */

#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"
#include "LoadThread.h"
#include "FLVParser.h"

#include <stdexcept>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#include <string>

// TODO: port to new AS architecture
//
#include "as_object.h" // for inheritance
#include "fn_call.h"

// Forward declarations
namespace gnash {
	//class NetStream;
}

namespace gnash {

class NetConnection: public as_object {
public:

	NetConnection();
	~NetConnection();

	/// Opens the connection to char_url
	bool openConnection(const char* char_url, as_object* owner);

	/// Put read pointer at given position
	bool seek(size_t pos);

	/// Read 'bytes' bytes into the given buffer.
	//
	/// Return number of actually read bytes
	///
	size_t read(void *dst, size_t bytes);

	/// Return true if EOF has been reached
	bool eof();

	/// Report global position within the file
	size_t tell();

	///	Returns the number of bytes cached
	long getBytesLoaded();

	///	Returns the total size of the file
	long getBytesTotal();

	/// Connects FLV parser with the LoadThread
	bool connectParser(FLVParser& parser);

	/// Returns whether the load is complete
	bool loadCompleted();

	/// Register the "NetConnection" constructor to the given global object
	static void registerConstructor(as_object& global);

private:

	/// Extend the URL to be used for playing
	void addToURL(const std::string& url);

	/// the url of the file
	std::string _url;

	/// the as_object which owns the connection
	boost::intrusive_ptr<as_object> _owner;

	/// The file/stream loader thread and interface
	std::auto_ptr<LoadThread> _loader;

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
