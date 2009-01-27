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

#ifndef GNASH_STREAMPROVIDER_H
#define GNASH_STREAMPROVIDER_H

#include <map>
#include <memory>
#include "NetworkAdapter.h"

#include "dsodefs.h" // for DSOEXPORT

// Forward declarations
namespace gnash {
	class URL;
	class IOChannel;
}


namespace gnash {

/// Provide IOChannel streams for network or filesystem resources
class DSOEXPORT StreamProvider
{

public:

    typedef std::string (*NamingPolicy) (const URL&);

    static std::string defaultNamingPolicy(const URL& url);

	StreamProvider() {}

	virtual ~StreamProvider() {}

	static StreamProvider& getDefaultInstance();

	/// Returned stream ownership is transferred to caller.
	//
	/// On error NULL is returned
	/// Derive from this for a CachingStreamProvider
	///
	virtual std::auto_ptr<IOChannel> getStream(const URL& url,
            NamingPolicy np = 0);

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
	virtual std::auto_ptr<IOChannel> getStream(const URL& url,
            const std::string& postdata, NamingPolicy np = 0);
	
	virtual std::auto_ptr<IOChannel> getStream(const URL& url,
            const std::string& postdata,
            const NetworkAdapter::RequestHeaders& headers, NamingPolicy np = 0);
	
};

} // namespace gnash

#endif // _GNASH_STREAMPROVIDER_H


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
