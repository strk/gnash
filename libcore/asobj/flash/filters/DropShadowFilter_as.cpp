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
#include "BitmapFilter_as.h"

namespace gnash {

class DropShadowFilter_as : public as_object, public DropShadowFilter
{
public:
    static as_value distance_gs(const fn_call& fn);
    static as_value angle_gs(const fn_call& fn);
    static as_value color_gs(const fn_call& fn);
    static as_value alpha_gs(const fn_call& fn);
    static as_value blurX_gs(const fn_call& fn);
    static as_value blurY_gs(const fn_call& fn);
    static as_value strength_gs(const fn_call& fn);
    static as_value quality_gs(const fn_call& fn);
    static as_value inner_gs(const fn_call& fn);
    static as_value knockout_gs(const fn_call& fn);
    static as_value hideObject_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);


    DropShadowFilter_as(as_object *obj)
        :
        as_object(obj)
    {}
    
    static as_object* Interface();
    static void attachInterface(as_object& o);
    static void attachProperties(as_object& o);
    static void registerCtor(as_object& global);
    static as_value ctor(const fn_call& fn);
    
private:
    static boost::intrusive_ptr<as_object> s_interface;
    static boost::intrusive_ptr<as_object> s_ctor;

};


boost::intrusive_ptr<as_object> DropShadowFilter_as::s_interface;

boost:: intrusive_ptr<as_object> DropShadowFilter_as::s_ctor;

as_object*
DropShadowFilter_as::Interface() {
    if (DropShadowFilter_as::s_interface == NULL) {
        DropShadowFilter_as::s_interface = new as_object (getBitmapFilterInterface());
    VM::get().addStatic(DropShadowFilter_as::s_interface.get());
    DropShadowFilter_as::attachInterface(*DropShadowFilter_as::s_interface);
    }
    return DropShadowFilter_as::s_interface.get();
}

void
DropShadowFilter_as::registerCtor(as_object& global) {
    if (DropShadowFilter_as::s_ctor != NULL) return;
    Global_as* gl = getGlobal(global);
    DropShadowFilter_as::s_ctor = gl->createClass(&DropShadowFilter_as::ctor, DropShadowFilter_as::Interface());
    VM::get().addStatic(DropShadowFilter_as::s_ctor.get());
    DropShadowFilter_as::attachInterface(*DropShadowFilter_as::s_ctor);
    global.init_member("DropShadowFilter" , DropShadowFilter_as::s_ctor.get());
}

void
dropshadowfilter_class_init(as_object& global)
{
    DropShadowFilter_as::registerCtor(global);
}


void DropShadowFilter_as::attachInterface(as_object& o) {
    Global_as* gl = getGlobal(o);
    boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), gl->createFunction(bitmap_clone));

}

void
DropShadowFilter_as::attachProperties(as_object& o)
{
    boost::intrusive_ptr<builtin_function> gs;
    o.init_property("distance" , DropShadowFilter_as::distance_gs, 
        DropShadowFilter_as::distance_gs);
    o.init_property("angle" , DropShadowFilter_as::angle_gs, 
        DropShadowFilter_as::angle_gs);
    o.init_property("color" , DropShadowFilter_as::color_gs, 
        DropShadowFilter_as::color_gs);
    o.init_property("alpha" , DropShadowFilter_as::alpha_gs, 
        DropShadowFilter_as::alpha_gs);
    o.init_property("blurX" , DropShadowFilter_as::blurX_gs, 
        DropShadowFilter_as::blurX_gs);
    o.init_property("blurY" , DropShadowFilter_as::blurY_gs, 
        DropShadowFilter_as::blurY_gs);
    o.init_property("strength" , DropShadowFilter_as::strength_gs, 
        DropShadowFilter_as::strength_gs);
    o.init_property("quality" , DropShadowFilter_as::quality_gs, 
        DropShadowFilter_as::quality_gs);
    o.init_property("inner" , DropShadowFilter_as::inner_gs, 
        DropShadowFilter_as::inner_gs);
    o.init_property("knockout" , DropShadowFilter_as::knockout_gs, 
        DropShadowFilter_as::knockout_gs);
    o.init_property("hideObject" , DropShadowFilter_as::hideObject_gs, 
        DropShadowFilter_as::hideObject_gs);

}

as_value
DropShadowFilter_as::distance_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_distance );
    }
    float sp_distance = fn.arg(0).to_number<float> ();
    ptr->m_distance = sp_distance;
    return as_value();
}


as_value
DropShadowFilter_as::angle_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_angle );
    }
    float sp_angle = fn.arg(0).to_number<float> ();
    ptr->m_angle = sp_angle;
    return as_value();
}


as_value
DropShadowFilter_as::color_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_color );
    }
    boost::uint32_t sp_color = fn.arg(0).to_number<boost::uint32_t> ();
    ptr->m_color = sp_color;
    return as_value();
}


as_value
DropShadowFilter_as::alpha_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_alpha );
    }
    boost::uint8_t sp_alpha = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_alpha = sp_alpha;
    return as_value();
}


as_value
DropShadowFilter_as::blurX_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}


as_value
DropShadowFilter_as::blurY_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}


as_value
DropShadowFilter_as::strength_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_strength );
    }
    float sp_strength = fn.arg(0).to_number<float> ();
    ptr->m_strength = sp_strength;
    return as_value();
}


as_value
DropShadowFilter_as::quality_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}


as_value
DropShadowFilter_as::inner_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_inner );
    }
    bool sp_inner = fn.arg(0).to_bool ();
    ptr->m_inner = sp_inner;
    return as_value();
}


as_value
DropShadowFilter_as::knockout_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    bool sp_knockout = fn.arg(0).to_bool ();
    ptr->m_knockout = sp_knockout;
    return as_value();
}


as_value
DropShadowFilter_as::hideObject_gs(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_hideObject );
    }
    bool sp_hideObject = fn.arg(0).to_bool ();
    ptr->m_hideObject = sp_hideObject;
    return as_value();
}

as_value
DropShadowFilter_as::bitmap_clone(const fn_call& fn)
{
    boost::intrusive_ptr<DropShadowFilter_as> ptr = ensureType<DropShadowFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<DropShadowFilter_as> obj = new DropShadowFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
 }

as_value
DropShadowFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new DropShadowFilter_as(DropShadowFilter_as::Interface());

    DropShadowFilter_as::attachProperties(*obj);


    return as_value(obj.get());

}

}
