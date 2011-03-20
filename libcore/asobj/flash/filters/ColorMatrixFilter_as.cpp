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

#include <boost/intrusive_ptr.hpp>

#include "ColorMatrixFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"
#include "smart_ptr.h"

namespace gnash {

namespace {
    as_value colormatrixfilter_new(const fn_call& fn);
    as_value colormatrixfilter_matrix(const fn_call& fn);

    void attachColorMatrixFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class ColorMatrixFilter_as : public Relay, public ColorMatrixFilter
{
public:
    ColorMatrixFilter_as() {}
};

/// The prototype of flash.filters.ColorMatrixFilter is a new BitmapFilter.
void
colormatrixfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, colormatrixfilter_new,
            attachColorMatrixFilterInterface, uri);
}

namespace {

void
attachColorMatrixFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("matrix", colormatrixfilter_matrix, 
        colormatrixfilter_matrix, flags);
}

as_value
colormatrixfilter_matrix(const fn_call& fn)
{
    ColorMatrixFilter_as* ptr = ensure<ThisIsNative<ColorMatrixFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
colormatrixfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new ColorMatrixFilter_as);
    return as_value();
}

}
}

