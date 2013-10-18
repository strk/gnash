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


#ifndef NETWORK_ADAPTER_H
#define NETWORK_ADAPTER_H

#include "dsodefs.h"

#include <map>
#include <string>
#include <memory>
#include <set>
#include "StringPredicates.h"

namespace gnash {
class IOChannel;

/// Code to use libcurl as an IOChannel stream.
class NetworkAdapter {

public:

    /// Custom headers for addRequestHeader. These are case insensitive, and
    /// subsequent addition of a header already there replaces any previous one.
    /// Some values are not allowed.
    typedef std::map<std::string, std::string, StringNoCaseLessThan>
        RequestHeaders;

    /// \brief
    /// Returns a read-only IOChannel that fetches data
    /// from an url.
    //
    /// @param url      The url to fetch data from.
    DSOEXPORT static std::auto_ptr<IOChannel> makeStream(
            const std::string& url, const std::string& cachefile);

    /// \brief
    /// Returns a read-only IOChannel that fetches data
    /// from an url getting posted to.
    //
    /// The caller owns the returned IOChannel.  
    ///
    /// @param url      The url to post to.
    /// @param postdata The url-encoded post data
    DSOEXPORT static std::auto_ptr<IOChannel> makeStream(
            const std::string& url, const std::string& postdata,
            const std::string& cachefile);

    /// \brief
    /// Returns a read-only IOChannel that fetches data
    /// from an url getting posted to.
    //
    /// The caller owns the returned IOChannel.  
    ///
    /// @param url      The url to post to.
    /// @param postdata The url-encoded post data
    /// @param headers  A RequestHeaders map of custom headers to send.
    DSOEXPORT static std::auto_ptr<IOChannel> makeStream(const std::string& url,
           const std::string& postdata, const RequestHeaders& headers,
           const std::string& cachefile);


    typedef std::set<std::string, StringNoCaseLessThan> ReservedNames;

    /// Check whether a RequestHeader is permitted.
    //
    /// @param  headerName is checked against a set of reserved names
    ///         (case-insensitive).
    /// @return true if the name is allowed.
    DSOEXPORT static bool isHeaderAllowed(const std::string& headerName)
    {
        const ReservedNames& names = reservedNames();
        return (names.find(headerName) == names.end());
    }

    /// \brief
    /// Set CookiesIn variable to cookiefile path.
    //
    /// @param cookiesin      Cookiefile path
    static void setCookiesIn(const std::string& cookiesin) {
        NetworkAdapter::CookiesIn = cookiesin;
    }

    /// \brief
    /// Get CookiesIn variable, cookiefile path.
    //
    /// Returns the cookiefile path.
    static const std::string& getCookiesIn() {
        return NetworkAdapter::CookiesIn;
    }

private:
    /// \brief
    /// Variable to pass cookiefile path from command line to curl.
    static std::string CookiesIn;

    static const ReservedNames& reservedNames();

};

} // namespace gnash

#endif // CURL_ADAPTER_H

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
