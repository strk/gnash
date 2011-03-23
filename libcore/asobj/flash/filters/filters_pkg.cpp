// filters_pkg.cpp:  ActionScript "flash.desktop" package, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#include "as_object.h"
#include "VM.h"
#include "VM.h"
#include "fn_call.h"
#include "MovieClip.h"

#include "BevelFilter_as.h"
#include "BitmapFilter_as.h"
#include "BlurFilter_as.h"
#include "ColorMatrixFilter_as.h"
#include "ConvolutionFilter_as.h"
#include "DisplacementMapFilter_as.h"
#include "DropShadowFilter_as.h"
#include "GlowFilter_as.h"
#include "GradientBevelFilter_as.h"
#include "GradientGlowFilter_as.h"
#include "namedStrings.h"
#include "filters_pkg.h"

namespace gnash {

namespace {

as_value
get_flash_filters_package(const fn_call& fn)
{

    log_debug("Loading flash.filters package");
    Global_as& gl = getGlobal(fn);
    as_object* pkg = createObject(gl);

    VM& vm = getVM(fn);

    bitmapfilter_class_init(*pkg, getURI(vm, "BitmapFilter"));
    bevelfilter_class_init(*pkg, getURI(vm, "BevelFilter"));
    blurfilter_class_init(*pkg, getURI(vm, "BlurFilter"));
    colormatrixfilter_class_init(*pkg, getURI(vm, "ColorMatrixFilter"));
    convolutionfilter_class_init(*pkg, getURI(vm, "ConvolutionFilter"));
    displacementmapfilter_class_init(*pkg, getURI(vm, "DisplacementMapFilter"));
    dropshadowfilter_class_init(*pkg, getURI(vm, "DropShadowFilter"));
    glowfilter_class_init(*pkg, getURI(vm, "GlowFilter"));
    gradientbevelfilter_class_init(*pkg, getURI(vm, "GradientBevelFilter"));
    gradientglowfilter_class_init(*pkg, getURI(vm, "GradientGlowFilter"));
    
    return pkg;
}

} // anonymous namespace

void
flash_filters_package_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri, get_flash_filters_package, flags);
}


} // end of gnash namespace
