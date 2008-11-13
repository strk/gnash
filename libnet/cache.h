// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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

#include <string>
#include <map> 
#include <iostream> // for output operator

#include "statistics.h"
#include "diskstream.h"

/// \namespace gnash
///	This is the main namespace for Gnash and it's libraries.
namespace gnash {

// forward instatiate
//class DiskStream;

/// \class Cache
//
class Cache {
public:
    Cache();
    ~Cache();
    
    void addPath(const std::string &name, const std::string &fullpath) { _pathnames[name] = fullpath; };
    void addResponse(const std::string &name, const std::string &response) { _responses[name] = response; };
    void addFile(const std::string &name, DiskStream *file) { _files[name] = file; };

    std::string &findPath(const std::string &name) { return _pathnames[name]; };
    std::string &findResponse(const std::string &name) { return _responses[name]; };
    DiskStream *findFile(const std::string &name) { return _files[name]; };

    ///  \brief Dump the internal data of this class in a human readable form.
    /// @remarks This should only be used for debugging purposes.
    void dump() const { dump(std::cerr); }
    /// \overload dump(std::ostream& os) const
    void dump(std::ostream& os) const;    
    
private:
    std::map<std::string, std::string> _pathnames;
    std::map<std::string, std::string> _responses;
    std::map<std::string, DiskStream *> _files;
    size_t _max_size;
    /// \var Cache::_pagesize
    ///		The memory page size.
    size_t	pagesize;        
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
// indent-tabs-mode: t
// End:
