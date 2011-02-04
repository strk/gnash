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
//

#include <boost/date_time/date.hpp>
#include "netstats.h"
#include "log.h"


namespace gnash {

NetStats::NetStats()
{
//    GNASH_REPORT_FUNCTION;
}

NetStats::~NetStats()
{
//    GNASH_REPORT_FUNCTION;    
}

boost::posix_time::ptime
NetStats::startClock()
{
//    GNASH_REPORT_FUNCTION;

    _starttime = boost::posix_time::microsec_clock::local_time();
    return _stoptime;
}

boost::posix_time::ptime
NetStats::stopClock()
{
//    GNASH_REPORT_FUNCTION;
    
    _stoptime = boost::posix_time::microsec_clock::local_time();
    return _stoptime;
}

NetStats &
NetStats::operator = (NetStats &stats) {
    _starttime = stats.getStartTime();
    _stoptime = stats.getStopTime();
    _bytes = stats.getBytes();
    _type = stats.getFileType();
    return *this;
}

} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
