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

#ifndef GNASH_MOVIELIBRARY_H
#define GNASH_MOVIELIBRARY_H

#include "rc.h"
#include "movie_definition.h"

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <map>
#include <algorithm>

namespace gnash {

/// Library of SWF movies indexed by URL strings
//
/// Elements are actually movie_definitions, the ones
/// associated with URLS. They may be BitmapMovieDefinitions or
/// SWFMovieDefinitions.
class MovieLibrary
{
public:

    struct LibraryItem
    {
        boost::intrusive_ptr<movie_definition> def;
        unsigned hitCount;
    };

    typedef std::map<std::string, LibraryItem> LibraryContainer;

    MovieLibrary()
        : 
        _limit(8) 
    {
        RcInitFile& rcfile = RcInitFile::getDefaultInstance();
	    setLimit(rcfile.getMovieLibraryLimit());
    }
  
    /// Sets the maximum number of items to hold in the library. When adding new
    /// items, the one with the least hit count is being removed in that case.
    /// Zero is a valid limit (disables library). 
    void setLimit(LibraryContainer::size_type limit)
    {
        _limit = limit;  
        limitSize(_limit);  
    }

    bool get(const std::string& key,
            boost::intrusive_ptr<movie_definition>* ret)
    {
        std::lock_guard<std::mutex> lock(_mapMutex);
        LibraryContainer::iterator it = _map.find(key);
        if (it == _map.end()) return false;
        
        *ret = it->second.def;
        it->second.hitCount++;
        return true;
    }

    void add(const std::string& key, movie_definition* mov)
    {

        if (!_limit) return;

        if (_limit) limitSize(_limit - 1);

        LibraryItem temp;

        temp.def = mov;
        temp.hitCount = 0;

        std::lock_guard<std::mutex> lock(_mapMutex);
        _map[key] = temp;
    }
  

    void clear()
    {
        std::lock_guard<std::mutex> lock(_mapMutex);
        _map.clear();
    }
  
private:

    static bool findWorstHitCount(const LibraryContainer::value_type& a,
                                const LibraryContainer::value_type& b)
    {
        return (a.second.hitCount < b.second.hitCount);
    }

    LibraryContainer _map;
    unsigned _limit;

    void limitSize(LibraryContainer::size_type max) {

        if (max < 1) {
            clear();
            return;
        }

        while (_map.size() > max) {
            std::lock_guard<std::mutex> lock(_mapMutex);
            _map.erase(std::min_element(_map.begin(), _map.end(),
                        &findWorstHitCount));
        }
    
    }

    mutable std::mutex _mapMutex;
  
};

}
#endif 


// Local Variables:
// mode: C++
// c-basic-offset: 8 
// tab-width: 8
// indent-tabs-mode: t
// End:
