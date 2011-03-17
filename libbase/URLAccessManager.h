// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifndef GNASH_URLACCESSMANAGER_H
#define GNASH_URLACCESSMANAGER_H

#include <string>

#include "dsodefs.h"

// Forward declarations
namespace gnash {
	class URL;
}

namespace gnash {

	/// Manage a list of URL access configuration
	// stuff for an URLAccessManager
	namespace URLAccessManager {

	/// Return true if access to given url is allowed, false otherwise.
	//
	/// Will use rc file for whitelist/blacklist and localSandbox.
	///
	/// @param url
	///   The url to check
	///
	/// @param baseurl
	///   The originating url, only used for local urls and only
	///   for protocol checking (ie: a non-local originating url
	///   will not be allowed to load a local url)
	///
	bool allow(const URL& url, const URL& baseurl);

	//bool allowHost(const std::string& str);

	// Used by XMLSocket::connect (and others?)
	DSOEXPORT bool allowXMLSocket(const std::string& str, short port);

	} // AccessManager

} // namespace gnash

#endif // _GNASH_URLACCESSMANAGER_H
