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

#include "GradientGlowFilter_as.h"

#include "Filters.h"
#include "BitmapFilter_as.h"
#include "as_object.h"
#include "VM.h"
#include "Global_as.h"

namespace gnash {

namespace {
    as_value gradientglowfilter_new(const fn_call& fn);
    as_value gradientglowfilter_distance(const fn_call& fn);
    as_value gradientglowfilter_angle(const fn_call& fn);
    as_value gradientglowfilter_colors(const fn_call& fn);
    as_value gradientglowfilter_alphas(const fn_call& fn);
    as_value gradientglowfilter_ratios(const fn_call& fn);
    as_value gradientglowfilter_blurX(const fn_call& fn);
    as_value gradientglowfilter_blurY(const fn_call& fn);
    as_value gradientglowfilter_strength(const fn_call& fn);
    as_value gradientglowfilter_quality(const fn_call& fn);
    as_value gradientglowfilter_type(const fn_call& fn);
    as_value gradientglowfilter_knockout(const fn_call& fn);

    void attachGradientGlowFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class GradientGlowFilter_as : public Relay, public GradientGlowFilter
{
public:
    GradientGlowFilter_as() {}
};

/// The prototype of flash.filters.GradientGlowFilter is a new BitmapFilter.
void
gradientglowfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, gradientglowfilter_new,
            attachGradientGlowFilterInterface, uri);
}

namespace {

void
attachGradientGlowFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("distance", gradientglowfilter_distance, 
        gradientglowfilter_distance, flags);
    o.init_property("angle", gradientglowfilter_angle, 
        gradientglowfilter_angle, flags);
    o.init_property("colors", gradientglowfilter_colors, 
        gradientglowfilter_colors, flags);
    o.init_property("alphas", gradientglowfilter_alphas, 
        gradientglowfilter_alphas, flags);
    o.init_property("ratios", gradientglowfilter_ratios, 
        gradientglowfilter_ratios, flags);
    o.init_property("blurX", gradientglowfilter_blurX, 
        gradientglowfilter_blurX, flags);
    o.init_property("blurY", gradientglowfilter_blurY, 
        gradientglowfilter_blurY, flags);
    o.init_property("strength", gradientglowfilter_strength, 
        gradientglowfilter_strength, flags);
    o.init_property("quality", gradientglowfilter_quality, 
        gradientglowfilter_quality, flags);
    o.init_property("type", gradientglowfilter_type, 
        gradientglowfilter_type, flags);
    o.init_property("knockout", gradientglowfilter_knockout, 
        gradientglowfilter_knockout, flags);

}

as_value
gradientglowfilter_distance(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = toNumber(fn.arg(0), getVM(fn));
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
gradientglowfilter_angle(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle);
    }
    double sp_angle = toNumber(fn.arg(0), getVM(fn));
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
gradientglowfilter_colors(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);

    UNUSED(ptr);
    return as_value();
}

as_value
gradientglowfilter_alphas(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
gradientglowfilter_ratios(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
gradientglowfilter_blurX(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
gradientglowfilter_blurY(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
gradientglowfilter_strength(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = toNumber(fn.arg(0), getVM(fn));
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
gradientglowfilter_quality(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
gradientglowfilter_knockout(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    const bool sp_knockout = toBool(fn.arg(0), getVM(fn));
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
gradientglowfilter_type(const fn_call& fn)
{
    GradientGlowFilter_as* ptr = ensure<ThisIsNative<GradientGlowFilter_as> >(fn);

    if (fn.nargs == 0)
    {
        switch (ptr->m_type)
        {
            case GradientGlowFilter::FULL_GLOW:
                return as_value("full");

                break;

            default:
            case GradientGlowFilter::INNER_GLOW:
                return as_value("inner");

                break;

            case GradientGlowFilter::OUTER_GLOW:
                return as_value("outer");

                break;

        }
    }

    std::string type = fn.arg(0).to_string();

    if (type == "outer")
        ptr->m_type = GradientGlowFilter::OUTER_GLOW;

    if (type == "inner")
        ptr->m_type = GradientGlowFilter::INNER_GLOW;

    if (type == "full")
        ptr->m_type = GradientGlowFilter::FULL_GLOW;


    return as_value();

}

as_value
gradientglowfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new GradientGlowFilter_as);
    return as_value();
}

}
}

