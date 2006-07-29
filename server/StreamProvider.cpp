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

#ifdef HAVE_CURL_CURL_H
#define USE_CURL 1
#endif

#include "StreamProvider.h"
#include "URL.h"
#include "tu_file.h"
#ifdef USE_CURL
# include <curl/curl.h>
# include "curl_adapter.h"
#endif
#include "log.h"
#include "rc.h" // for rcfile

// temporary use of console for confirm load of network urls
#include <iostream>

#ifdef WIN32
#	include <io.h>
#else
#	include <unistd.h>
#endif

#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace gnash
{

// stuff for an URLAccessManager
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

/// The default AccessPolicy when prompting user is not possible
/// (this happens when input is not a tty, at the moment)
static AccessPolicy defaultAccessPolicy = GRANT;

/// A cache of AccessPolicy defined for URLs
typedef std::map< std::string, AccessPolicy > AccessPolicyCache;

/// A global AccessPolicyCache
static AccessPolicyCache policyCache;


/// Is access allowed to given url ?
/// This function uses the global AccessPolicyCache
/// so once a policy is defined for an url it will
/// be remembered for the whole run.
///
/// Prompts the user on the tty. If inut is not a tty
/// uses the global defaultAccessPolicy.
///
bool
allow(std::string& url)
{
	// Look in cached policy first
	AccessPolicyCache::iterator it = policyCache.find(url);
	if ( it != policyCache.end() )
	{
		log_msg("%s access to %s (cached).\n",
			accessPolicyString(it->second),
			url.c_str());

		return ( it->second == GRANT );
	}

	if ( ! isatty(fileno(stdin)) )
	{
		log_msg("%s access to %s (input is not a terminal).\n",
			accessPolicyString(defaultAccessPolicy),
			url.c_str());

		// If we can't prompt user return default policy
		return ( defaultAccessPolicy == GRANT );
	}

	/// I still don't like this method, typing just
	/// a newline doesn't spit another prompt
	std::string yesno;
	do {
		std::cout << "Attempt to access url " << url << std::endl;
		std::cout << "Block it [yes/no] ? "; 
		std::cin >> yesno;
	} while (yesno != "yes" && yesno != "no");

	AccessPolicy userChoice;

	if ( yesno == "yes" ) {
		userChoice = BLOCK;
	} else {
		userChoice = GRANT;
	}

	// cache for next time
	policyCache[url] = userChoice;
	
	log_msg("%s access to %s (user choice).\n",
		accessPolicyString(userChoice),
		url.c_str());

	return userChoice;

}

bool
host_check(const std::string& host)
{
    GNASH_REPORT_FUNCTION;

    std::cerr << "Checking security of host: " << host << std::endl;
    
    assert(host.size() > 0);
#if 0
    if (host.size() == 0) {
        return true;
    }
#endif
    
    bool check_domain = rcfile.useLocalDomain();
    bool check_localhost = rcfile.useLocalHost();
    char name[200];
    memset(name, 0, 200);
    gethostname(name, 200);

    if (check_domain) {
        char *domain = strchr(name, '.') + 1;
        if (host != domain) {
//        throw gnash::GnashException("Not in the local domain!");
            log_error("Not in the local domain!");
            return false;
        }
    }
    
    if (check_localhost) {
        *(strchr(name, '.')) = 0;
        if ((host != name) || (host == "localhost")) {
//        throw gnash::GnashException("Not on the localhost!");
            log_error("Not on the localhost!");
            return false;
        }
    }
    
    std::vector<std::string> whitelist = rcfile.getWhiteList();
    std::vector<std::string>::iterator it;
    for (it = whitelist.begin(); it != whitelist.end(); ++it) {
        if (*it == host) {
            dbglogfile << "Whitelisted host " << host.c_str() << "!" <<
		std::endl;
            return true;
        }
    }

    std::vector<std::string> blacklist = rcfile.getBlackList();
    for (it = blacklist.begin(); it != blacklist.end(); ++it) {
        if (*it == host) {
            dbglogfile << "Blacklisted host " << host.c_str() << "!"
               << std::endl;
            return false;
        }
    }
    
    return true;
}


} // AccessManager

tu_file*
StreamProvider::getStream(const URL& url)
{
//    GNASH_REPORT_FUNCTION;

	if (url.protocol() == "file")
	{
		std::string path = url.path();
		if ( path == "-" )
		{
			FILE *newin = fdopen(dup(0), "rb");
			return new tu_file(newin, false);
		}
		else
		{
        		return new tu_file(path.c_str(), "rb");
		}
	}
	else
	{
#ifdef USE_CURL
		std::string url_str = url.str();
		const char* c_url = url_str.c_str();
		//if ( URLAccessManager::allow(url_str) ) {
		if ( URLAccessManager::host_check(url.hostname()) ) {
			return curl_adapter::make_stream(c_url);
		} else {
			return NULL;
		}
#else
		log_error("Unsupported network connections");
		return NULL;
#endif
	}
}

} // namespace gnash

