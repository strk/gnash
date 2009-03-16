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
#include "GlowFilter.h"
#include "VM.h"
#include "builtin_function.h"

// These _must_ be defined.
#define phelp_helper GlowFilter_as
#define phelp_class GlowFilter
#include "prophelper.h"
#include "BitmapFilter_as.h"

namespace gnash {

class GlowFilter_as : public as_object, public GlowFilter
{
public:
    phelp_gs(color);
    phelp_gs(alpha);
    phelp_gs(blurX);
    phelp_gs(blurY);
    phelp_gs(strength);
    phelp_gs(quality);
    phelp_gs(inner); 
    phelp_gs(knockout);

    phelp_i(bitmap_clone);
private:
    phelp_base_def;
};

phelp_base_imp((bitmapFilter_interface()), GlowFilter)

// Filters are property based.
phelp_i_attach_begin
phelp_i_replace(clone, bitmap_clone);
phelp_i_attach_end

phelp_gs_attach_begin
phelp_gs_attach(color);
phelp_gs_attach(alpha);
phelp_gs_attach(blurX);
phelp_gs_attach(blurY);
phelp_gs_attach(strength);
phelp_gs_attach(quality);
phelp_gs_attach(inner);
phelp_gs_attach(knockout);
phelp_gs_attach_end

phelp_property(boost::uint32_t, number<boost::uint32_t>, color)
phelp_property(boost::uint8_t, number<boost::uint8_t>, alpha)
phelp_property(float, number<float>, blurX)
phelp_property(float, number<float>, blurY)
phelp_property(float, number<float>, strength)
phelp_property(boost::uint8_t, number<boost::uint8_t>, quality)
phelp_property(bool, bool, inner)
phelp_property(bool, bool, knockout)

easy_clone(GlowFilter_as)

as_value
GlowFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new GlowFilter_as(GlowFilter_as::Interface());
    GlowFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

