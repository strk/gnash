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

#include <boost/intrusive_ptr.hpp>

#include "GradientBevelFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"

namespace gnash {

namespace {
    as_value gradientbevelfilter_new(const fn_call& fn);
    as_value gradientbevelfilter_distance(const fn_call& fn);
    as_value gradientbevelfilter_angle(const fn_call& fn);
    as_value gradientbevelfilter_alphas(const fn_call& fn);
    as_value gradientbevelfilter_colors(const fn_call& fn);
    as_value gradientbevelfilter_ratios(const fn_call& fn);
    as_value gradientbevelfilter_blurX(const fn_call& fn);
    as_value gradientbevelfilter_blurY(const fn_call& fn);
    as_value gradientbevelfilter_strength(const fn_call& fn);
    as_value gradientbevelfilter_quality(const fn_call& fn);
    as_value gradientbevelfilter_type(const fn_call& fn);
    as_value gradientbevelfilter_knockout(const fn_call& fn);

    void attachGradientBevelFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class GradientBevelFilter_as : public Relay, public GradientBevelFilter
{
public:
    GradientBevelFilter_as() {}
};

/// The prototype of flash.filters.GradientBevelFilter is a new BitmapFilter.
void
gradientbevelfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, gradientbevelfilter_new,
            attachGradientBevelFilterInterface, uri);
}

namespace {

void
attachGradientBevelFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("distance", gradientbevelfilter_distance, 
        gradientbevelfilter_distance, flags);
    o.init_property("angle", gradientbevelfilter_angle, 
        gradientbevelfilter_angle, flags);
    o.init_property("alphas", gradientbevelfilter_alphas, 
        gradientbevelfilter_alphas, flags);
    o.init_property("colors", gradientbevelfilter_colors, 
        gradientbevelfilter_colors, flags);
    o.init_property("ratios", gradientbevelfilter_ratios, 
        gradientbevelfilter_ratios, flags);
    o.init_property("blurX", gradientbevelfilter_blurX, 
        gradientbevelfilter_blurX, flags);
    o.init_property("blurY", gradientbevelfilter_blurY, 
        gradientbevelfilter_blurY, flags);
    o.init_property("strength", gradientbevelfilter_strength, 
        gradientbevelfilter_strength, flags);
    o.init_property("quality", gradientbevelfilter_quality, 
        gradientbevelfilter_quality, flags);
    o.init_property("type", gradientbevelfilter_type, 
        gradientbevelfilter_type, flags);
    o.init_property("knockout", gradientbevelfilter_knockout, 
        gradientbevelfilter_knockout, flags);

}

as_value
gradientbevelfilter_distance(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = toNumber(fn.arg(0), getVM(fn));
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
gradientbevelfilter_angle(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle);
    }
    double sp_angle = toNumber(fn.arg(0), getVM(fn));
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
gradientbevelfilter_alphas(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
gradientbevelfilter_colors(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
gradientbevelfilter_ratios(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    UNUSED(ptr);
    return as_value();
}

as_value
gradientbevelfilter_blurX(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
gradientbevelfilter_blurY(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
gradientbevelfilter_strength(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = toNumber(fn.arg(0), getVM(fn));
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
gradientbevelfilter_quality(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
gradientbevelfilter_knockout(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    const bool sp_knockout = toBool(fn.arg(0), getVM(fn));
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
gradientbevelfilter_type(const fn_call& fn)
{
    GradientBevelFilter_as* ptr = ensure<ThisIsNative<GradientBevelFilter_as> >(fn);

    if (fn.nargs == 0)
    {
        switch (ptr->m_type)
        {
            case GradientBevelFilter::FULL_BEVEL:
                return as_value("full");

                break;

            default:
            case GradientBevelFilter::INNER_BEVEL:
                return as_value("inner");

                break;

            case GradientBevelFilter::OUTER_BEVEL:
                return as_value("outer");

                break;

        }
    }

    std::string type = fn.arg(0).to_string();

    if (type == "outer")
        ptr->m_type = GradientBevelFilter::OUTER_BEVEL;

    if (type == "inner")
        ptr->m_type = GradientBevelFilter::INNER_BEVEL;

    if (type == "full")
        ptr->m_type = GradientBevelFilter::FULL_BEVEL;


    return as_value();

}

as_value
gradientbevelfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new GradientBevelFilter_as);
    return as_value();
}

}
}

