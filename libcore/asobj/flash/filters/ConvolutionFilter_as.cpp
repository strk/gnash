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
#include "ConvolutionFilter.h"
#include "VM.h"
#include "builtin_function.h"

#define phelp_helper ConvolutionFilter_as
#define phelp_class ConvolutionFilter
#include "prophelper.h"
#include "BitmapFilter_as.h"

namespace gnash {

class ConvolutionFilter_as : public as_object, public ConvolutionFilter
{
public:
    phelp_gs(matrixX);
    phelp_gs(matrixY);
    phelp_gs(matrix);
    phelp_gs(divisor);
    phelp_gs(bias);
    phelp_gs(preserveAlpha);
    phelp_gs(clamp);
    phelp_gs(color);
    phelp_gs(alpha);

    phelp_i(bitmap_clone);
private:
    phelp_base_def;
};

phelp_base_imp((bitmapFilter_interface()), ConvolutionFilter)

phelp_i_attach_begin
phelp_i_replace(clone, bitmap_clone);
phelp_i_attach_end

phelp_gs_attach_begin
phelp_gs_attach(matrixX);
phelp_gs_attach(matrixY);
phelp_gs_attach(matrix);
phelp_gs_attach(divisor);
phelp_gs_attach(bias);
phelp_gs_attach(preserveAlpha);
phelp_gs_attach(clamp);
phelp_gs_attach(color);
phelp_gs_attach(alpha);
phelp_gs_attach_end

phelp_property(boost::uint8_t, number<boost::uint8_t>, matrixX)
phelp_property(boost::uint8_t, number<boost::uint8_t>, matrixY)
phelp_property(float, number<float>, divisor)
phelp_property(float, number<float>, bias)
phelp_property(bool, bool, preserveAlpha)
phelp_property(bool, bool, clamp)
phelp_property(boost::uint32_t, number<boost::uint32_t>, color)
phelp_property(boost::uint8_t, number<boost::uint8_t>, alpha)
phelp_array_property(matrix)

easy_clone(ConvolutionFilter_as)

as_value
ConvolutionFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new ConvolutionFilter_as(ConvolutionFilter_as::Interface());
    ConvolutionFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

