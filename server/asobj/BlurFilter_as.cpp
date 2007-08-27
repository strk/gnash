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

/* $Id: BlurFilter_as.cpp,v 1.1 2007/08/27 18:13:41 cmusick Exp $ */

#include "BitmapFilter_as.h"
#include "BlurFilter.h"
#include "VM.h"
#include "builtin_function.h"

// These _must_ be defined.
#define phelp_helper BlurFilter_as
#define phelp_class BlurFilter
#include "prophelper.h"

namespace gnash {

class BlurFilter_as
{
    phelp_base_def;
public:
    phelp_gs(blurX);
    phelp_gs(blurY);
    phelp_gs(quality);
};

phelp_base_imp((bitmapFilter_interface()), BlurFilter);

// Filters are property based.
phelp_i_attach_empty

phelp_gs_attach_begin
phelp_gs_attach(blurX);
phelp_gs_attach(blurY);
phelp_gs_attach(quality);
phelp_gs_attach_end

phelp_property(float, number<float>, blurX)
phelp_property(float, number<float>, blurY)
phelp_property(uint8_t, number<uint8_t>, quality)

as_value
BlurFilter_as::ctor(const fn_call& /*fn*/)
{
    boost::intrusive_ptr<as_object> obj = new BlurFilter(BlurFilter_as::Interface());
    BlurFilter_as::attachProperties(*obj);

    return as_value(obj.get());
}

} // Namespace gnash

