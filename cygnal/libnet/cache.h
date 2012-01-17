// 
//   Copyright (C) 2008, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
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

#ifndef __CACHE_H__
#define __CACHE_H__

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <string>
#include <map> 
#include <iostream>
#include <boost/shared_ptr.hpp>

#include "statistics.h"
#include "diskstream.h"
#include "getclocktime.hpp"
#include "dsodefs.h"

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

// max size of files to map enirely into the cache
static const size_t CACHE_LIMIT = 102400000;

// forward instatiate
//class DiskStream;

/// \class Cache
//
class DSOEXPORT Cache {
public:
    Cache();
    ~Cache();
    DSOEXPORT static Cache& getDefaultInstance();
    
    void DSOEXPORT addPath(const std::string &name, const std::string &fullpath);
    std::string &findPath(const std::string &name);
    void removePath(const std::string &name);
    
    void addResponse(const std::string &name, const std::string &response);
    std::string &findResponse(const std::string &name);
    void removeResponse(const std::string &name);
    
    void addFile(const std::string &name, boost::shared_ptr<DiskStream > &file);
    boost::shared_ptr<DiskStream> & findFile(const std::string &name);
    void removeFile(const std::string &name);
    
    ///  \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;    

#ifdef USE_STATS_CACHE
    std::string DSOEXPORT stats(bool xml) const;
#endif
private:
    /// \var Cache::_pathnames
    ///		The cache of file names converted to absolute path names.
    std::map<std::string, std::string> _pathnames;
    /// \var Cache::
    ///		The cache of HTTP responses.
    std::map<std::string, std::string> _responses;
    /// \var Cache::_responses
    ///		The cache of Distream handles to often played files.
    std::map<std::string, boost::shared_ptr<DiskStream> > _files;

    /// \var Cache::_max_size
    ///		The maximum amount of memory the cache is allowed to use.
    size_t _max_size;

    /// \brief Cache file statistics variables are defined here.
#ifdef USE_STATS_CACHE
    struct timespec _last_access;
    long	_pathname_lookups;
    long	_pathname_hits;
    long	_response_lookups;
    long	_response_hits;
    long	_file_lookups;
    long	_file_hits;
#endif
    /// \var Cache::_pagesize
    ///		The memory page size.
    size_t	_pagesize;    

cygnal::AMF::filetype_e  _filetype; // FIXME: this shouldn't be here still
};

/// \brief Dump to the specified output stream.
inline std::ostream& operator << (std::ostream& os, const Cache& cache)
{
	cache.dump(os);
	return os;
}

} // end of cygnal namespace

#endif // __CACHE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
