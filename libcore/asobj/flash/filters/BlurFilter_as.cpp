// 
//   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include "BlurFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    as_value blurfilter_new(const fn_call& fn);
    as_value blurfilter_blurX(const fn_call& fn);
    as_value blurfilter_blurY(const fn_call& fn);
    as_value blurfilter_quality(const fn_call& fn);

    void attachBlurFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class BlurFilter_as : public Relay, public BlurFilter
{
public:
    BlurFilter_as() {}
};

void
blurfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, blurfilter_new, attachBlurFilterInterface,
            uri);
}

namespace {

void
attachBlurFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("blurX", blurfilter_blurX, blurfilter_blurX, flags);
    o.init_property("blurY", blurfilter_blurY, blurfilter_blurY, flags);
    o.init_property("quality", blurfilter_quality, blurfilter_quality, flags);
}

as_value
blurfilter_blurX(const fn_call& fn)
{
    BlurFilter_as* ptr = ensure<ThisIsNative<BlurFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
blurfilter_blurY(const fn_call& fn)
{
    BlurFilter_as* ptr = ensure<ThisIsNative<BlurFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
blurfilter_quality(const fn_call& fn)
{
    BlurFilter_as* ptr = ensure<ThisIsNative<BlurFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
blurfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new BlurFilter_as);
    return as_value();
}

}
}

