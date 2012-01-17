// ColorTransform_as.cpp:  ActionScript "ColorTransform" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
//   Free Software Foundation, Inc
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


#include "ColorTransform_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "NativeFunction.h"
#include "VM.h"

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

void
ColorTransform_as::concat(const ColorTransform_as& other)
{
    _redOffset += _redMultiplier * other.getRedOffset();
    _greenOffset += _greenMultiplier * other.getGreenOffset();
    _blueOffset += _blueMultiplier * other.getBlueOffset();
    _alphaOffset += _alphaMultiplier * other.getAlphaOffset();

    _redMultiplier *= other.getRedMultiplier();
    _greenMultiplier *= other.getGreenMultiplier();
    _blueMultiplier *= other.getBlueMultiplier();
    _alphaMultiplier *= other.getAlphaMultiplier();
}

// extern 
void
colortransform_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(uri,
            get_flash_geom_color_transform_constructor, flags);
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
    
    /// These have no flags:
    const int flags = 0;
    VM& vm = getVM(o);
    Global_as& gl = getGlobal(o);
    o.init_member("concat", vm.getNative(1105, 1), flags);
    o.init_member("toString", gl.createFunction(colortransform_toString),
            flags);

    /// These all have SWF8 visibility
    const int swf8 = PropFlags::onlySWF8Up;
    NativeFunction* getset = vm.getNative(1105, 101);
    o.init_property("alphaMultiplier", *getset, *getset, swf8);
    getset = vm.getNative(1105, 102);
    o.init_property("redMultiplier", *getset, *getset, swf8);
    getset = vm.getNative(1105, 103);
    o.init_property("greenMultiplier",*getset, *getset, swf8);
    getset = vm.getNative(1105, 104);
    o.init_property("blueMultiplier", *getset, *getset, swf8);
    getset = vm.getNative(1105, 105);
    o.init_property("alphaOffset", *getset, *getset, swf8);
    getset = vm.getNative(1105, 106);
    o.init_property("redOffset", *getset, *getset, swf8);
    getset = vm.getNative(1105, 107);
    o.init_property("greenOffset", *getset, *getset, swf8);
    getset = vm.getNative(1105, 108);
    o.init_property("blueOffset", *getset, *getset, swf8);
    getset = vm.getNative(1105, 109);
    o.init_property("rgb", *getset, *getset, swf8);
}


as_value
colortransform_alphaMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);

    if (!fn.nargs) {
        return as_value(relay->getAlphaMultiplier());
    }
    
    relay->setAlphaMultiplier(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_alphaOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    if (!fn.nargs) {
        return as_value(relay->getAlphaOffset());
    }
    
    relay->setAlphaOffset(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_blueMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    if (!fn.nargs) {
        return as_value(relay->getBlueMultiplier());
    }
    
    relay->setBlueMultiplier(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_blueOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    if (!fn.nargs) {
        return as_value(relay->getBlueOffset());
    }
    
    relay->setBlueOffset(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_greenMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    if (!fn.nargs) {
        return as_value(relay->getGreenMultiplier());
    }
    
    relay->setGreenMultiplier(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_greenOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    
    if (!fn.nargs) {
        return as_value(relay->getGreenOffset());
    }
    
    relay->setGreenOffset(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_redMultiplier(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);

    if (!fn.nargs) {
        return as_value(relay->getRedMultiplier());
    }
    
    relay->setRedMultiplier(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}

as_value
colortransform_redOffset(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);
    if (!fn.nargs)
    {
        // Getter
        return as_value(relay->getRedOffset());
    }
    
    // Setter
    relay->setRedOffset(toNumber(fn.arg(0), getVM(fn)));
	return as_value();
}


as_value
colortransform_concat(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);

    if (!fn.nargs) {
        // Log error
        return as_value();
    }
    as_object* o = toObject(fn.arg(0), getVM(fn));
    ColorTransform_as* tr;
    if (!isNativeType(o, tr)) {
        return as_value();
    }
    
    relay->concat(*tr);

	return as_value();
}

as_value
colortransform_toString(const fn_call& fn)
{

    as_object* ptr = ensure<ValidThis>(fn);

    VM& vm = getVM(fn);

    const as_value& am = getMember(*ptr, getURI(vm, "alphaMultiplier"));
    const as_value& ao = getMember(*ptr, getURI(vm, "alphaOffset"));
    const as_value& bm = getMember(*ptr, getURI(vm, "blueMultiplier"));
    const as_value& bo = getMember(*ptr, getURI(vm, "blueOffset"));
    const as_value& gm = getMember(*ptr, getURI(vm, "greenMultiplier"));
    const as_value& go = getMember(*ptr, getURI(vm, "greenOffset"));
    const as_value& rm = getMember(*ptr, getURI(vm, "redMultiplier"));
    const as_value& ro = getMember(*ptr, getURI(vm, "redOffset"));
   
    as_value ret("(redMultiplier=");
    newAdd(ret, rm, vm);
    newAdd(ret, ", greenMultiplier=", vm);
    newAdd(ret, gm, vm);
    newAdd(ret, ", blueMultiplier=", vm);
    newAdd(ret, bm, vm);
    newAdd(ret, ", alphaMultiplier=", vm);
    newAdd(ret, am, vm);
    newAdd(ret, ", redOffset=", vm);
    newAdd(ret, ro, vm);
    newAdd(ret, ", greenOffset=", vm);
    newAdd(ret, go, vm);
    newAdd(ret, ", blueOffset=", vm);
    newAdd(ret, bo, vm);
    newAdd(ret, ", alphaOffset=", vm);
    newAdd(ret, ao, vm);
    newAdd(ret, ")", vm);

    return ret;

}


// Alpha values are left untouched, RGB multipliers reset to 0.
// The getter merely bit-shifts the values without checking for
// validity. We fmod the double values to avoid undefined behaviour
// on overflow.
as_value
colortransform_rgb(const fn_call& fn)
{
	ColorTransform_as* relay = ensure<ThisIsNative<ColorTransform_as> >(fn);

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

    boost::uint32_t rgb = toInt(fn.arg(0), getVM(fn));
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

    as_object* obj = ensure<ValidThis>(fn);

    // Default arguments.
    if (fn.nargs < 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("ColorTransform(%s): not enough arguments (need 8). "
                          "Constructing with default values"), ss.str());
        );

        obj->setRelay(new ColorTransform_as(1, 1, 1, 1, 0, 0, 0, 0));

	    return as_value();
        
    }

    if (fn.nargs > 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror(_("ColorTransform(%s): discarding extra arguments"),
                ss.str());
        );
    }

	obj->setRelay(new ColorTransform_as(toNumber(fn.arg(0), getVM(fn)),
                                        toNumber(fn.arg(1), getVM(fn)),
                                        toNumber(fn.arg(2), getVM(fn)),
                                        toNumber(fn.arg(3), getVM(fn)),
                                        toNumber(fn.arg(4), getVM(fn)),
                                        toNumber(fn.arg(5), getVM(fn)),
                                        toNumber(fn.arg(6), getVM(fn)),
                                        toNumber(fn.arg(7), getVM(fn))));

    return as_value();
}


as_value
get_flash_geom_color_transform_constructor(const fn_call& fn)
{
    log_debug("Loading flash.geom.ColorTransform class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
    as_object* cl = gl.createClass(&colortransform_ctor, proto);
    attachColorTransformInterface(*proto);
    return cl;
}

} // anonymous namespace

} // end of gnash namespace
