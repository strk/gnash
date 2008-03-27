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

#ifndef _GNASH_STREAMPROVIDER_H
#define _GNASH_STREAMPROVIDER_H

#include <string>

// Forward declarations
class tu_file;
namespace gnash {
	class URL;
}


namespace gnash {

/// Provide tu_file streams for network or filesystem resources
class StreamProvider
{

public:

	StreamProvider() {}

	virtual ~StreamProvider() {}

	static StreamProvider& getDefaultInstance();

	/// Returned stream ownership is transferred to caller.
	//
	/// On error NULL is returned
	/// Derive from this for a CachingStreamProvider
	///
	virtual tu_file* getStream(const URL& url);

	/// Get a stream from the response of a POST operation
	//
	/// Returned stream ownership is transferred to caller.
	///
	/// On error NULL is returned
	/// Derive from this for a CachingStreamProvider
	///
	/// @param url
	///	The url to post to.
	///
	/// @param postdata
	///	Post data in url-encoded form.
	///
	///
	virtual tu_file* getStream(const URL& url, const std::string& postdata);
	
};

} // namespace gnash

#endif // _GNASH_STREAMPROVIDER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
