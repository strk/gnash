// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/* $Id: BevelFilter_as.cpp,v 1.1 2007/08/27 03:06:42 cmusick Exp $ */

#include "BitmapFilter_as.h"
#include "BevelFilter.h"
#include "VM.h"
#include "builtin_function.h"

namespace gnash {

class BevelFilter_as
{
public:
    static as_object* Interface(); // To a BitmapFilter
    static void attachInterface(as_object& o); // Attach the interface.
    static void attachProperties(as_object& o); // Attach the properties.

    static void registerCtor(as_object& global); // public ctor
    static as_value ctor(const fn_call& fn); // constructor for BitmapFilter

    // Get set functions for ActionScript
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

private:
    static boost::intrusive_ptr<as_object> s_interface;
    static boost::intrusive_ptr<builtin_function> s_ctor;
};

boost::intrusive_ptr<as_object> BevelFilter_as::s_interface;
boost::intrusive_ptr<builtin_function> BevelFilter_as::s_ctor;

as_object*
BevelFilter_as::Interface()
{
    if (BevelFilter_as::s_interface == NULL)
    {
        BevelFilter_as::s_interface = new as_object(bitmapFilter_interface());
        VM::get().addStatic(BevelFilter_as::s_interface.get());

        BevelFilter_as::attachInterface(*BevelFilter_as::s_interface);
    }

    return BevelFilter_as::s_interface.get();
}

void
BevelFilter_as::attachInterface(as_object& /*o*/)
{
    // Filters are all properties.
    return;
}

void
BevelFilter_as::attachProperties(as_object& o)
{
    boost::intrusive_ptr<builtin_function> gs;

    gs = new builtin_function(BevelFilter_as::distance_gs, NULL);
    o.init_property("distance", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::angle_gs, NULL);
    o.init_property("angle", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::highlightColor_gs, NULL);
    o.init_property("highlightColor", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::highlightAlpha_gs, NULL);
    o.init_property("highlightAlpha", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::shadowColor_gs, NULL);
    o.init_property("shadowColor", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::shadowAlpha_gs, NULL);
    o.init_property("shadowAlpha", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::blurX_gs, NULL);
    o.init_property("blurX", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::blurY_gs, NULL);
    o.init_property("blurY", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::strength_gs, NULL);
    o.init_property("strength", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::quality_gs, NULL);
    o.init_property("quality", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::type_gs, NULL);
    o.init_property("type", *gs, *gs);

    gs = new builtin_function(BevelFilter_as::knockout_gs, NULL);
    o.init_property("knockout", *gs, *gs);
}

as_value
BevelFilter_as::distance_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_distance);
    }
    // setter
    float distance = fn.arg(0).to_number<float>();
    ptr->m_distance = distance;

    return as_value();
}

as_value
BevelFilter_as::angle_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_angle);
    }
    // setter
    float angle = fn.arg(0).to_number<float>();
    ptr->m_angle = angle;

    return as_value();
}

as_value
BevelFilter_as::highlightColor_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_highlightColor);
    }
    // setter
    uint32_t highlightColor = fn.arg(0).to_number<uint32_t>();
    ptr->m_highlightColor = highlightColor;

    return as_value();
}

as_value
BevelFilter_as::highlightAlpha_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_highlightAlpha);
    }
    // setter
    uint8_t highlightAlpha = fn.arg(0).to_number<uint8_t>();
    ptr->m_highlightAlpha = highlightAlpha;

    return as_value();
}

as_value
BevelFilter_as::shadowColor_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_shadowColor);
    }
    // setter
    uint32_t shadowColor = fn.arg(0).to_number<uint32_t>();
    ptr->m_shadowColor = shadowColor;

    return as_value();
}

as_value
BevelFilter_as::shadowAlpha_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_shadowAlpha);
    }
    // setter
    uint8_t shadowAlpha = fn.arg(0).to_number<uint8_t>();
    ptr->m_shadowAlpha = shadowAlpha;

    return as_value();
}

as_value
BevelFilter_as::blurX_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_blurX);
    }
    // setter
    float blurX = fn.arg(0).to_number<float>();
    ptr->m_blurX = blurX;

    return as_value();
}

as_value
BevelFilter_as::blurY_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_blurY);
    }
    // setter
    float blurY = fn.arg(0).to_number<float>();
    ptr->m_blurY = blurY;

    return as_value();
}

as_value
BevelFilter_as::strength_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_strength);
    }
    // setter
    float strength = fn.arg(0).to_number<float>();
    ptr->m_strength = strength;

    return as_value();
}

void
BevelFilter_as::registerCtor(as_object& global)
{
    if (BevelFilter_as::s_ctor != NULL)
        return;

    BevelFilter_as::s_ctor = new builtin_function(&BevelFilter_as::ctor,
        BevelFilter_as::Interface());
    VM::get().addStatic(BevelFilter_as::s_ctor.get());

    // TODO: Is this correct?
    BevelFilter_as::attachInterface(*BevelFilter_as::s_ctor);

    global.init_member("BevelFilter", BevelFilter_as::s_ctor.get());
}

as_value
BevelFilter_as::quality_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_quality);
    }
    // setter
    uint8_t quality = fn.arg(0).to_number<uint8_t>();
    ptr->m_quality = quality;

    return as_value();
}

as_value
BevelFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
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
    // setter
    string type = fn.arg(0).to_string();
    if (type == "outer")
        ptr->m_type = BevelFilter::OUTER_BEVEL;
    if (type == "inner")
        ptr->m_type = BevelFilter::INNER_BEVEL;
    if (type == "full")
        ptr->m_type = BevelFilter::FULL_BEVEL;

    return as_value();
}

as_value
BevelFilter_as::knockout_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter> ptr = ensureType<BevelFilter>(fn.this_ptr);

    if (fn.nargs == 0) // getter
    {
        return as_value(ptr->m_knockout);
    }
    // setter
    bool knockout = fn.arg(0).to_bool();
    ptr->m_knockout = knockout;

    return as_value();
}

as_value
BevelFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BevelFilter(BevelFilter_as::Interface());
    BevelFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

void bevelFilter_class_init(as_object& global)
{
   BevelFilter_as::registerCtor(global);
}

} // Namespace gnash

