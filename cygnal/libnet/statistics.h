// 
//   Copyright (C) 2006, 2007, 2008, 2009, 2010,
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
//

#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <vector>
#include <sys/time.h>
#include <network.h>
#include <list>

#include "netstats.h"

namespace gnash 
{

class Statistics : public NetStats {
public:
    Statistics();
    ~Statistics();
    typedef enum {
        NO_BROWSER,
        MOZILLA,
        FIREFOX,
        OPERA,
        KONQUEROR,
        GALEON,
        EPIPHANY,
        SAFARI,
        IE
    } browser_e;
    typedef enum {
        OSTYPE_NONE,
        OSTYPE_LINUX,
        OSTYPE_BSD,
        OSTYPE_DARWIN,
        OSTYPE_WIN32,
        OSTYPE_SOLARIS
    } ostype_e;

    // Add a sample
    int addStats();
    
    // these make calculations on the collected network data.
    float getFPS();
    int getBitRate();
    
    // Accessors
    void setIPaddr(in_addr_t x) { _ipaddr = x; };
    void setBrowser(browser_e x) { _browser = x; } ;
    void setOS(ostype_e x) { _os = x; } ;
    in_addr_t getIPaddr() { return _ipaddr; };
    browser_e getBrowser() { return _browser; };
    ostype_e getOS() { return _os; };
    
//    void setFilespec(std::string &x) { _filespec = x; } ;
//    std::string &getFilespec() { return _filespec; };
    // Dump the collected network statistics in a human readable form.
    void dump();
    void clear();
private:
    in_addr_t           _ipaddr;
    browser_e           _browser;
    ostype_e            _os;
    std::list<NetStats *> _netstats;
    boost::uint32_t     _msg_count;
    std::vector<std::string> _filespec;
};
 
} // end of gnash namespace

#endif // __STATISTICS_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
