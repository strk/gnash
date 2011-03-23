// GnashFileUtilities.cpp     File handling for Gnash
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

#ifndef GNASH_FILE_HEADERS_H
#define GNASH_FILE_HEADERS_H

#include "GnashFileUtilities.h"

#include <string>
#include <boost/tokenizer.hpp>
#include <cerrno>

namespace gnash {

/// Create a directory for a given filename.
//
/// Everything after the last '/' is assumed to be the filename.
bool
mkdirRecursive(const std::string& filename)
{
    // Not a directory, nothing to do.
    std::string::size_type pos = filename.rfind("/");
    if (pos == std::string::npos) {
        return true;
    }
    const std::string& target = filename.substr(0, pos);
    
    typedef boost::tokenizer<boost::char_separator<char> > Tok;
    boost::char_separator<char> sep("/");
    Tok t(target, sep);
    std::string newdir;
    // Start from the root if the given filename was given
    // with an absolute path
    if ( filename[0] == '/' ) newdir += "/";

    for (Tok::iterator tit = t.begin(); tit != t.end(); ++tit) {

        newdir += *tit;

        if (newdir.find("..") != std::string::npos) {
            return false;
        }

        int ret = mkdirUserPermissions(newdir);
        
        if ((errno != EEXIST) && (ret != 0)) {
            return false;
        }
        newdir.push_back('/');
    }
    return true;
}

} // namespace gnash

#endif
