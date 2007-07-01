// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/time.h>
#include <network.h>
#include <list>

#include "netstats.h"

namespace cygnal 
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
        NO_OSTYPE,
        LINUX,
        BSDS,
        DARWIN,
        WIN32,
        SOLARIS
    } ostype_e;
    void setIPaddr(in_addr_t x) { _ipaddr = x; };
    void setBrowser(browser_e x) { _browser = x; } ;
    int addStats();
    // these make calculations on the collected network data.
    float getFPS();
    int getBitRate();
    // Dump the collected network statistics in a human readable form.
    void dump();
    void clear();
private:
    in_addr_t           _ipaddr;
    const char	        *_filespec;
    browser_e           _browser;
    ostype_e            _os;
    std::list<NetStats *> _netstats;
};

 
} // end of cygnal namespace

#endif // __STATISTICS_H__

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
