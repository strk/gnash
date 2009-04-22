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
#include "GradientGlowFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "BitmapFilter_as.h"

namespace gnash {

class GradientGlowFilter_as : public as_object, public GradientGlowFilter
{
public:
    static as_value distance_gs(const fn_call& fn);
    static as_value angle_gs(const fn_call& fn);
    static as_value colors_gs(const fn_call& fn);
    static as_value alphas_gs(const fn_call& fn);
    static as_value ratios_gs(const fn_call& fn);
    static as_value blurX_gs(const fn_call& fn);
    static as_value blurY_gs(const fn_call& fn);
    static as_value strength_gs(const fn_call& fn);
    static as_value quality_gs(const fn_call& fn);
    static as_value type_gs(const fn_call& fn);
    static as_value knockout_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);

    GradientGlowFilter_as(as_object *obj)
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
    static boost::intrusive_ptr<builtin_function> s_ctor;

};


boost::intrusive_ptr<as_object> GradientGlowFilter_as::s_interface;

boost:: intrusive_ptr<builtin_function> GradientGlowFilter_as::s_ctor;

as_object*
GradientGlowFilter_as::Interface()
{
    if (GradientGlowFilter_as::s_interface == NULL) {
        GradientGlowFilter_as::s_interface = new as_object (getBitmapFilterInterface());
        VM::get().addStatic(GradientGlowFilter_as::s_interface.get());
        GradientGlowFilter_as::attachInterface(*GradientGlowFilter_as::s_interface);
    }
    return GradientGlowFilter_as::s_interface.get();
}

void
GradientGlowFilter_as::registerCtor(as_object& global)
{
    if (GradientGlowFilter_as::s_ctor != NULL) return;
    GradientGlowFilter_as::s_ctor = new builtin_function(&GradientGlowFilter_as::ctor, GradientGlowFilter_as::Interface());
    VM::get().addStatic(GradientGlowFilter_as::s_ctor.get());
    GradientGlowFilter_as::attachInterface(*GradientGlowFilter_as::s_ctor);
    global.init_member("GradientGlowFilter" , GradientGlowFilter_as::s_ctor.get());
}

void
GradientGlowFilter_class_init(as_object& global)
{
    GradientGlowFilter_as::registerCtor(global);
}


void
GradientGlowFilter_as::attachInterface(as_object& o)
{
	boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), new builtin_function(bitmap_clone));

}


void
GradientGlowFilter_as::attachProperties(as_object& o)
{
	boost::intrusive_ptr<builtin_function> gs;

    gs = new builtin_function(GradientGlowFilter_as::distance_gs, NULL);
    o.init_property("distance" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::angle_gs, NULL);
    o.init_property("angle" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::colors_gs, NULL);
    o.init_property("colors" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::alphas_gs, NULL);
    o.init_property("alphas" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::ratios_gs, NULL);
    o.init_property("ratios" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::blurX_gs, NULL);
    o.init_property("blurX" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::blurY_gs, NULL);
    o.init_property("blurY" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::strength_gs, NULL);
    o.init_property("strength" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::quality_gs, NULL);
    o.init_property("quality" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::type_gs, NULL);
    o.init_property("type" , *gs, *gs);

    gs = new builtin_function(GradientGlowFilter_as::knockout_gs, NULL);
    o.init_property("knockout" , *gs, *gs);

}

as_value
GradientGlowFilter_as::distance_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_distance );
    }
    float sp_distance = fn.arg(0).to_number<float> ();
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
GradientGlowFilter_as::angle_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_angle );
    }
    float sp_angle = fn.arg(0).to_number<float> ();
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
GradientGlowFilter_as::colors_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    return as_value();
}


as_value
GradientGlowFilter_as::alphas_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    return as_value();
}

as_value
GradientGlowFilter_as::ratios_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    return as_value();
}

as_value
GradientGlowFilter_as::blurX_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}


as_value
GradientGlowFilter_as::blurY_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
GradientGlowFilter_as::strength_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_strength );
    }
    float sp_strength = fn.arg(0).to_number<float> ();
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
GradientGlowFilter_as::quality_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
GradientGlowFilter_as::knockout_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    bool sp_knockout = fn.arg(0).to_bool ();
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
GradientGlowFilter_as::bitmap_clone(const fn_call& fn)
{
	boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<GradientGlowFilter_as> obj = new GradientGlowFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
GradientGlowFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);

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
GradientGlowFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new GradientGlowFilter_as(GradientGlowFilter_as::Interface());

    GradientGlowFilter_as::attachProperties(*obj);
    return as_value(obj.get());

}

}
