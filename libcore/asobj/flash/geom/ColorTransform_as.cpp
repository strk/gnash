// ColorTransform_as.cpp:  ActionScript "ColorTransform" class, for Gnash.
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "ColorTransform_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" 
#include "NativeFunction.h"
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

namespace {

    as_value colortransform_concat(const fn_call& fn);
    as_value colortransform_toString(const fn_call& fn);
    as_value colortransform_alphaMultiplier(const fn_call& fn);
    as_value colortransform_alphaOffset(const fn_call& fn);
    as_value colortransform_blueMultiplier(const fn_call& fn);
    as_value colortransform_blueOffset(const fn_call& fn);
    as_value colortransform_greenMultiplier(const fn_call& fn);
    as_value colortransform_greenOffset(const fn_call& fn);
    as_value colortransform_redMultiplier(const fn_call& fn);
    as_value colortransform_redOffset(const fn_call& fn);
    as_value colortransform_rgb(const fn_call& fn);
    as_value colortransform_ctor(const fn_call& fn);

    void attachColorTransformInterface(as_object& o);
    as_value get_flash_geom_color_transform_constructor(const fn_call& fn);

}



ColorTransform_as::ColorTransform_as(double rm, double gm,
                                     double bm, double am,
                                     double ro, double go,
                                     double bo, double ao)
		:
        _alphaMultiplier(am),
        _alphaOffset(ao),
        _blueMultiplier(bm),
        _blueOffset(bo),
        _greenMultiplier(gm),
        _greenOffset(go),
        _redMultiplier(rm),
        _redOffset(ro)
{
}

// extern 
void
colortransform_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(getName(uri),
            get_flash_geom_color_transform_constructor, flags,
            getNamespace(uri));
}

void
registerColorTransformNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(colortransform_ctor, 1105, 0);
    vm.registerNative(colortransform_concat, 1105, 1);
    vm.registerNative(colortransform_alphaMultiplier, 1105, 101);
    vm.registerNative(colortransform_redMultiplier, 1105, 102);
    vm.registerNative(colortransform_greenMultiplier, 1105, 103);
    vm.registerNative(colortransform_blueMultiplier, 1105, 104);
    vm.registerNative(colortransform_alphaOffset, 1105, 105);
    vm.registerNative(colortransform_redOffset, 1105, 106);
    vm.registerNative(colortransform_greenOffset, 1105, 107);
    vm.registerNative(colortransform_blueOffset, 1105, 108);
    vm.registerNative(colortransform_rgb, 1105, 109);
}

namespace {

void
attachColorTransformInterface(as_object& o)
{
    int flags = 0;
    
    /// This has no flags:
    VM& vm = getVM(o);
    o.init_member("concat", vm.getNative(1105, 1), flags);

    flags = PropFlags::isProtected |
            PropFlags::onlySWF8Up;

    /// These are all protected and have SWF8 visibility
    Global_as* gl = getGlobal(o);
    o.init_member("toString", gl->createFunction(colortransform_toString),
            flags);

    NativeFunction* getset = vm.getNative(1105, 101);
    o.init_property("alphaMultiplier", *getset, *getset, flags);
    getset = vm.getNative(1105, 102);
    o.init_property("redMultiplier", *getset, *getset, flags);
    getset = vm.getNative(1105, 103);
    o.init_property("greenMultiplier",*getset, *getset, flags);
    getset = vm.getNative(1105, 104);
    o.init_property("blueMultiplier", *getset, *getset, flags);
    getset = vm.getNative(1105, 105);
    o.init_property("alphaOffset", *getset, *getset, flags);
    getset = vm.getNative(1105, 106);
    o.init_property("redOffset", *getset, *getset, flags);
    getset = vm.getNative(1105, 107);
    o.init_property("greenOffset", *getset, *getset, flags);
    getset = vm.getNative(1105, 108);
    o.init_property("blueOffset", *getset, *getset, flags);
    getset = vm.getNative(1105, 109);
    o.init_property("rgb", *getset, *getset, flags);
}


as_value
colortransform_alphaMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);

    if (!fn.nargs) {
        return as_value(relay->getAlphaMultiplier());
    }
    
    relay->setAlphaMultiplier(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_alphaOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value(relay->getAlphaOffset());
    }
    
    relay->setAlphaOffset(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_blueMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value(relay->getBlueMultiplier());
    }
    
    relay->setBlueMultiplier(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_blueOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value(relay->getBlueOffset());
    }
    
    relay->setBlueOffset(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_greenMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs) {
        return as_value(relay->getGreenMultiplier());
    }
    
    relay->setGreenMultiplier(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_greenOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    
    if (!fn.nargs) {
        return as_value(relay->getGreenOffset());
    }
    
    relay->setGreenOffset(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_redMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);

    if (!fn.nargs) {
        return as_value(relay->getRedMultiplier());
    }
    
    relay->setRedMultiplier(fn.arg(0).to_number());
	return as_value();
}

as_value
colortransform_redOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(relay->getRedOffset());
    }
    
    // Setter
    relay->setRedOffset(fn.arg(0).to_number());
	return as_value();
}


as_value
colortransform_concat(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);
	UNUSED(relay);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
colortransform_toString(const fn_call& fn)
{

	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);

    // We need the as_value to_string method, but using ptr->get_member
    // is unnecessary when we can read directly from the object.
    as_value alphaMultiplier(relay->getAlphaMultiplier());
    as_value alphaOffset(relay->getAlphaOffset());
    as_value blueMultiplier(relay->getBlueMultiplier());
    as_value blueOffset(relay->getBlueOffset());
    as_value greenMultiplier(relay->getGreenMultiplier());
    as_value greenOffset(relay->getGreenOffset());
    as_value redMultiplier(relay->getRedMultiplier());
    as_value redOffset(relay->getRedOffset());
   
    std::ostringstream ss;
    
    ss << "(redMultiplier=" << redMultiplier.to_string() << ", "
       << "greenMultiplier=" << greenMultiplier.to_string() << ", "
       << "blueMultiplier=" << blueMultiplier.to_string() << ", "
       << "alphaMultiplier=" << alphaMultiplier.to_string() << ", "
       << "redOffset=" << redOffset.to_string() << ", "
       << "greenOffset=" << greenOffset.to_string() << ", "
       << "blueOffset=" << blueOffset.to_string() << ", "
       << "alphaOffset=" << alphaOffset.to_string() << ")";
       
    return as_value(ss.str());

}


// Alpha values are left untouched, RGB multipliers reset to 0.
// The getter merely bit-shifts the values without checking for
// validity. We fmod the double values to avoid undefined behaviour
// on overflow.
as_value
colortransform_rgb(const fn_call& fn)
{
	ColorTransform_as* relay = ensureNativeType<ColorTransform_as>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        boost::uint32_t r = static_cast<boost::uint32_t>(
                std::fmod(relay->getRedOffset(), 4294967296.0));
        boost::uint32_t g = static_cast<boost::uint32_t>(
                std::fmod(relay->getGreenOffset(), 4294967296.0));
        boost::uint32_t b = static_cast<boost::uint32_t>(
                std::fmod(relay->getBlueOffset(), 4294967296.0));
        boost::uint32_t rgb = (r << 16) + (g << 8) + b;

        return as_value(rgb);
    }

    // Setter

    boost::uint32_t rgb = fn.arg(0).to_int();
    relay->setRedOffset((rgb & 0xFF0000) >> 16);
    relay->setGreenOffset((rgb & 0x00FF00) >> 8);
    relay->setBlueOffset(rgb & 0x0000FF);
    relay->setRedMultiplier(0);
    relay->setGreenMultiplier(0);
    relay->setBlueMultiplier(0);

	return as_value();
}

// Arguments passed to the constructor are converted to a number;
// objects without a valueOf method are therefore NaN.
// There must be a minimum of 8 arguments, or the default values are
// used. Extra arguments are discarded.
as_value
colortransform_ctor(const fn_call& fn)
{

    as_object* obj = ensureType<as_object>(fn.this_ptr).get();

    // Default arguments.
    if (fn.nargs < 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("ColorTransform(%s): not enough arguments (need 8). "
                        "Constructing with default values", ss.str());
        );

        obj->setRelay(new ColorTransform_as(1, 1, 1, 1, 0, 0, 0, 0));

	    return as_value();
        
    }

    if (fn.nargs > 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("ColorTransform(%s): discarding extra arguments",
                ss.str());
        );
    }

	obj->setRelay(new ColorTransform_as(fn.arg(0).to_number(),
                                        fn.arg(1).to_number(),
                                        fn.arg(2).to_number(),
                                        fn.arg(3).to_number(),
                                        fn.arg(4).to_number(),
                                        fn.arg(5).to_number(),
                                        fn.arg(6).to_number(),
                                        fn.arg(7).to_number()));

    return as_value();
}


as_value
get_flash_geom_color_transform_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.ColorTransform class");
    Global_as* gl = getGlobal(fn);
    as_object* proto = gl->createObject();
    as_object* cl = gl->createClass(&colortransform_ctor, proto);
    attachColorTransformInterface(*proto);
    return cl;
}

} // anonymous namespace

} // end of gnash namespace
