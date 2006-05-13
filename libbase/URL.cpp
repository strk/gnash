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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "log.h"
#include "URL.h"

#include <string>
#include <cstring>
#include <stdexcept>
#include <cassert>

// these are for stat(2)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace gnash {

/*private*/
void
URL::init(const char* in)
{
	size_t len = strlen(in);
	const char* last = in+len;

	assert(*last==0);

	// Find protocol
	char* ptr = strstr(in, "://");
	if ( ptr )
	{
		// copy initial part to protocol
		_proto.assign(in, ptr-in);

		// advance input pointer to past the :// part
		in = ptr+3;


		// Find host (only if not 'file' protocol
		ptr = strchr(in, '/');
		if ( ! ptr )
		{
			// no slashes ? all hostname, I presume
			_host.assign(in, last-in);
		}
		else
		{
			// copy hostname
			_proto.assign(in, ptr-in);

			// advance input pointer to the path
			in = ptr;
		}
	}
	else
	{
		_proto = "file";
	}

	// What remains now is a path
	_path.assign(in, last-in);
}

/*public*/
URL::URL(const std::string& absolute_url)
{
	init(absolute_url.c_str());
}

/*public*/
URL::URL(const std::string& relative_url, const URL& baseurl)
{
	const char* in = relative_url.c_str();

	// If has a protocol, call absolute_url ctor
	if ( strstr(in, "://") )
	{
		init(in);
		return;
	}

	// use protocol and host from baseurl
	_proto = baseurl._proto;
	_host = baseurl._host;

	if ( in[0] == '/' ) // path-absolute
	{
		// get path from here
		_path.assign(in, strlen(in));
	}

	else // path-relative
	{
		// get dirname from basurl path
		_path = baseurl._path.substr(
			0,
			baseurl._path.find_last_of("/")+1);
		_path += relative_url;
	}

}

/*public*/
std::string
URL::str() const
{
	std::string ret = _proto;

	if ( _host != "" ) {
		ret += "://" + _host;
	} else {
		// it's a local filename
		ret += ":/" + _host;
	}
	ret += _path;

	return ret;
}

std::ostream& operator<< (std::ostream& o, const URL& u)
{
	return o << u.str();
}

} // end of gnash namespace

