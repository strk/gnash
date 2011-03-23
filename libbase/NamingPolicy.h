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

#ifndef GNASH_NAMINGPOLICY_H
#define GNASH_NAMINGPOLICY_H

#include "URL.h"
#include "dsodefs.h"

#include <string>

namespace gnash {


class NamingPolicy
{
public:
    NamingPolicy() {}
    virtual ~NamingPolicy() {}
    virtual std::string operator()(const URL&) const
    {
        return std::string();
    }
};


/// Make a non-unique cachefile name from the supplied name.
/// If the directory cannot be created, return an empty string.
class OverwriteExisting : public NamingPolicy
{
public:
    virtual std::string operator()(const URL&) const;
};

/// Make a unique cachefile name from the supplied name.
/// If all possible filenames are taken, return an empty string.
class DSOEXPORT IncrementalRename : public NamingPolicy
{
public:
    IncrementalRename(const URL& baseURL);
    virtual std::string operator()(const URL& url) const;
    
private:
    const URL _baseURL;
};

} // namespace gnash

#endif
