// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

/* $Id: NetConnection.h,v 1.19 2007/03/04 21:35:31 tgc Exp $ */

#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_file.h"

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
	class NetStream;
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

	/// Extend the URL to be used for playing
	void addToURL(const char* url);

	///	Returns the number of bytes cached
	long getBytesLoaded();

	///	Returns the total size of the file
	long getBytesTotal();


private:

	/// the url of the file
	std::string _url;

	/// the as_object which owns the connection
	as_object* _owner;

	/// The file connection
	tu_file* _stream;
};

void netconnection_class_init(as_object& global);

} // end of gnash namespace

// __NETCONNECTION_H__
#endif
