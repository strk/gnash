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

#ifndef __GNASH_URL_H__
#define __GNASH_URL_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace gnash {

/// Uniform Resource Locator
//
/// This class is used to manage URLs.
/// 
class URL
{

public:

	friend std::ostream& operator<< (const URL& u, std::ostream& o);

	/// Construct an URL from the given absolute url string.
	//
	/// A relative URL will be considered a filesystem
	/// path relative to the current working directory.
	///
	/// Throws std::runtime_error on error
	///
	URL(const std::string& absolute_url);

	/// Construct an URL from the given relative url string,
	/// using the given URL for resolving it.
	//
	/// Throws std::runtime_error on error
	///
	URL(const std::string& relative_url, const URL& baseurl);

	/// Return the 'protocol' member of this URL, as a string
	std::string protocol() const { return _proto; }

	/// Return the 'hostname' member of this URL, as a string
	//
	/// NOTE: return the empty string if protocol() is "file"
	///
	std::string hostname() const { return _host; }

	/// Return the 'path' member of this URL, as a string
	std::string path() const { return _path; }

	/// Return the full absolute URL as a string.
	//
	/// TODO: make output operator and operator+ for strings
	std::string toString() const;

private:

	void init(const char* absurl);

	std::string _proto;

	std::string _host;

	std::string _path;

};

std::ostream& operator<< (const URL& u, std::ostream& o);

} // end of gnash namespace

// __GNASH_URL_H__
#endif

