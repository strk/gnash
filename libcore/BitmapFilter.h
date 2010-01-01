// 
//   Copyright (C) 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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


#ifndef GNASH_BITMAPFILTER_H
#define GNASH_BITMAPFILTER_H

#include <boost/shared_ptr.hpp>

namespace gnash {

class SWFStream;
class BitmapFilter;
typedef boost::shared_ptr<BitmapFilter> Filter;

// The common base class for AS display filters.
class BitmapFilter
{
public:
    // Fill from a stream. See parser/filter_factory.cpp for the implementations.
    virtual bool read(SWFStream& /*in*/) { return true; }

    BitmapFilter() { return; }
    virtual ~BitmapFilter() { return; }
};

} // Namespace gnash

#endif // GNASH_BITMAPFILTER_H
