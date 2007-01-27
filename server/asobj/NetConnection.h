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

/* $Id: NetConnection.h,v 1.14 2007/01/27 16:55:05 tgc Exp $ */

#ifndef __NETCONNECTION_H__
#define __NETCONNECTION_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_CURL_CURL_H
#include <curl/curl.h>
#include <stdexcept>
#include <cstdio>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <string>

// TODO: port to new AS architecture
//
#include "as_object.h" // for inheritance
#include "fn_call.h"

namespace gnash {
#ifdef HAVE_CURL_CURL_H
class NetConnection {
public:

	NetConnection();
	~NetConnection();

	/// Opens the connection to char_url
	bool openConnection(const char* char_url, as_object* ns);

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

	// Extend the URL to be used for playing
	void addToURL(const char* url);
	
private:
	// Use this file to cache data
	FILE* _cache;

	// _cache file descriptor
	int _cachefd;

	// we keep a copy here to be sure the char*
	// is alive for the whole CurlStreamFile lifetime
	// TODO: don't really do this :)
	std::string _url;

	// the libcurl easy handle
	CURL *_handle;

	// the libcurl multi handle
	CURLM *_mhandle;

	// transfer in progress
	int _running;

	// Attempt at filling the cache up to the given size.
	// Will call libcurl routines to fetch data.
	void fill_cache(off_t size);

	// Append sz bytes to the cache
	size_t cache(void *from, size_t sz);

	void printInfo();

	// Callback for libcurl, will be called
	// by fill_cache() and will call cache() 
	static size_t recv(void *buf, size_t  size, 
		size_t  nmemb, void *userp);

	// If the file is local
	bool localFile;

	// The NetStream object which handles the video playback
	as_object* netStreamObj;

	// Total filesize
	double totalSize;

	// Callback for libcurl, will be used to detect total filesize
	static int progress_callback(void *clientp, double dltotal, 
		double dlnow, double ultotal, double ulnow);


};

#else
class NetConnection {
public:
	NetConnection() {}
	~NetConnection() {}
	bool openConnection(const char* /*char_url*/, as_object* /*ns*/) { return false; }
	bool seek(size_t /*pos*/) { return 0; }
	size_t read(void* /*dst*/, size_t /*bytes*/) { return false; }
	void addToURL(const char* /*url*/) {}
};

#endif // HAVE_CURL_CURL_H

class netconnection_as_object : public as_object
{
public:
	netconnection_as_object();
	~netconnection_as_object();
	NetConnection obj;
};

void netconnection_class_init(as_object& global);

} // end of gnash namespace

// __NETCONNECTION_H__
#endif
