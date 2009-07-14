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
#include "BevelFilter.h"
#include "VM.h"
#include "builtin_function.h"
#include "flash/filters/BitmapFilter_as.h"
#include "Global_as.h"

namespace gnash {

class BevelFilter_as : public as_object, public BevelFilter
{
public:
    static as_value distance_gs(const fn_call& fn);
    static as_value angle_gs(const fn_call& fn);
    static as_value highlightColor_gs(const fn_call& fn);
    static as_value highlightAlpha_gs(const fn_call& fn);
    static as_value shadowColor_gs(const fn_call& fn);
    static as_value shadowAlpha_gs(const fn_call& fn);
    static as_value blurX_gs(const fn_call& fn);
    static as_value blurY_gs(const fn_call& fn);
    static as_value strength_gs(const fn_call& fn);
    static as_value quality_gs(const fn_call& fn);
    static as_value type_gs(const fn_call& fn);
    static as_value knockout_gs(const fn_call& fn);
    static as_value bitmap_clone(const fn_call& fn);

    public: BevelFilter_as(as_object *obj)
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


boost::intrusive_ptr<as_object> BevelFilter_as::s_interface;
boost::intrusive_ptr<as_object> BevelFilter_as::s_ctor;

as_object*
BevelFilter_as::Interface() {
    if (BevelFilter_as::s_interface == NULL) {
        BevelFilter_as::s_interface = new as_object (getBitmapFilterInterface());
        VM::get().addStatic(BevelFilter_as::s_interface.get());
        BevelFilter_as::attachInterface(*BevelFilter_as::s_interface);
    }
    return BevelFilter_as::s_interface.get();
}

void
BevelFilter_as::registerCtor(as_object& global) {
    if (BevelFilter_as::s_ctor != NULL) return;
    Global_as* gl = getGlobal(global);
    BevelFilter_as::s_ctor = gl->createClass(&BevelFilter_as::ctor, BevelFilter_as::Interface());
    VM::get().addStatic(BevelFilter_as::s_ctor.get());
    BevelFilter_as::attachInterface(*BevelFilter_as::s_ctor);
    global.init_member("BevelFilter" , BevelFilter_as::s_ctor.get());
}

void
bevelfilter_class_init(as_object& global)
{
    BevelFilter_as::registerCtor(global);
}


void
BevelFilter_as::attachInterface(as_object& o)
{
    Global_as* gl = getGlobal(o);
    boost::intrusive_ptr<builtin_function> gs;

    o.set_member(VM::get().getStringTable().find("clone"), gl->createFunction(bitmap_clone));

}


void
BevelFilter_as::attachProperties(as_object& o) {
    boost::intrusive_ptr<builtin_function> gs;
    o.init_property("distance" , BevelFilter_as::distance_gs, 
        BevelFilter_as::distance_gs);
    o.init_property("angle" , BevelFilter_as::angle_gs, 
        BevelFilter_as::angle_gs);
    o.init_property("highlightColor" , BevelFilter_as::highlightColor_gs, 
        BevelFilter_as::highlightColor_gs);
    o.init_property("highlightAlpha" , BevelFilter_as::highlightAlpha_gs, 
        BevelFilter_as::highlightAlpha_gs);
    o.init_property("shadowColor" , BevelFilter_as::shadowColor_gs, 
        BevelFilter_as::shadowColor_gs);
    o.init_property("shadowAlpha" , BevelFilter_as::shadowAlpha_gs, 
        BevelFilter_as::shadowAlpha_gs);
    o.init_property("blurX" , BevelFilter_as::blurX_gs, 
        BevelFilter_as::blurX_gs);
    o.init_property("blurY" , BevelFilter_as::blurY_gs, 
        BevelFilter_as::blurY_gs);
    o.init_property("strength" , BevelFilter_as::strength_gs, 
        BevelFilter_as::strength_gs);
    o.init_property("quality" , BevelFilter_as::quality_gs, 
        BevelFilter_as::quality_gs);
    o.init_property("type" , BevelFilter_as::type_gs, 
        BevelFilter_as::type_gs);
    o.init_property("knockout" , BevelFilter_as::knockout_gs, 
        BevelFilter_as::knockout_gs);

}

as_value
BevelFilter_as::distance_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_distance );
    }
    
    float sp_distance = fn.arg(0).to_number();
    ptr->m_distance = sp_distance;
    return as_value();
}

as_value
BevelFilter_as::angle_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_angle );
    }
    float sp_angle = fn.arg(0).to_number<float> ();
    ptr->m_angle = sp_angle;
    return as_value();
}

as_value
BevelFilter_as::highlightColor_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_highlightColor );
    }
    boost::uint32_t sp_highlightColor = fn.arg(0).to_number<boost::uint32_t> ();
    ptr->m_highlightColor = sp_highlightColor;
    return as_value();
}

as_value
BevelFilter_as::highlightAlpha_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_highlightAlpha );
    }
    boost::uint8_t sp_highlightAlpha = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_highlightAlpha = sp_highlightAlpha;
    return as_value();
}

as_value
BevelFilter_as::shadowColor_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_shadowColor );
    }
    boost::uint32_t sp_shadowColor = fn.arg(0).to_number<boost::uint32_t> ();
    ptr->m_shadowColor = sp_shadowColor;
    return as_value();
}

as_value
BevelFilter_as::shadowAlpha_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_shadowAlpha );
    }
    boost::uint8_t sp_shadowAlpha = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_shadowAlpha = sp_shadowAlpha;
    return as_value();
}

as_value
BevelFilter_as::blurX_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_blurX );
    }
    float sp_blurX = fn.arg(0).to_number<float> ();
    ptr->m_blurX = sp_blurX;
    return as_value();
}

as_value
BevelFilter_as::blurY_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_blurY );
    }
    float sp_blurY = fn.arg(0).to_number<float> ();
    ptr->m_blurY = sp_blurY;
    return as_value();
}

as_value
BevelFilter_as::strength_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
        return as_value(ptr->m_strength );
    }
    float sp_strength = fn.arg(0).to_number<float> ();
    ptr->m_strength = sp_strength;
    return as_value();
}

as_value
BevelFilter_as::quality_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_quality );
    }
    boost::uint8_t sp_quality = fn.arg(0).to_number<boost::uint8_t> ();
    ptr->m_quality = sp_quality;
    return as_value();
}

as_value
BevelFilter_as::knockout_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    if (fn.nargs == 0) {
		return as_value(ptr->m_knockout );
    }
    bool sp_knockout = fn.arg(0).to_bool ();
    ptr->m_knockout = sp_knockout;
    return as_value();
}

as_value
BevelFilter_as::bitmap_clone(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);
    boost::intrusive_ptr<BevelFilter_as> obj = new BevelFilter_as(*ptr);
    boost::intrusive_ptr<as_object> r = obj;
    r->set_prototype(ptr->get_prototype());
    r->copyProperties(*ptr);
    return as_value(r);
}

as_value
BevelFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);

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
BevelFilter_as::ctor(const fn_call& )
{
    boost::intrusive_ptr<as_object> obj = new BevelFilter_as(BevelFilter_as::Interface());

    BevelFilter_as::attachProperties(*obj);
    return as_value(obj.get());

}

}
