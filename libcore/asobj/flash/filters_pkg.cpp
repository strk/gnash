// filters_pkg.cpp:  ActionScript "flash.filters" package, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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
//

#include "Object.h" // for getObjectInterface
#include "as_object.h"

#include "string_table.h"
#include "VM.h"

#include "flash/filters/BevelFilter_as.h"
#include "flash/filters/BitmapFilter_as.h"
#include "flash/filters/BlurFilter_as.h"
#include "flash/filters/ColorMatrixFilter_as.h"
#include "flash/filters/ConvolutionFilter_as.h"
#include "flash/filters/DisplacementMapFilter_as.h"
#include "flash/filters/DropShadowFilter_as.h"
#include "flash/filters/GlowFilter_as.h"
#include "flash/filters/GradientBevelFilter_as.h"
#include "flash/filters/GradientGlowFilter_as.h"

namespace gnash {

as_value
get_flash_filters_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.filters package");

	as_object* pkg = new as_object(getObjectInterface());

	BevelFilter_class_init(*pkg);
	BitmapFilter_class_init(*pkg);
	BlurFilter_class_init(*pkg);
	ColorMatrixFilter_class_init(*pkg);
	ConvolutionFilter_class_init(*pkg);
	DisplacementMapFilter_class_init(*pkg);
	DropShadowFilter_class_init(*pkg);
	GlowFilter_class_init(*pkg);
	GradientBevelFilter_class_init(*pkg);
	GradientGlowFilter_class_init(*pkg);

	return pkg;
}

void
flash_filters_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
    
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(st.find("filters"),
            get_flash_filters_package, flags);
}


} // end of gnash namespace
