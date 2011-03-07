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

#include "DropShadowFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    as_value dropshadowfilter_new(const fn_call& fn);
    as_value dropshadowfilter_distance(const fn_call& fn);
    as_value dropshadowfilter_alpha(const fn_call& fn);
    as_value dropshadowfilter_color(const fn_call& fn);
    as_value dropshadowfilter_angle(const fn_call& fn);
    as_value dropshadowfilter_blurX(const fn_call& fn);
    as_value dropshadowfilter_blurY(const fn_call& fn);
    as_value dropshadowfilter_strength(const fn_call& fn);
    as_value dropshadowfilter_quality(const fn_call& fn);
    as_value dropshadowfilter_knockout(const fn_call& fn);
    as_value dropshadowfilter_hideObject(const fn_call& fn);
    as_value dropshadowfilter_inner(const fn_call& fn);

    void attachDropShadowFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class DropShadowFilter_as : public Relay, public DropShadowFilter
{
public:
    DropShadowFilter_as() {}
};

/// The prototype of flash.filters.DropShadowFilter is a new BitmapFilter.
void
dropshadowfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, dropshadowfilter_new,
            attachDropShadowFilterInterface, uri);
}

namespace {

void
attachDropShadowFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("color", dropshadowfilter_color, 
        dropshadowfilter_color, flags);
    o.init_property("alpha", dropshadowfilter_alpha, 
        dropshadowfilter_alpha, flags);
    o.init_property("inner", dropshadowfilter_inner, 
        dropshadowfilter_inner, flags);
    o.init_property("hideObject", dropshadowfilter_hideObject, 
        dropshadowfilter_hideObject, flags);
    o.init_property("distance", dropshadowfilter_distance, 
        dropshadowfilter_distance, flags);
    o.init_property("angle", dropshadowfilter_angle, 
        dropshadowfilter_angle, flags);
    o.init_property("blurX", dropshadowfilter_blurX, 
        dropshadowfilter_blurX, flags);
    o.init_property("blurY", dropshadowfilter_blurY, 
        dropshadowfilter_blurY, flags);
    o.init_property("strength", dropshadowfilter_strength, 
        dropshadowfilter_strength, flags);
    o.init_property("quality", dropshadowfilter_quality, 
        dropshadowfilter_quality, flags);
    o.init_property("knockout", dropshadowfilter_knockout, 
        dropshadowfilter_knockout, flags);

}

as_value
dropshadowfilter_distance(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = toNumber(fn.arg(0), getVM(fn));
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
dropshadowfilter_color(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_color);
    }
    double sp_color = toNumber(fn.arg(0), getVM(fn));
    ptr->m_color = sp_color;
    return as_value();
}

as_value
dropshadowfilter_alpha(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_alpha);
    }
    double sp_alpha = toNumber(fn.arg(0), getVM(fn));
    ptr->m_alpha = sp_alpha;
    return as_value();
}

as_value
dropshadowfilter_angle(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle);
    }
    double sp_angle = toNumber(fn.arg(0), getVM(fn));
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
dropshadowfilter_blurX(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
dropshadowfilter_blurY(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
dropshadowfilter_strength(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = toNumber(fn.arg(0), getVM(fn));
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
dropshadowfilter_quality(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
dropshadowfilter_knockout(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    const bool sp_knockout = toBool(fn.arg(0), getVM(fn));
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
dropshadowfilter_inner(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_inner );
    }
    const bool sp_inner = toBool(fn.arg(0), getVM(fn));
    ptr->m_inner = sp_inner;
    return as_value();
}


as_value
dropshadowfilter_hideObject(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensure<ThisIsNative<DropShadowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_hideObject );
    }
    const bool sp_hideObject = toBool(fn.arg(0), getVM(fn));
    ptr->m_hideObject = sp_hideObject;
    return as_value();
}

as_value
dropshadowfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new DropShadowFilter_as);
    return as_value();
}

}
}


