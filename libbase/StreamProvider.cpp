// StreamProvider.cpp:  ActionScript file: or http: stream reader, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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

#include "GnashFileUtilities.h"
#include "StreamProvider.h"
#include "URL.h"
#include "tu_file.h"
#include "NetworkAdapter.h"
#include "URLAccessManager.h"
#include "log.h"
#include "rc.h" // for rcfile
#include "NamingPolicy.h"
#include "IOChannel.h"

#include <cerrno>
#include <cstring> // for strerror
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace gnash {

StreamProvider::StreamProvider(URL orig, URL base,
        std::unique_ptr<NamingPolicy> np)
    :
    _namingPolicy(std::move(np)),
    _base(std::move(base)),
    _original(std::move(orig))
{
}
    
bool
StreamProvider::allow(const URL& url) const
{
    return URLAccessManager::allow(url, _original);
}

struct dummy : public IOChannel
{
};

std::unique_ptr<IOChannel>
StreamProvider::getStream(const URL& url, bool namedCacheFile) const
{
    std::unique_ptr<IOChannel> stream;

	if (url.protocol() == "file") {

		std::string path = url.path();
		if (path == "-") {
            // TODO: only allow this as the *very first* call ?
            //       Rationale is a movie might request load of
            //       standard input, being a security issue.
            //       Note also that the FB gui will use stdin
            //       for key events.
            //
			int fd = dup(0);
			if (0 > fd) {
				log_error(_("Could not stdin (filename -): %2%"),
				          std::strerror(errno));
				return nullptr;
			}
			FILE *newin = fdopen(fd, "rb");

			// Close on destruction.
			stream = std::move(makeFileChannel(newin, true));
			return stream;
		}
		else {
            // check security here !!
		    if (!allow(url)) return stream;

			FILE *newin = std::fopen(path.c_str(), "rb");
			if (!newin)  { 
				log_error(_("Could not open file %1%: %2%"),
				          path, std::strerror(errno));
				return stream;
			}
			// Close on destruction
			stream = std::move(makeFileChannel(newin, true));
			return stream;
		}
	}
	else {
		if (allow(url)) {
			stream = std::move(NetworkAdapter::makeStream(url.str(), 
                    namedCacheFile ? namingPolicy()(url) : ""));
		}

        // Will return 0 unique_ptr if not allowed.
		return stream;
	}
}

std::unique_ptr<IOChannel>
StreamProvider::getStream(const URL& url, const std::string& postdata,
        const NetworkAdapter::RequestHeaders& headers, bool namedCacheFile)
        const
{

    if (url.protocol() == "file") {
        if (!headers.empty()) {
            log_error(_("Request Headers discarded while getting stream from file: uri"));
        }
        return getStream(url, postdata);
    }

	if (allow(url) ) {
		return NetworkAdapter::makeStream(url.str(), postdata, headers,
                    namedCacheFile ? namingPolicy()(url) : "");
	}

	return std::unique_ptr<IOChannel>();

}

std::unique_ptr<IOChannel>
StreamProvider::getStream(const URL& url, const std::string& postdata,
       bool namedCacheFile) const
{

    std::unique_ptr<IOChannel> stream;

	if (url.protocol() == "file") {
        if (!postdata.empty()) {    
		    log_error(_("POST data discarded while getting a stream "
                        "from file: uri"));
        }
		std::string path = url.path();
		if (path == "-") {
			FILE *newin = fdopen(dup(0), "rb");
			stream = makeFileChannel(newin, false);
			return stream;
		}
		else {
			if (!allow(url)) return stream;

			FILE *newin = std::fopen(path.c_str(), "rb");
			if (!newin)  { 
				log_error(_("Could not open file %1%: %2%"),
				          path, std::strerror(errno));
				return stream;
			}
			stream = makeFileChannel(newin, false);
			return stream;
		}
	}
	else {
		if (allow(url)) {
			stream = NetworkAdapter::makeStream(url.str(), postdata,
                    namedCacheFile ? namingPolicy()(url) : "");
		}
        // Will return 0 unique_ptr if not allowed.
		return stream;		

	}
}


} // namespace gnash

