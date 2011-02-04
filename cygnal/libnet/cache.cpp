// cache.cpp:  HyperText Transport Protocol handler for Cygnal, for Gnash.
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

using std::string;
using std::map;
using std::endl;

static boost::mutex cache_mutex;

namespace gnash
{

Cache::Cache() 
    : _max_size(0),
#ifdef USE_STATS_CACHE
      _pathname_lookups(0),
      _pathname_hits(0),
      _response_lookups(0),
      _response_hits(0),
      _file_lookups(0),
      _file_hits(0),
#endif
      _pagesize(0)
{
//    GNASH_REPORT_FUNCTION;
    log_error("using this constructor is only allowed for testing purposes.");
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
#endif
}

Cache::~Cache()
{
//    GNASH_REPORT_FUNCTION;
//    dump();
}

Cache&
Cache::getDefaultInstance()
{
//    GNASH_REPORT_FUNCTION;
    static Cache c;
    return c;
}

void
Cache::addPath(const std::string &name, const std::string &fullpath)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
    _pathnames[name] = fullpath;
}

void
Cache::addResponse(const std::string &name, const std::string &response)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);

    _responses[name] = response;
}

void
Cache::addFile(const std::string &name, boost::shared_ptr<DiskStream> &file)
{
    // GNASH_REPORT_FUNCTION;

    boost::mutex::scoped_lock lock(cache_mutex);
    log_network("Adding file %s to cache.", name);
    _files[name] = file;
}

string &
Cache::findPath(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _pathname_lookups++;
    map<string, string>::iterator it;
    it = _pathnames.find(name);
    if (it != _pathnames.end()) {
        _pathname_hits++;
    }
#endif
    return _pathnames[name];
}

string &
Cache::findResponse(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _response_lookups++;
    map<string, string>::const_iterator it;
    it = _responses.find(name);
    if (it != _responses.end()) {
        _response_hits++;
    }
#endif
    return _responses[name];
}

boost::shared_ptr<DiskStream> &
Cache::findFile(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;

    log_network("Trying to find %s in the cache.", name);
    boost::mutex::scoped_lock lock(cache_mutex);
#ifdef USE_STATS_CACHE
    clock_gettime (CLOCK_REALTIME, &_last_access);
    _file_lookups++;
    map<string, boost::shared_ptr<DiskStream> >::const_iterator it;
    it = _files.find(name);
    if (it != _files.end()) {
        _file_hits++;
    }
#endif
    return _files[name];
}

void
Cache::removePath(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
    _pathnames.erase(name);
}

void
Cache::removeResponse(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
    _responses.erase(name);
}

void
Cache::removeFile(const std::string &name)
{
//    GNASH_REPORT_FUNCTION;
    boost::mutex::scoped_lock lock(cache_mutex);
    _files.erase(name);
}

#ifdef USE_STATS_CACHE
string
Cache::stats(bool xml) const
{
//    GNASH_REPORT_FUNCTION;
    // dump timing related data
    struct timespec now;
    std::stringstream text;
    
    clock_gettime (CLOCK_REALTIME, &now);    
    double time = ((now.tv_sec - _last_access.tv_sec) + ((now.tv_nsec - _last_access.tv_nsec)/1e9));

    if (xml) {
	text << "<cache>" << endl;
	text << "	<LastAccess>"       << time              << " </LastAccess>" << endl;
	text << "	<PathNames>" << endl
	     << "		<Total>" << _pathnames.size() << "</Total>" << endl
	     << "		<Hits>"     << _pathname_hits    << "</Hits>" << endl
	     << "	</PathNames>" << endl;
	text << "	<Responses>" << endl;
	text << "		<Total>" << _responses.size() << "</Total>" << endl
	     << "		<Hits>"     << _response_hits   << "</Hits>" << endl
	     << "       </Responses>" << endl;
	text << "	<Files>" << endl
	     << "		<Total>"     << _files.size()     << "</Total>" << endl
	     << "		<Hits>"     << _file_hits        << "</Hits>" << endl
	     << "       </Files>" << endl;
    } else {
	text << "Time since last access:  " << std::fixed << time << " seconds ago." << endl;
	
	text << "Pathnames in cache: " << _pathnames.size() << ", accessed "
	     << _pathname_lookups << " times" << endl;
	text << "	Pathname hits from cache: " << _pathname_hits << endl;
	
	text << "Responses in cache: " << _responses.size() << ", accessed "
	     << _response_lookups << " times" << endl;
	text << "	Response hits from cache: " << _response_hits << endl;
	
	text << "Files in cache: " << _files.size() << ", accessed "
	     << _file_lookups << " times" << endl;
	text << "	File hits from cache: " << _file_hits << endl;
    }
    
    map<std::string, boost::shared_ptr<DiskStream> >::const_iterator data;
    for (data = _files.begin(); data != _files.end(); data++) {
	const struct timespec *last = data->second->getLastAccessTime();
	time = ((now.tv_sec - last->tv_sec) + ((now.tv_nsec - last->tv_nsec)/1e9));
	if (xml) {
	    text << "	<DiskStreams>" << endl
		 << "		<Name>\"" << data->first << "\"</Name>" << endl
		 << "		<Hits>" << data->second->getAccessCount() << "</Hits>" << endl
		 << "		<LastAccess>" << time << "</LastAccess>" << endl
		 << "	</DiskStreams>" << endl;
	} else {
	    text << "Disktream: " << data->first
		 << ", accessed: " << data->second->getAccessCount()
		 << " times." << endl;
	    text << "	Time since last file access:  " << std::fixed << time << " seconds ago." << endl;
	}
    }

    if (xml) {
	text << "</cache>" << endl;
    }
    
    return text.str();
}
#endif

void
Cache::dump(std::ostream& os) const
{    
    GNASH_REPORT_FUNCTION;    
    boost::mutex::scoped_lock lock(cache_mutex);

    // Dump all the pathnames
    os << "Pathname cache has " << _pathnames.size() << " files." << endl;
    map<string, string>::const_iterator name;
    for (name = _pathnames.begin(); name != _pathnames.end(); name++) {
        os << "Full path for \"" << name->first << "\" is: " << name->second << endl;
    }

    // Dump the responses
    os << "Responses cache has " << _responses.size() << " files." << endl;
    for (name = _responses.begin(); name != _responses.end(); name++) {
        os << "Response for \"" << name->first << "\" is: " << name->second << endl;
    }
    
    os << "DiskStream cache has " << _files.size() << " files." << endl;
    
    map<std::string, boost::shared_ptr<DiskStream> >::const_iterator data;
    for (data = _files.begin(); data != _files.end(); data++) {
        boost::shared_ptr<DiskStream> filedata = data->second;
        os << "file info for \"" << data->first << "\" is: " << endl;
        filedata->dump();
        os << "-----------------------------" << endl;
    }

#ifdef USE_STATS_CACHE
    this->stats(false);
#endif
}

} // end of gnash namespace


// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
