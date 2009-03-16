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
#include "ColorMatrixFilter.h"
#include "VM.h"
#include "builtin_function.h"

#define phelp_helper ColorMatrixFilter_as
#include "prophelper.h"
#include "BitmapFilter_as.h"

namespace gnash {

class ColorMatrixFilter_as : public as_object, public ColorMatrixFilter
{
public:
    phelp_gs(matrix);

    phelp_i(bitmap_clone);

private:
    phelp_base_def;
};

phelp_base_imp((bitmapFilter_interface()), ColorMatrixFilter)

// Filters are purely property based.
phelp_i_attach_begin
phelp_i_replace(clone, bitmap_clone);
phelp_i_attach_end

phelp_gs_attach_begin
phelp_gs_attach(matrix);
phelp_gs_attach_end

phelp_array_property(matrix)

easy_clone(ColorMatrixFilter_as)

as_value
ColorMatrixFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new ColorMatrixFilter_as(ColorMatrixFilter_as::Interface());
    ColorMatrixFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

