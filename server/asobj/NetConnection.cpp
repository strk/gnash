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

/* $Id: NetConnection.cpp,v 1.27 2007/03/04 21:35:31 tgc Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string>
#include <new>
#include "NetConnection.h"
#include "log.h"
#include "GnashException.h"
#include "builtin_function.h"
#include "movie_root.h"

#include "StreamProvider.h"
#include "URLAccessManager.h"
#include "URL.h"

using namespace std;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace gnash {

static void netconnection_new(const fn_call& fn);
static void netconnection_connect(const fn_call& fn);
static as_object* getNetConnectionInterface();

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system, using curl.


NetConnection::NetConnection()
	:
	as_object(getNetConnectionInterface()),
	_url(),
	_owner(NULL),
	_stream(NULL)
{
}

NetConnection::~NetConnection() {

}

/// Open a connection to stream FLV files.
//
/// \param arg is the URL
/// \return true on success, false on error.
/// \note Older Flash movies can only take a NULL value as
/// the parameter, which therefor only connects to the localhost using
/// RTMP. Newer Flash movies have a parameter to connect which is a
/// URL string like rtmp://foobar.com/videos/bar.flv
/*public*/
bool NetConnection::openConnection(const char* char_url, as_object* owner)
{

	// if already running there is no need to setup things again
	if (_stream) return true;

	_owner = owner;
	if (_url.size() > 0) {
		_url += "/";
	}
	_url += char_url;

	URL uri(_url, get_base_url());

	_url = uri.str().c_str();

	// Check if we're allowed to open url
	if (!URLAccessManager::allow(uri)) {
		log_warning("Gnash is not allowed to open this url.");
		return false;
	}

	_stream = StreamProvider::getDefaultInstance().getStream(uri);

	if (!_stream) return false;

	return true;
}

/*public*/
void
NetConnection::addToURL(const char* url)
{
	if (strcmp(url, "null") != 0 && strcmp(url, "NULL") != 0) {
		_url += url;
	}
}

/*public*/
bool
NetConnection::eof()
{

	if (!_stream) return false;

	return _stream->get_eof();
}

/*public*/
size_t
NetConnection::read(void *dst, size_t bytes)
{
	if (!_stream) return 0;

	return _stream->read_bytes(dst, bytes);
}

/*public*/
bool
NetConnection::seek(size_t pos)
{
	if (!_stream) return false;

	_stream->set_position(pos);

	if (_stream->get_position() == pos)	return true;
	else return false;
}

/*public*/
size_t
NetConnection::tell()
{
	if (!_stream) return 0;

	return _stream->get_position();

}

/*public*/
long
NetConnection::getBytesLoaded()
{
	if (!_stream) return -1;

	return _stream->get_cur_size();
}


/*public*/
long
NetConnection::getBytesTotal()
{
	if (!_stream) return -1;

	return _stream->get_size();
}


// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static NetConnection*
ensure_netconnection(as_object* obj)
{
	NetConnection* ret = dynamic_cast<NetConnection*>(obj);
	if ( ! ret )
	{
		throw ActionException("builtin method or gettersetter for NetConnection objects called against non-NetConnection instance");
	}
	return ret;
}


/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
static void
netconnection_new(const fn_call& fn)
{
	GNASH_REPORT_FUNCTION;

	NetConnection *netconnection_obj = new NetConnection;

	fn.result->set_as_object(netconnection_obj);
}

static void
netconnection_connect(const fn_call& fn)
{
	GNASH_REPORT_FUNCTION;

	NetConnection *ptr = ensure_netconnection(fn.this_ptr); 
    
	if (fn.nargs > 0) {
		ptr->addToURL(fn.arg(0).to_string());
	}
}

void
attachNetConnectionInterface(as_object& o)
{

	o.init_member("connect", new builtin_function(netconnection_connect));

}

static as_object*
getNetConnectionInterface()
{

	static boost::intrusive_ptr<as_object> o;
	if ( o == NULL )
	{
		o = new as_object();
		attachNetConnectionInterface(*o);
	}

	return o.get();
}


// extern (used by Global.cpp)
void netconnection_class_init(as_object& global)
{

	// This is going to be the global NetConnection "class"/"function"
	static boost::intrusive_ptr<builtin_function> cl;

	if ( cl == NULL )
	{
		cl=new builtin_function(&netconnection_new, getNetConnectionInterface());
		// replicate all interface to class, to be able to access
		// all methods as static functions
		attachNetConnectionInterface(*cl);
		     
	}

	// Register _global.String
	global.init_member("NetConnection", cl.get());

}


} // end of gnash namespace

