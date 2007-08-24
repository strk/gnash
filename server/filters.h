// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/* $Id: filters.h,v 1.1 2007/08/24 05:44:18 cmusick Exp $ */

#ifndef GNASH_FILTERS
#define GNASH_FILTERS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <vector>
#include <boost/shared_ptr.hpp>

namespace gnash {

class stream;

namespace effect_filters {

class effect_filter;

typedef boost::shared_ptr<effect_filter> effect_filter_ptr;
typedef std::vector<effect_filter_ptr> effect_filters_vec;

class filter_factory
{
public:
    /// \brief Read one, possibly multiple filters from the stream,
    /// and push them into the vector store. Returns the number read.
    /// If read_multiple is true, the first byte of the stream is
    /// expected to contain the number of filters to be read.
    static int const read(stream* in, int movie_version, bool read_multiple,
        effect_filters_vec *store);
};

// Common base class for the filters.
class effect_filter
{
public:
    // Fill from a stream.
    virtual bool read(stream* in) = 0;

    virtual ~effect_filter() { return; }
};

} // Namespace effect_filters
} // Namespace gnash

#endif // GNASH_FILTERS
