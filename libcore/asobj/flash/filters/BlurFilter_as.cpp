// 
//   Copyright (C) 2007, 2008, 2009 Free Software Foundation, Inc.
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


#include "as_object.h"
#include "BlurFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "Global_as.h"

#include "BitmapFilter_as.h"

namespace gnash {

namespace {
    as_value blurfilter_new(const fn_call& fn);
    as_value blurfilter_distance(const fn_call& fn);
    as_value blurfilter_angle(const fn_call& fn);
    as_value blurfilter_highlightColor(const fn_call& fn);
    as_value blurfilter_highlightAlpha(const fn_call& fn);
    as_value blurfilter_shadowColor(const fn_call& fn);
    as_value blurfilter_shadowAlpha(const fn_call& fn);
    as_value blurfilter_blurX(const fn_call& fn);
    as_value blurfilter_blurY(const fn_call& fn);
    as_value blurfilter_strength(const fn_call& fn);
    as_value blurfilter_quality(const fn_call& fn);
    as_value blurfilter_type(const fn_call& fn);
    as_value blurfilter_knockout(const fn_call& fn);

    void attachBlurFilterInterface(as_object& o);
}

/// TODO: should this inherit from BitmapFilter_as (relay)? This might
/// make cloning easier, but needs some testing first.
class BlurFilter_as : public Relay, public BlurFilter
{
public:
    BlurFilter_as() {}
};

/// The prototype of flash.filters.BlurFilter is a new BitmapFilter.
void
blurfilter_class_init(as_object& where, const ObjectURI& uri)
{
    Global_as* gl = getGlobal(where);
    string_table& st = getStringTable(where);

    as_function* ctor =
        gl->getMember(st.find("flash.filters.BitmapFilter")).to_as_function();
    
    as_object* proto;
    if (ctor) {
        fn_call::Args args;
        VM& vm = getVM(where);
        proto = ctor->constructInstance(as_environment(vm), args).get();
    }
    else proto = 0;

    as_object* cl = gl->createClass(blurfilter_new, proto);
    attachBlurFilterInterface(*proto);
    where.init_member(getName(uri) , cl, as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachBlurFilterInterface(as_object& o)
{
    o.init_property("blurX", blurfilter_blurX, blurfilter_blurX);
    o.init_property("blurY", blurfilter_blurY, blurfilter_blurY);
    o.init_property("quality", blurfilter_quality, blurfilter_quality);
}

as_value
blurfilter_blurX(const fn_call& fn)
{
    BlurFilter_as* ptr = ensureNativeType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
blurfilter_blurY(const fn_call& fn)
{
    BlurFilter_as* ptr = ensureNativeType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
blurfilter_quality(const fn_call& fn)
{
    BlurFilter_as* ptr = ensureNativeType<BlurFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
blurfilter_new(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    obj->setRelay(new BlurFilter_as);
    return as_value();
}

}
}

