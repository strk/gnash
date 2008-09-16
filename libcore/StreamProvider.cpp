// StreamProvider.cpp:  ActionScript file: or http: stream reader, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif


#include "StreamProvider.h"
#include "URL.h"
#include "tu_file.h"
#include "NetworkAdapter.h"
#include "URLAccessManager.h"
#include "log.h"
#include "rc.h" // for rcfile

#include <cstdio>
#include <map>
#include <string>
#include <vector>


#if defined(_WIN32) || defined(WIN32)
#	include <io.h>
#	define dup _dup
#else
#include <unistd.h>
#endif

namespace gnash
{

StreamProvider&
StreamProvider::getDefaultInstance()
{
	static StreamProvider inst;
	return inst;
}

IOChannel*
StreamProvider::getStream(const URL& url)
{

	if (url.protocol() == "file")
	{
		std::string path = url.path();
		if ( path == "-" )
		{
            // TODO: only allow this as the *very first* call ?
            //       Rationale is a movie might request load of
            //       standar input, being a security issue.
            //       Note also that the FB gui will use stdin
            //       for key events.
            //
			FILE *newin = fdopen(dup(0), "rb");
			return new tu_file(newin, true); // close by dtor
		}
		else
		{
            // check security here !!
		    if ( ! URLAccessManager::allow(url) ) return NULL;

			FILE *newin = fopen(path.c_str(), "rb");
			if (!newin)  { 
				return NULL;
			}
			return new tu_file(newin, true); // close by dtor
		}
	}
	else
	{
		std::string url_str = url.str();
		const char* c_url = url_str.c_str();
		if ( URLAccessManager::allow(url) ) {
			return NetworkAdapter::make_stream(c_url);
		} else {
			return NULL;
		}
	}
}

std::auto_ptr<IOChannel>
StreamProvider::getStream(const URL& url, const std::string& postdata,
                          const NetworkAdapter::RequestHeaders& headers)
{
    if (url.protocol() == "file")
    {
        log_error("Request Headers discarded while getting stream from file: uri");
        return std::auto_ptr<IOChannel>(getStream(url, postdata));
    }

    std::auto_ptr<IOChannel> ret;

	std::string url_str = url.str();
	const char* c_url = url_str.c_str();
	if ( URLAccessManager::allow(url) ) {
		ret.reset(NetworkAdapter::makeStream(c_url, postdata, headers));
	}
	return ret;

}

IOChannel*
StreamProvider::getStream(const URL& url, const std::string& postdata)
{

	if (url.protocol() == "file")
	{
		log_error(_("POST data discarded while getting a stream from file: uri"));
		std::string path = url.path();
		if ( path == "-" )
		{
			FILE *newin = fdopen(dup(0), "rb");
			return new tu_file(newin, false);
		}
		else
		{
			if ( ! URLAccessManager::allow(url) ) return NULL;
			FILE *newin = fopen(path.c_str(), "rb");
			if (!newin)  { 
				return NULL;
			}
			return new tu_file(newin, false);
		}
	}
	else
	{
		std::string url_str = url.str();
		const char* c_url = url_str.c_str();
		if ( URLAccessManager::allow(url) ) {
			return NetworkAdapter::make_stream(c_url, postdata);
		} else {
			return NULL;
		}
	}
}

} // namespace gnash

