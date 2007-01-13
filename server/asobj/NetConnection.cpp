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

/* $id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string>
#include <new>
#include "NetConnection.h"
#include "fn_call.h"
#include "rtmp.h"
#include "log.h"
#include "GnashException.h"

using namespace std;
using namespace amf;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace gnash {

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
		double dlnow, double /*ultotal*/, double /*ulnow*/)
{

	NetConnection* stream = (NetConnection*)clientp;
	stream->totalSize = dltotal;
	/*if (stream->callback && stream->callback_cache >= dltotal) {
		stream->netStreamObj->startPlayback();
		stream->callback = false;
	}*/
	printf("\tprogress callback, cached: %lf, totalsize: %lf\n", dlnow, dltotal);
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
NetConnection::NetConnection() {
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
/// \return nothing
/// \note Older Flash movies can only take a NULL value as
/// the parameter, which therefor only connects to the localhost using
/// RTMP. Newer Flash movies have a parameter to connect which is a
/// URL string like rtmp://foobar.com/videos/bar.flv
/*public*/
bool NetConnection::openConnection(const char* char_url, as_object* ns, bool local)
{
	netStreamObj = ns;
	_url = std::string(char_url);
	_running = 1;
	_cache = NULL;

	localFile = local;

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

/// \brief callback to instantiate a new NetConnection object.
/// \param fn the parameters from the Flash movie
/// \return nothing from the function call.
/// \note The return value is returned through the fn.result member.
void
netconnection_new(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
        
    netconnection_as_object *netconnection_obj = new netconnection_as_object;

    netconnection_obj->set_member("connect", &netconnection_connect);

    fn.result->set_as_object(netconnection_obj);
}

void netconnection_connect(const fn_call& fn)
{
    GNASH_REPORT_FUNCTION;
    
    string filespec;
    netconnection_as_object *ptr = (netconnection_as_object*)fn.this_ptr;
    
    assert(ptr);
    if (fn.nargs != 0) {
        filespec = fn.env->bottom(fn.first_arg_bottom_index).to_string();
//        ptr->obj.connect(filespec.c_str());
    } else {
//        ptr->obj.connect(0);
    }    
}


} // end of gnash namespace

