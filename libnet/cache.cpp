// cache.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/shared_array.hpp>
#include <boost/scoped_array.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <map>
#include <iostream>
#include <unistd.h>

#include "cache.h"
#include "log.h"
#include "diskstream.h"

using namespace std;

//static boost::mutex cache_mutex;

namespace gnash
{

Cache::Cache() 
    : _max_size(0)
{
}

Cache::~Cache()
{
}

void
Cache::dump(std::ostream& os) const
{
    
//    GNASH_REPORT_FUNCTION;
    
//    boost::mutex::scoped_lock lock(cache_mutex);

    // Dump all the pathnames
    map<std::string, std::string>::const_iterator name;
    for (name = _pathnames.begin(); name != _pathnames.end(); name++) {
        os << "Full path for \"" << name->first << "\" is: " << name->second << endl;
    }

    // Dump the responses
    for (name = _responses.begin(); name != _responses.end(); name++) {
        os << "Response for \"" << name->first << "\" is: " << name->second << endl;
    }

#if 0
    map<std::string, DiskStream>::const_iterator data;
    for (data = _files.begin(); data != _files.end(); data++) {
        DiskStream filedata = data->second;
        os << "File info for \"" << data->first << "\" is: ";
//        filedata.dump(os) << endl;
    }
#endif
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
