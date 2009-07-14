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
#include "GradientBevelFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "BitmapFilter_as.h"
#include "Global_as.h"

namespace gnash {

class GradientBevelFilter_as : public as_object, public GradientBevelFilter
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


    GradientBevelFilter_as(as_object *obj)
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


boost::intrusive_ptr<as_object> GradientBevelFilter_as::s_interface;
boost::intrusive_ptr<as_object> GradientBevelFilter_as::s_ctor;

as_object*
GradientBevelFilter_as::Interface() {
    if (GradientBevelFilter_as::s_interface == NULL) {
        GradientBevelFilter_as::s_interface = new as_object (getBitmapFilterInterface());
        VM::get().addStatic(GradientBevelFilter_as::s_interface.get());
        GradientBevelFilter_as::attachInterface(*GradientBevelFilter_as::s_interface);
    }

    return GradientBevelFilter_as::s_interface.get();
}

void
GradientBevelFilter_as::registerCtor(as_object& global) {
    if (GradientBevelFilter_as::s_ctor != NULL) return;
        Global_as* gl = getGlobal(global);
	GradientBevelFilter_as::s_ctor = gl->createClass(&GradientBevelFilter_as::ctor, GradientBevelFilter_as::Interface());
    VM::get().addStatic(GradientBevelFilter_as::s_ctor.get());
    GradientBevelFilter_as::attachInterface(*GradientBevelFilter_as::s_ctor);
    global.init_member("GradientBevelFilter" , GradientBevelFilter_as::s_ctor.get());
}

void
gradientbevelfilter_class_init(as_object& global)
{
    GradientBevelFilter_as::registerCtor(global);
}


void
GradientBevelFilter_as::attachInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
	boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), gl->createFunction(bitmap_clone));

}


void
GradientBevelFilter_as::attachProperties(as_object& o) {
    o.init_property("distance" , GradientBevelFilter_as::distance_gs, 
        GradientBevelFilter_as::distance_gs);

    o.init_property("angle" , GradientBevelFilter_as::angle_gs, 
        GradientBevelFilter_as::angle_gs);
    o.init_property("colors" , GradientBevelFilter_as::colors_gs, 
        GradientBevelFilter_as::colors_gs);

    o.init_property("ratios" , GradientBevelFilter_as::ratios_gs, 
        GradientBevelFilter_as::ratios_gs);

    o.init_property("blurX" , GradientBevelFilter_as::blurX_gs, 
        GradientBevelFilter_as::blurX_gs);
    o.init_property("blurY" , GradientBevelFilter_as::blurY_gs, 
        GradientBevelFilter_as::blurY_gs);

    o.init_property("strength" , GradientBevelFilter_as::strength_gs, 
        GradientBevelFilter_as::strength_gs);

    o.init_property("quality" , GradientBevelFilter_as::quality_gs, 
        GradientBevelFilter_as::quality_gs);

    o.init_property("type" , GradientBevelFilter_as::type_gs, 
        GradientBevelFilter_as::type_gs);
    o.init_property("knockout" , GradientBevelFilter_as::knockout_gs, 
        GradientBevelFilter_as::knockout_gs);

}

as_value
GradientBevelFilter_as::distance_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
 if (fn.nargs == 0) {
		return as_value(ptr->m_distance );
    }
    float sp_distance = fn.arg(0).to_number<float> ();
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
GradientBevelFilter_as::angle_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
 if (fn.nargs == 0) {
		return as_value(ptr->m_angle );
    }
    float sp_angle = fn.arg(0).to_number<float> ();
    ptr->m_angle = sp_angle;
    return as_value();
}


as_value
GradientBevelFilter_as::colors_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    return as_value();
}


as_value
GradientBevelFilter_as::alphas_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    return as_value();
}


as_value
GradientBevelFilter_as::ratios_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    return as_value();
}


as_value
GradientBevelFilter_as::blurX_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
 if (fn.nargs == 0) {
		return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}


as_value
GradientBevelFilter_as::blurY_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
 if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}


as_value
GradientBevelFilter_as::strength_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_strength );
    }
    float sp_strength = fn.arg(0).to_number<float> ();
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
GradientBevelFilter_as::quality_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
     ptr->m_quality = sp_quality;
    return as_value();
}

as_value
GradientBevelFilter_as::knockout_gs(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    bool sp_knockout = fn.arg(0).to_bool ();
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
GradientBevelFilter_as::bitmap_clone(const fn_call& fn)
{
	boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<GradientBevelFilter_as> obj = new GradientBevelFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
GradientBevelFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<GradientBevelFilter_as> ptr = ensureType<GradientBevelFilter_as>(fn.this_ptr);

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
GradientBevelFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new GradientBevelFilter_as(GradientBevelFilter_as::Interface());

    GradientBevelFilter_as::attachProperties(*obj);
    return as_value(obj.get());

}

}
