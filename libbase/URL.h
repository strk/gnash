// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_URL_H
#define GNASH_URL_H

#include "dsodefs.h"

#include <iosfwd>
#include <string>
#include <map>

namespace gnash {

/// Uniform Resource Locator
//
/// This class is used to manage URLs.
/// 
class DSOEXPORT URL
{

public:

	friend std::ostream& operator<< (std::ostream&o, const URL& u);

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
	const std::string& protocol() const { return _proto; }

	/// Return the 'hostname' member of this URL, as a string
	//
	/// NOTE: return the empty string if protocol() is "file"
	///
	const std::string& hostname() const { return _host; }

	/// Return the 'port' member of this URL, as a string
	//
	/// NOTE: return the empty string if the port isn't specified,
        /// as this is an optional field.
	///
	const std::string& port() const { return _port; }

	/// Return the 'path' member of this URL, as a string
    //
    /// The returned path starts with '/'
	const std::string& path() const { return _path; }

	/// Return the 'anchor' member of this URL, as a string
	//
	/// The anchor is the string after the '#' character
	///
	const std::string& anchor() const { return _anchor; }

	/// Return the 'querystring' member of this URL, as a string
	//
	/// The query is the string after the '?' character
	///
	const std::string& querystring() const { return _querystring; }
	
	/// Set the 'querystring' member of this URL to a new value
	//
	void set_querystring(const std::string& value) { _querystring = value; } 

	/// Return the full absolute URL as a string.
	//
	/// TODO: make output operator and operator+ for strings
	std::string str() const;

	/// Parse a query string filling the provided map
	//
	/// @param query_string 
	///	the url-escaped query string
	///	(can include or not the starting question mark)
	///
	/// @param target_map
	///	A standard map to put parsed values into.
	///	Note: existing elements of the map will be replaced.
	///
	/// @todo url-unescape names and values
	///
	/// @todo supports duplicated keys (var=value1&var=value2)
	///	
	static void parse_querystring(const std::string& query_string,
		 std::map<std::string, std::string>& target_map);

	/// \brief
	/// Encode a string to URL-encoded format
	/// converting all dodgy characters to %AB hex sequences
	//
	/// Characters that need escaping are:
	/// - ASCII control characters: 0-31 and 127
	/// - Non-ASCII chars: 128-255
	/// - URL syntax characters: $ & + , / : ; = ? @
	/// - Unsafe characters: SPACE " < > # % { } | \ ^ ~ [ ] `
	/// Encoding is a % followed by two hexadecimal characters, case insensitive.
	/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
	/// Section 2.2 "URL Character Encoding Issues"
	///
	///
	/// @param str
	///	The input/output string
	///
	static void encode(std::string& str);

	/// \brief
	/// Encode a string to URL-encoded format
	/// converting all dodgy characters to %AB hex sequences. This
	/// merely uses the void encode() function on a new string.
	///
	/// @ param str
	///	The input string
	/// @ return
	///	An encoded version of the input string	
	static std::string encode(const std::string& str);

	/// \brief
	/// Decode a string from URL-encoded format
	/// converting all hexadecimal sequences to ASCII characters.
	//
	/// A sequence to convert is % followed by two case-independent hexadecimal
	/// digits, which is replaced by the equivalent ASCII character.
	/// See RFC1738 http://www.rfc-editor.org/rfc/rfc1738.txt,
	/// Section 2.2 "URL Character Encoding Issues"
	///
	/// @param str
	///	The input/output string
	///
	static void decode(std::string& str);

private:
	void init_absolute(const std::string& absurl);

	void init_relative(const std::string& relurl, const URL& baseurl);

	/// Extract anchor from path
	void split_anchor_from_path();

	/// Extract tcp/ip port from path
	void split_port_from_host();

	/// Extract and parse query string from path
	void split_querystring_from_path();

	/// Normalize a 'path' component of an url
	//
	/// Normalization currently include removal
	/// of adjacent slashes, "./" dirs components
	/// removal, and "/../" components resolution
	///
	void normalize_path(std::string& path);

	std::string _proto;
	std::string _host;
	std::string _port;
	std::string _path;
	std::string _anchor;
	std::string _querystring;
};

DSOEXPORT std::ostream& operator<< (std::ostream&o, const URL& u);

} // end of gnash namespace

// __GNASH_URL_H__
#endif

