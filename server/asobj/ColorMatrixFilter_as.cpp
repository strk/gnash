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

/* $Id: ColorMatrixFilter_as.cpp,v 1.1 2007/08/27 18:13:41 cmusick Exp $ */

#include "BitmapFilter_as.h"
#include "ColorMatrixFilter.h"
#include "VM.h"
#include "builtin_function.h"

#define phelp_helper ColorMatrixFilter_as
#define phelp_class ColorMatrixFilter
#include "prophelper.h"

namespace gnash {

class ColorMatrixFilter_as
{
    phelp_base_def;
public:
    phelp_gs(matrix);
};

phelp_base_imp((bitmapFilter_interface()), ColorMatrixFilter);

// Filters are purely property based.
phelp_i_attach_empty

phelp_gs_attach_begin
phelp_gs_attach(matrix);
phelp_gs_attach_end

phelp_array_property(matrix);

as_value
ColorMatrixFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new ColorMatrixFilter(ColorMatrixFilter_as::Interface());
    ColorMatrixFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

