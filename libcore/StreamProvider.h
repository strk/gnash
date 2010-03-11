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

#ifndef GNASH_STREAMPROVIDER_H
#define GNASH_STREAMPROVIDER_H

#include "NetworkAdapter.h"
#include "dsodefs.h" // for DSOEXPORT
#include "NamingPolicy.h"

#include <string>
#include <memory>

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

	StreamProvider(std::auto_ptr<NamingPolicy> = 
            std::auto_ptr<NamingPolicy>(new NamingPolicy));

	virtual ~StreamProvider() {}

	/// Returned stream ownership is transferred to caller.
	//
	/// On error NULL is returned
	/// Derive from this for a CachingStreamProvider
	///
	virtual std::auto_ptr<IOChannel> getStream(const URL& url,
            bool namedCacheFile = false) const;

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
            const std::string& postdata, bool namedCacheFile = false) const;
	
	virtual std::auto_ptr<IOChannel> getStream(const URL& url,
            const std::string& postdata,
            const NetworkAdapter::RequestHeaders& headers,
            bool namedCacheFile = false) const;
	
    /// Set the NamingPolicy for cache files
    //
    /// This is only used when cache file naming is requested in getStream()
    /// This StreamProvider owns the NamingPolicy instance.
    void setNamingPolicy(std::auto_ptr<NamingPolicy> np)
    {
        _namingPolicy = np;
    }

    /// Return the currently selected policy for converting URL to filename
    const NamingPolicy& namingPolicy() const
    {
        assert(_namingPolicy.get());
        return *_namingPolicy;
    }

private:

    /// The current naming policy for cache files.
    std::auto_ptr<NamingPolicy> _namingPolicy;

};

} // namespace gnash

#endif 


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
