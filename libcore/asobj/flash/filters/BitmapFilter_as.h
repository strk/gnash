// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc.
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

#ifndef GNASH_ASOBJ_BITMAPFILTER_H
#define GNASH_ASOBJ_BITMAPFILTER_H

#include "Global_as.h"

namespace gnash {
    class as_object;
    struct ObjectURI;
}

namespace gnash {

/// Initialize the global BitmapFilter class
void bitmapfilter_class_init(as_object& where, const ObjectURI& uri);

void registerBitmapFilterNative(as_object& global);

/// Convenience function only for BitmapFilter subclasses.
//
/// This implements the AS code necessary for defining subclasses of
/// BitmapFilter in AS2.
void registerBitmapClass(as_object& where, Global_as::ASFunction ctor,
        Global_as::Properties p, const ObjectURI& uri);

} // end of gnash namespace

#endif

