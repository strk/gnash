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
#include "DropShadowFilter.h"
#include "VM.h"
#include "Global_as.h"
#include "builtin_function.h"

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

    as_object* cl = gl->createClass(dropshadowfilter_new, proto);
    attachDropShadowFilterInterface(*proto);
    where.init_member(getName(uri) , cl, as_object::DefaultFlags,
            getNamespace(uri));
}

namespace {

void
attachDropShadowFilterInterface(as_object& o)
{
    o.init_property("color", dropshadowfilter_color, 
        dropshadowfilter_color);
    o.init_property("alpha", dropshadowfilter_alpha, 
        dropshadowfilter_alpha);
    o.init_property("inner", dropshadowfilter_inner, 
        dropshadowfilter_inner);
    o.init_property("hideObject", dropshadowfilter_hideObject, 
        dropshadowfilter_hideObject);
    o.init_property("distance", dropshadowfilter_distance, 
        dropshadowfilter_distance);
    o.init_property("angle", dropshadowfilter_angle, 
        dropshadowfilter_angle);
    o.init_property("blurX", dropshadowfilter_blurX, 
        dropshadowfilter_blurX);
    o.init_property("blurY", dropshadowfilter_blurY, 
        dropshadowfilter_blurY);
    o.init_property("strength", dropshadowfilter_strength, 
        dropshadowfilter_strength);
    o.init_property("quality", dropshadowfilter_quality, 
        dropshadowfilter_quality);
    o.init_property("knockout", dropshadowfilter_knockout, 
        dropshadowfilter_knockout);

}

as_value
dropshadowfilter_distance(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = fn.arg(0).to_number();
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
dropshadowfilter_color(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_color);
    }
    double sp_color = fn.arg(0).to_number();
    ptr->m_color = sp_color;
    return as_value();
}

as_value
dropshadowfilter_alpha(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_alpha);
    }
    double sp_alpha = fn.arg(0).to_number();
    ptr->m_alpha = sp_alpha;
    return as_value();
}

as_value
dropshadowfilter_angle(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle);
    }
    double sp_angle = fn.arg(0).to_number();
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
dropshadowfilter_blurX(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
dropshadowfilter_blurY(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
dropshadowfilter_strength(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = fn.arg(0).to_number<float> ();
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
dropshadowfilter_quality(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
dropshadowfilter_knockout(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    bool sp_knockout = fn.arg(0).to_bool ();
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
dropshadowfilter_inner(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_inner );
    }
    bool sp_inner = fn.arg(0).to_bool ();
    ptr->m_inner = sp_inner;
    return as_value();
}


as_value
dropshadowfilter_hideObject(const fn_call& fn)
{
    DropShadowFilter_as* ptr = ensureNativeType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_hideObject );
    }
    bool sp_hideObject = fn.arg(0).to_bool ();
    ptr->m_hideObject = sp_hideObject;
    return as_value();
}

as_value
dropshadowfilter_new(const fn_call& fn)
{
    boost::intrusive_ptr<as_object> obj = ensureType<as_object>(fn.this_ptr);
    obj->setRelay(new DropShadowFilter_as);
    return as_value();
}

}
}


