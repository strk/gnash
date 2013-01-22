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

#include "URLAccessManager.h"
#include "URL.h"
#include "log.h"
#include "StringPredicates.h" 
#include "rc.h" // for rcfile
#include "GnashSystemNetHeaders.h"

#include <cerrno> 
#include <algorithm> // for find / find_if
#include <cstring> // for strerror
#include <cstdio>
#include <map>
#include <string>
#include <vector>
#include <cassert>

// Android fails to define this constant
#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN 256
#endif

namespace gnash {
namespace URLAccessManager {

/// Possible access policies for URLs
enum AccessPolicy {	

	/// Forbid access 
	BLOCK,

	/// Allow access
	GRANT
};

const char*
accessPolicyString(AccessPolicy policy)
{
	switch(policy)
	{
		case BLOCK:
			return "BLOCKED";
		case GRANT:
			return "GRANTED";
		default:
			return "UNKNOWN";
	}
}

// The default AccessPolicy when prompting user is not possible
// (this happens when input is not a tty, at the moment)
//static AccessPolicy defaultAccessPolicy = GRANT;

/// A cache of AccessPolicy defined for URLs
typedef std::map<std::string, AccessPolicy> AccessPolicyCache;

/// A global AccessPolicyCache
static AccessPolicyCache policyCache;

// check host against black/white lists
// return true if we allow load from host, false otherwise
// it is assumed localhost/localdomain was already checked
static bool
host_check_blackwhite_lists(const std::string& host)
{
	using std::vector;
	using std::string;

	RcInitFile& rcfile = RcInitFile::getDefaultInstance();


	const std::vector<std::string>& whitelist = rcfile.getWhiteList();
	if (!whitelist.empty()) {
		// TODO: case-insensitive matching ? 
        std::vector<std::string>::const_iterator it =
            std::find(whitelist.begin(), whitelist.end(), host);
		if (it != whitelist.end()) {
			log_security(_("Load from host %s granted (whitelisted)"),
				host);
			return true;
		}

		// if there is a whitelist, anything NOT listed is denied
		log_security(_("Load from host %s forbidden "
			"(not in non-empty whitelist)"),
			host);

		return false;
	}

    const std::vector<std::string>& blacklist = rcfile.getBlackList();

	// TODO: case-insensitive matching ? 
    std::vector<std::string>::const_iterator it =
        std::find(blacklist.begin(), blacklist.end(), host);

	if (it != blacklist.end()) {
		log_security(_("Load from host %s forbidden (blacklisted)"),
			host);
		return false;
	}

	log_security(_("Load from host %s granted (default)"), host);
	return true;
}

static bool
pathIsUnderDir(const std::string& path, const std::string& dir)
{
    size_t dirLen = dir.length();
    if ( dirLen > path.length() ) return false; // can't contain it, right ?

    // Path must be equal to dir for the whole dir length
    //
    // TODO: this is pretty lame, can do better with some normalization
    //       we'd need a generic splitPathInComponents.. maybe as a static
    //       public method of gnash::URL ?
    //
    if ( path.compare(0, dirLen, dir) ) return false;

    return true;
}

/// Return true if we allow load of the local resource, false otherwise.
//
static bool
local_check(const std::string& path, const URL& baseUrl)
{
//    GNASH_REPORT_FUNCTION;

    assert( ! path.empty() );

    // Don't allow local access if starting movie is a network resource.
   if (baseUrl.protocol() != "file") {
      log_security(_("Load of file %s forbidden"
          " (starting URL %s is not a local resource)"),
          path, baseUrl.str());
      return false;
   }

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    
    typedef RcInitFile::PathList PathList;
    const PathList& sandbox = rcfile.getLocalSandboxPath();

    for (PathList::const_iterator i=sandbox.begin(), e=sandbox.end();
            i!=e; ++i)
    {
        const std::string& dir = *i;
        if ( pathIsUnderDir(path, dir) ) 
        {
            log_security(_("Load of file %s granted (under local sandbox %s)"),
                path, dir);
            return true;
        }
    }

    // TODO: dump local sandboxes here ? (or maybe send the info to the GUI properties
    //       view
    log_security(_("Load of file %s forbidden (not under local sandboxes)"),
        path);
    return false;

}

/// Return true if we allow load from host, false otherwise.
//
/// This function will check for localhost/localdomain (if requested)
/// and finally call host_check_blackwhitelists
/// 
static bool
host_check(const std::string& host)
{
//    GNASH_REPORT_FUNCTION;

    //log_security(_("Checking security of host: %s"), host.c_str());
    
    assert( ! host.empty() );

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    
    bool check_domain = rcfile.useLocalDomain();
    bool check_localhost = rcfile.useLocalHost();

    // Don't bother gettin hostname if we're not going to need it
    if ( ! ( check_domain  || check_localhost ) )
    {
        return host_check_blackwhite_lists(host);
    }

    //
    // Get hostname
    //

//  #define MAXHOSTNAMELEN 200
    char name[MAXHOSTNAMELEN];
    if (::gethostname(name, MAXHOSTNAMELEN) == -1)
    {
        // FIXME: strerror is NOT thread-safe
        log_error(_("gethostname failed: %s"), std::strerror(errno)); 
        return host_check_blackwhite_lists(host);
    }
    // From GETHOSTNAME(2): 
    // In case the NUL-terminated hostname does not fit,
    // no  error is returned, but the hostname is truncated. It is unspecified
    // whether the truncated hostname will be NUL-terminated.
    name[MAXHOSTNAMELEN - 1] = '\0'; // unlikely, still worth making sure...

    // ok, let's use std::strings... we're a C++ program after all !
    std::string hostname(name); // the hostname
    std::string domainname;     // the domainname

    // Split hostname/domainname or take it all as an hostname if
    // no dot is found
    std::string::size_type dotloc = hostname.find('.', 0);
    if ( dotloc != std::string::npos ) {
        domainname = hostname.substr(dotloc+1);
        hostname.erase(dotloc);
    }

    if ( check_domain && domainname != host ) {
	log_security(_("Load from host %s forbidden (not in the local domain)"),
		host);
        return false;
	}
    
    if ( check_localhost && hostname != host ) {
	log_security(_("Load from host %s forbidden (not on the local host)"),
		host);
        return false;
    }

    return host_check_blackwhite_lists(host);

}

bool
allowHost(const std::string& host)
{
	if (host.size() == 0) {
		return true;
	}
	return host_check(host);
}

bool
allowXMLSocket(const std::string& host, short port)
{
    if (port < 1024) {
        log_security(_("Attempt to connect to disallowed port %s"), port);
        return false;
    }
	return allowHost(host);
}


bool
allow(const URL& url, const URL& baseurl)
{
	log_security(_("Checking security of URL '%s'"), url);

	// We might reintroduce use of an AccessPolicy cache

	std::string host = url.hostname();

	// Local resources can be accessed only if they are
	// in a directory listed as local sandbox
	if (host.size() == 0)
	{
		if (url.protocol() != "file")
        {
            log_error(_("Network connection without hostname requested"));
            return false;
        }
		return local_check(url.path(), baseurl);
	}
	return host_check(host);
}


} // AccessManager
} // namespace gnash

