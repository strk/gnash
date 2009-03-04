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

// These _must_ be defined for prophelpers to work correctly.
// This is enforced by the file itself.
#define phelp_helper GradientGlowFilter_as
#define phelp_class GradientGlowFilter
#include "prophelper.h"
#include "BitmapFilter_as.h"

namespace gnash {

class GradientGlowFilter_as : public as_object, public GradientGlowFilter
{
public:
    phelp_gs(distance);
    phelp_gs(angle);
    phelp_gs(colors);
    phelp_gs(alphas);
    phelp_gs(ratios);
    phelp_gs(blurX);
    phelp_gs(blurY);
    phelp_gs(strength);
    phelp_gs(quality);
    phelp_gs(type); // No automation
    phelp_gs(knockout);

    phelp_i(bitmap_clone);

private:
    phelp_base_def;
};

phelp_base_imp((bitmapFilter_interface()), GradientGlowFilter)

// Filters are purely property based.
phelp_i_attach_begin
phelp_i_replace(clone, bitmap_clone);
phelp_i_attach_end

// Begin attaching properties, then attach them, then end.
phelp_gs_attach_begin
phelp_gs_attach(distance);
phelp_gs_attach(angle);
phelp_gs_attach(colors);
phelp_gs_attach(alphas);
phelp_gs_attach(ratios);
phelp_gs_attach(blurX);
phelp_gs_attach(blurY);
phelp_gs_attach(strength);
phelp_gs_attach(quality);
phelp_gs_attach(type);
phelp_gs_attach(knockout);
phelp_gs_attach_end

phelp_property(float, number<float>, distance)
phelp_property(float, number<float>, angle)
phelp_array_property(colors)
phelp_array_property(alphas)
phelp_array_property(ratios)
phelp_property(float, number<float>, blurX)
phelp_property(float, number<float>, blurY)
phelp_property(float, number<float>, strength)
phelp_property(boost::uint8_t, number<boost::uint8_t>, quality)
// Type is not automatable.
phelp_property(bool, bool, knockout)

easy_clone(GradientGlowFilter_as)

as_value
GradientGlowFilter_as::type_gs(const fn_call& fn)
{
    boost::intrusive_ptr<GradientGlowFilter_as> ptr = ensureType<GradientGlowFilter_as>(fn.this_ptr);

    if (fn.nargs == 0) // getter
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
    // setter
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
GradientGlowFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new GradientGlowFilter_as(GradientGlowFilter_as::Interface());
    GradientGlowFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

