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

#include "GlowFilter_as.h"

#include "as_object.h"
#include "VM.h"
#include "Global_as.h"
#include "BitmapFilter_as.h"
#include "Filters.h"
#include "smart_ptr.h"

namespace gnash {

namespace {
    as_value glowfilter_new(const fn_call& fn);
    as_value glowfilter_color(const fn_call& fn);
    as_value glowfilter_alpha(const fn_call& fn);
    as_value glowfilter_inner(const fn_call& fn);
    as_value glowfilter_blurX(const fn_call& fn);
    as_value glowfilter_blurY(const fn_call& fn);
    as_value glowfilter_strength(const fn_call& fn);
    as_value glowfilter_quality(const fn_call& fn);
    as_value glowfilter_knockout(const fn_call& fn);

    void attachGlowFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class GlowFilter_as : public Relay, public GlowFilter
{
public:
    GlowFilter_as() {}
};

/// The prototype of flash.filters.GlowFilter is a new BitmapFilter.
void
glowfilter_class_init(as_object& where, const ObjectURI& uri)
{
    registerBitmapClass(where, glowfilter_new, attachGlowFilterInterface,
            uri);
}

namespace {

void
attachGlowFilterInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;
    o.init_property("color", glowfilter_color, 
        glowfilter_color, flags);
    o.init_property("alpha", glowfilter_alpha, 
        glowfilter_alpha, flags);
    o.init_property("inner", glowfilter_inner, 
        glowfilter_inner, flags);
    o.init_property("blurX", glowfilter_blurX, 
        glowfilter_blurX, flags);
    o.init_property("blurY", glowfilter_blurY, 
        glowfilter_blurY, flags);
    o.init_property("strength", glowfilter_strength, 
        glowfilter_strength, flags);
    o.init_property("quality", glowfilter_quality, 
        glowfilter_quality, flags);
    o.init_property("knockout", glowfilter_knockout, 
        glowfilter_knockout, flags);

}

as_value
glowfilter_inner(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_inner );
    }
    boost::uint32_t sp_inner = toNumber(fn.arg(0), getVM(fn));
    ptr->m_inner = sp_inner;
    return as_value();
}

as_value
glowfilter_color(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_color );
    }
    float sp_color = toNumber(fn.arg(0), getVM(fn));
    ptr->m_color = sp_color;
    return as_value();
}

as_value
glowfilter_alpha(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_alpha );
    }
    float sp_alpha = toNumber(fn.arg(0), getVM(fn));
    ptr->m_alpha = sp_alpha;
    return as_value();
}

as_value
glowfilter_blurX(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
glowfilter_blurY(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = toNumber(fn.arg(0), getVM(fn));
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
glowfilter_strength(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = toNumber(fn.arg(0), getVM(fn));
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
glowfilter_quality(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = toNumber(fn.arg(0), getVM(fn));
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
glowfilter_knockout(const fn_call& fn)
{
    GlowFilter_as* ptr = ensure<ThisIsNative<GlowFilter_as> >(fn);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    const bool sp_knockout = toBool(fn.arg(0), getVM(fn));
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
glowfilter_new(const fn_call& fn)
{
    as_object* obj = ensure<ValidThis>(fn);
    obj->setRelay(new GlowFilter_as);
    return as_value();
}

}
}

