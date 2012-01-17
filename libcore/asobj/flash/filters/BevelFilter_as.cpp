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

#include "BevelFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    as_value bevelfilter_new(const fn_call& fn);
    as_value bevelfilter_distance(const fn_call& fn);
    as_value bevelfilter_angle(const fn_call& fn);
    as_value bevelfilter_highlightColor(const fn_call& fn);
    as_value bevelfilter_highlightAlpha(const fn_call& fn);
    as_value bevelfilter_shadowColor(const fn_call& fn);
    as_value bevelfilter_shadowAlpha(const fn_call& fn);
    as_value bevelfilter_blurX(const fn_call& fn);
    as_value bevelfilter_blurY(const fn_call& fn);
    as_value bevelfilter_strength(const fn_call& fn);
    as_value bevelfilter_quality(const fn_call& fn);
    as_value bevelfilter_type(const fn_call& fn);
    as_value bevelfilter_knockout(const fn_call& fn);

    void attachBevelFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class BevelFilter_as : public Relay, public BevelFilter
{
public:
    BevelFilter_as() {}
};

/// The prototype of flash.filters.BevelFilter is a new BitmapFilter.
void
bevelfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, bevelfilter_new, attachBevelFilterInterface,
            uri);
}

namespace {

void
attachBevelFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("distance", bevelfilter_distance, 
        bevelfilter_distance, flags);
    o.init_property("angle", bevelfilter_angle, 
        bevelfilter_angle, flags);
    o.init_property("highlightColor", bevelfilter_highlightColor, 
        bevelfilter_highlightColor, flags);
    o.init_property("highlightAlpha", bevelfilter_highlightAlpha, 
        bevelfilter_highlightAlpha, flags);
    o.init_property("shadowColor", bevelfilter_shadowColor, 
        bevelfilter_shadowColor, flags);
    o.init_property("shadowAlpha", bevelfilter_shadowAlpha, 
        bevelfilter_shadowAlpha, flags);
    o.init_property("blurX", bevelfilter_blurX, 
        bevelfilter_blurX, flags);
    o.init_property("blurY", bevelfilter_blurY, 
        bevelfilter_blurY, flags);
    o.init_property("strength", bevelfilter_strength, 
        bevelfilter_strength, flags);
    o.init_property("quality", bevelfilter_quality, 
        bevelfilter_quality, flags);
    o.init_property("type", bevelfilter_type, 
        bevelfilter_type, flags);
    o.init_property("knockout", bevelfilter_knockout, 
        bevelfilter_knockout, flags);

}

as_value
bevelfilter_distance(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = toNumber(fn.arg(0), getVM(fn));
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
bevelfilter_angle(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle);
    }
    double sp_angle = toNumber(fn.arg(0), getVM(fn));
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
bevelfilter_highlightColor(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_highlightColor );
    }
    boost::uint32_t sp_highlightColor = toNumber(fn.arg(0), getVM(fn));
    ptr->m_highlightColor = sp_highlightColor;
    return as_value();
}

as_value
bevelfilter_highlightAlpha(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_highlightAlpha );
    }
    boost::uint8_t sp_highlightAlpha = toNumber(fn.arg(0), getVM(fn));
    ptr->m_highlightAlpha = sp_highlightAlpha;
    return as_value();
}

as_value
bevelfilter_shadowColor(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_shadowColor );
    }
    boost::uint32_t sp_shadowColor = toNumber(fn.arg(0), getVM(fn));
    ptr->m_shadowColor = sp_shadowColor;
    return as_value();
}

as_value
bevelfilter_shadowAlpha(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_shadowAlpha );
    }
    boost::uint8_t sp_shadowAlpha = toNumber(fn.arg(0), getVM(fn));
    ptr->m_shadowAlpha = sp_shadowAlpha;
    return as_value();
}

as_value
bevelfilter_blurX(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
bevelfilter_blurY(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
bevelfilter_strength(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = toNumber(fn.arg(0), getVM(fn));
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
bevelfilter_quality(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
bevelfilter_knockout(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    const bool sp_knockout = toBool(fn.arg(0), getVM(fn));
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
bevelfilter_type(const fn_call& fn)
{
    BevelFilter_as* ptr = ensure<ThisIsNative<BevelFilter_as> >(fn);

    if (fn.nargs == 0)
    {
        switch (ptr->m_type)
        {
            case BevelFilter::FULL_BEVEL:
                return as_value("full");

                break;

            default:
            case BevelFilter::INNER_BEVEL:
                return as_value("inner");

                break;

            case BevelFilter::OUTER_BEVEL:
                return as_value("outer");

                break;

        }
    }

    std::string type = fn.arg(0).to_string();

    if (type == "outer")
        ptr->m_type = BevelFilter::OUTER_BEVEL;

    if (type == "inner")
        ptr->m_type = BevelFilter::INNER_BEVEL;

    if (type == "full")
        ptr->m_type = BevelFilter::FULL_BEVEL;


    return as_value();

}

as_value
bevelfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new BevelFilter_as);
    return as_value();
}

}
}
