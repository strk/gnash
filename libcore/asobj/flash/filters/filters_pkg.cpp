// filters_pkg.cpp:  ActionScript "flash.desktop" package, for Gnash.
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
#include "MovieClip.h"

#include "BevelFilter_as3.h"
#include "BitmapFilterQuality_as3.h"
#include "BitmapFilterType_as3.h"
#include "BitmapFilter_as3.h"
#include "BlurFilter_as3.h"
#include "ColorMatrixFilter_as3.h"
#include "ConvolutionFilter_as3.h"
#include "DisplacementMapFilterMode_as3.h"
#include "DisplacementMapFilter_as3.h"
#include "DropShadowFilter_as3.h"
#include "GlowFilter_as3.h"
#include "GradientBevelFilter_as3.h"
#include "GradientGlowFilter_as3.h"

#include "filters_pkg.h"
#include "filtersclasses.h"

namespace gnash {

static as_value
get_flash_filters_package(const fn_call& /*fn*/)
{
	log_debug("Loading flash.filters package");
	as_object *pkg = new as_object(getObjectInterface());

	// Call the [objectname]_init() function for each class.
	int i = 0;
	do {
	    asclasses[i](*pkg);
	} while (asclasses[++i] != 0);

	return pkg;
}

void
flash_filters_package_init(as_object& where)
{
	string_table& st = where.getVM().getStringTable();
	where.init_destructive_property(st.find("filters"), get_flash_filters_package);
}


} // end of gnash namespace
