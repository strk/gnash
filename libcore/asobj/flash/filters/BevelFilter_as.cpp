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

// These _must_ be defined for prophelpers to work correctly.
// This is enforced by the file itself.
#define phelp_helper BevelFilter_as
#include "prophelper.h"
#include "flash/filters/BitmapFilter_as.h"

namespace gnash {

class BevelFilter_as : public as_object, public BevelFilter
{
public:
    phelp_gs(distance);
    phelp_gs(angle);
    phelp_gs(highlightColor);
    phelp_gs(highlightAlpha);
    phelp_gs(shadowColor);
    phelp_gs(shadowAlpha);
    phelp_gs(blurX);
    phelp_gs(blurY);
    phelp_gs(strength);
    phelp_gs(quality);
    phelp_gs(type);
    phelp_gs(knockout);

    phelp_i(bitmap_clone);
private:
    phelp_base_def;
};

phelp_base_imp((bitmapFilter_interface()), BevelFilter)

// Replace BitmapFilter clone.
phelp_i_attach_begin
phelp_i_replace(clone, bitmap_clone);
phelp_i_attach_end

// Begin attaching properties, then attach them, then end.
phelp_gs_attach_begin
phelp_gs_attach(distance);
phelp_gs_attach(angle);
phelp_gs_attach(highlightColor);
phelp_gs_attach(highlightAlpha);
phelp_gs_attach(shadowColor);
phelp_gs_attach(shadowAlpha);
phelp_gs_attach(blurX);
phelp_gs_attach(blurY);
phelp_gs_attach(strength);
phelp_gs_attach(quality);
phelp_gs_attach(type);
phelp_gs_attach(knockout);
phelp_gs_attach_end

phelp_property(float, number<float>, distance)
phelp_property(float, number<float>, angle)
phelp_property(boost::uint32_t, number<boost::uint32_t>, highlightColor)
phelp_property(boost::uint8_t, number<boost::uint8_t>, highlightAlpha)
phelp_property(boost::uint32_t, number<boost::uint32_t>, shadowColor)
phelp_property(boost::uint8_t, number<boost::uint8_t>, shadowAlpha)
phelp_property(float, number<float>, blurX)
phelp_property(float, number<float>, blurY)
phelp_property(float, number<float>, strength)
phelp_property(boost::uint8_t, number<boost::uint8_t>, quality)
phelp_property(bool, bool, knockout)

easy_clone(BevelFilter_as)

as_value
BevelFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<BevelFilter_as> ptr = ensureType<BevelFilter_as>(fn.this_ptr);

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
BevelFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BevelFilter_as(BevelFilter_as::Interface());
    BevelFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

