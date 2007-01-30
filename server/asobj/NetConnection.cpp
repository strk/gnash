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

/* $Id: NetConnection.cpp,v 1.19 2007/01/30 10:52:15 strk Exp $ */

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

#ifdef HAVE_CURL_CURL_H

/// \class NetConnection
/// \brief Opens a local connection through which you can play
/// back video (FLV) files from an HTTP address or from the local file
/// system, using curl.


// Ensure libcurl is initialized
static void ensure_libcurl_initialized()
{
	static bool initialized=0;
	if ( ! initialized ) {
		// TODO: handle an error here
		curl_global_init(CURL_GLOBAL_ALL);
		initialized=1;
	}
}

/*static private*/
int
NetConnection::progress_callback(void *clientp, double dltotal, 
		double /*dlnow*/, double /*ultotal*/, double /*ulnow*/)
{

	NetConnection* stream = (NetConnection*)clientp;
	stream->totalSize = dltotal;
	/*if (stream->callback && stream->callback_cache >= dltotal) {
		stream->netStreamObj->startPlayback();
		stream->callback = false;
	}*/
	return 0;
}

/*static private*/
size_t
NetConnection::recv(void *buf, size_t  size,  size_t  nmemb, 
	void *userp)
{

	NetConnection* stream = (NetConnection*)userp;
	return stream->cache(buf, size*nmemb);
}

	
/*private*/
size_t
NetConnection::cache(void *from, size_t sz)
{
	// take note of current position
	long curr_pos = ftell(_cache);

	// seek to the end
	fseek(_cache, 0, SEEK_END);

	size_t wrote = fwrite(from, 1, sz, _cache);
	if ( wrote < 1 )
	{
		char errmsg[256];
	
		snprintf(errmsg, 255,
			"writing to cache file: requested " SIZET_FMT ", wrote " SIZET_FMT " (%s)",
			sz, wrote, strerror(errno));
		fprintf(stderr, "%s\n", errmsg);
		throw gnash::GnashException(errmsg);
	}

	// reset position for next read
	fseek(_cache, curr_pos, SEEK_SET);

	return wrote;
}

/*public*/
size_t
NetConnection::tell()
{
	long ret =  ftell(_cache);

	return ret;

}

/*private*/
void
NetConnection::fill_cache(off_t size)
{
	struct stat statbuf;

	CURLMcode mcode;
	while (_running)
	{
		do
		{
			mcode=curl_multi_perform(_mhandle, &_running);
		} while ( mcode == CURLM_CALL_MULTI_PERFORM );

		if ( mcode != CURLM_OK )
		{
			throw gnash::GnashException(curl_multi_strerror(mcode));
		}

		// we already have that much data
		fstat(_cachefd, &statbuf);
		if ( statbuf.st_size >= size ) 
		{
			return;
		}


	}

}
NetConnection::NetConnection()
	:
	_cache(NULL),
	_cachefd(0),
	_url(),
	_handle(NULL),
	_mhandle(NULL),
	_running(false),
	localFile(true),
	netStreamObj(NULL),
	totalSize(0)
{
}

NetConnection::~NetConnection() {
	curl_multi_remove_handle(_mhandle, _handle);
	curl_easy_cleanup(_handle);
	curl_multi_cleanup(_mhandle);
	fclose(_cache);
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
bool NetConnection::openConnection(const char* char_url, as_object* ns)
{
	netStreamObj = ns;
	if (_url.size() > 0) {
		_url += "/";
	}
	_url += char_url;
	_running = 1;
	_cache = NULL;

	localFile = false;


	URL uri(_url);

	// Check if we're allowed to open url
	if (URLAccessManager::allow(uri)) {

		if (uri.protocol() == "file")
		{
			localFile = true;
		}
	} else {
		return false;
	}

	if (localFile) {
		_cache = fopen(char_url, "rb");
		if (_cache) {
			_cachefd = fileno(_cache);
			return true;
		} else {
			_cachefd = -1;
			return false;
		}
	}

	ensure_libcurl_initialized();

	_handle = curl_easy_init();
	_mhandle = curl_multi_init();

	/// later on we might want to accept a filename
	/// in the constructor
	_cache = tmpfile();
	if ( ! _cache ) {
		throw gnash::GnashException("Could not create temporary cache file");
	}
	_cachefd = fileno(_cache);

	CURLcode ccode;
	CURLMcode mcode;

	ccode = curl_easy_setopt(_handle, CURLOPT_USERAGENT, "Gnash-" VERSION);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}


/* from libcurl-tutorial(3)
When using multiple threads you should set the CURLOPT_NOSIGNAL  option
to TRUE for all handles. Everything will work fine except that timeouts
are not honored during the DNS lookup - which you can  work  around  by
*/

	ccode = curl_easy_setopt(_handle, CURLOPT_NOSIGNAL, true);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	// set url
	ccode = curl_easy_setopt(_handle, CURLOPT_URL, char_url);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	// set write data and function
	ccode = curl_easy_setopt(_handle, CURLOPT_WRITEDATA, this);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	ccode = curl_easy_setopt(_handle, CURLOPT_WRITEFUNCTION,
		NetConnection::recv);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	ccode = curl_easy_setopt(_handle, CURLOPT_FOLLOWLOCATION, 1);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	// set progress callback and function
	ccode = curl_easy_setopt(_handle, CURLOPT_PROGRESSFUNCTION, 
		NetConnection::progress_callback);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	ccode = curl_easy_setopt(_handle, CURLOPT_PROGRESSDATA, this);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	ccode = curl_easy_setopt(_handle, CURLOPT_NOPROGRESS, false);
	if ( ccode != CURLE_OK ) {
		throw gnash::GnashException(curl_easy_strerror(ccode));
	}

	// CURLMcode ret = 
	mcode = curl_multi_add_handle(_mhandle, _handle);
	if ( mcode != CURLM_OK ) {
		throw gnash::GnashException(curl_multi_strerror(mcode));
	}
	fill_cache(50000); // pre-cache 50 Kbytes

	return true;
}

/*public*/
void
NetConnection::addToURL(const char* url)
{
	_url += url;
}

/*public*/
bool
NetConnection::eof()
{
	bool ret = ( ! _running && feof(_cache) );
	
	return ret;

}
/*public*/
size_t
NetConnection::read(void *dst, size_t bytes)
{
	if ( eof() ) return 0;

	if (!localFile) fill_cache(tell()+bytes);

	return fread(dst, 1, bytes, _cache);

}

/*public*/
bool
NetConnection::seek(size_t pos)
{
	if (!localFile) fill_cache(pos);

	if ( fseek(_cache, pos, SEEK_SET) == -1 ) {
		return false;
	} else {
		return true;
	}

}


#endif // HAVE_CURL_CURL_H



netconnection_as_object::netconnection_as_object()
	:
	as_object(getNetConnectionInterface())
{
}

netconnection_as_object::~netconnection_as_object()
{
}

// Wrapper around dynamic_cast to implement user warning.
// To be used by builtin properties and methods.
static netconnection_as_object*
ensure_netconnection(as_object* obj)
{
	netconnection_as_object* ret = dynamic_cast<netconnection_as_object*>(obj);
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

	netconnection_as_object *netconnection_obj = new netconnection_as_object;

	fn.result->set_as_object(netconnection_obj);
}

static void
netconnection_connect(const fn_call& fn)
{
	GNASH_REPORT_FUNCTION;

	string filespec;
	netconnection_as_object *ptr = ensure_netconnection(fn.this_ptr); 
    
	if (fn.nargs > 0) {
		ptr->obj.addToURL(fn.arg(0).to_string());
	}    
}

void
attachNetConnectionInterface(as_object& o)
{

	o.init_member("connect", &netconnection_connect);

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

