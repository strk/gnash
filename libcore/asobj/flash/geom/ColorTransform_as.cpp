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
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include <sstream>

namespace gnash {

static as_value ColorTransform_concat(const fn_call& fn);
static as_value ColorTransform_toString(const fn_call& fn);
static as_value ColorTransform_alphaMultiplier_getset(const fn_call& fn);
static as_value ColorTransform_alphaOffset_getset(const fn_call& fn);
static as_value ColorTransform_blueMultiplier_getset(const fn_call& fn);
static as_value ColorTransform_blueOffset_getset(const fn_call& fn);
static as_value ColorTransform_greenMultiplier_getset(const fn_call& fn);
static as_value ColorTransform_greenOffset_getset(const fn_call& fn);
static as_value ColorTransform_redMultiplier_getset(const fn_call& fn);
static as_value ColorTransform_redOffset_getset(const fn_call& fn);
static as_value ColorTransform_rgb_getset(const fn_call& fn);

as_value ColorTransform_ctor(const fn_call& fn);


static void
attachColorTransformInterface(as_object& o)
{
    int flags = 0;
    /// This has no flags:
    o.init_member("concat", gl->createFunction(ColorTransform_concat), flags);

    flags = as_prop_flags::isProtected;

    /// These are all protected:
    o.init_member("toString", gl->createFunction(ColorTransform_toString),
            flags);

    o.init_property("alphaMultiplier", ColorTransform_alphaMultiplier_getset,
            ColorTransform_alphaMultiplier_getset, flags);
    o.init_property("alphaOffset", ColorTransform_alphaOffset_getset,
            ColorTransform_alphaOffset_getset, flags);
    o.init_property("blueMultiplier", ColorTransform_blueMultiplier_getset,
            ColorTransform_blueMultiplier_getset, flags);
    o.init_property("blueOffset", ColorTransform_blueOffset_getset,
            ColorTransform_blueOffset_getset, flags);
    o.init_property("greenMultiplier", ColorTransform_greenMultiplier_getset,
            ColorTransform_greenMultiplier_getset, flags);
    o.init_property("greenOffset", ColorTransform_greenOffset_getset,
            ColorTransform_greenOffset_getset, flags);
    o.init_property("redMultiplier", ColorTransform_redMultiplier_getset,
            ColorTransform_redMultiplier_getset, flags);
    o.init_property("redOffset", ColorTransform_redOffset_getset,
            ColorTransform_redOffset_getset, flags);
    o.init_property("rgb", ColorTransform_rgb_getset,
            ColorTransform_rgb_getset, flags);
}


static as_object*
getColorTransformInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachColorTransformInterface(*o);

	}

	return o.get();
}


static as_value
ColorTransform_alphaMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getAlphaMultiplier());
    }
    
    // Setter
    ptr->setAlphaMultiplier(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_alphaOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getAlphaOffset());
    }
    
    // Setter
    ptr->setAlphaOffset(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_blueMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getBlueMultiplier());
    }
    
    // Setter
    ptr->setBlueMultiplier(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_blueOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getBlueOffset());
    }
    
    // Setter
    ptr->setBlueOffset(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_greenMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getGreenMultiplier());
    }
    
    // Setter
    ptr->setGreenMultiplier(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_greenOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getGreenOffset());
    }
    
    // Setter
    ptr->setGreenOffset(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_redMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getRedMultiplier());
    }
    
    // Setter
    ptr->setRedMultiplier(fn.arg(0).to_number());
	return as_value();
}

static as_value
ColorTransform_redOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
    if (!fn.nargs)
    {
        // Getter
        return as_value(ptr->getRedOffset());
    }
    
    // Setter
    ptr->setRedOffset(fn.arg(0).to_number());
	return as_value();
}


static as_value
ColorTransform_concat(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_toString(const fn_call& fn)
{

    // Must be a ColorTransform
	boost::intrusive_ptr<ColorTransform_as> ptr = 
        ensureType<ColorTransform_as>(fn.this_ptr);

    // We need the as_value to_string method, but using ptr->get_member
    // is unnecessary when we can read directly from the object.
    as_value alphaMultiplier(ptr->getAlphaMultiplier());
    as_value alphaOffset(ptr->getAlphaOffset());
    as_value blueMultiplier(ptr->getBlueMultiplier());
    as_value blueOffset(ptr->getBlueOffset());
    as_value greenMultiplier(ptr->getGreenMultiplier());
    as_value greenOffset(ptr->getGreenOffset());
    as_value redMultiplier(ptr->getRedMultiplier());
    as_value redOffset(ptr->getRedOffset());
   
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
static as_value
ColorTransform_rgb_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);

    if (!fn.nargs)
    {
        // Getter
        boost::uint32_t r = static_cast<boost::uint32_t>(std::fmod(ptr->getRedOffset(), 4294967296.0));
        boost::uint32_t g = static_cast<boost::uint32_t>(std::fmod(ptr->getGreenOffset(), 4294967296.0));
        boost::uint32_t b = static_cast<boost::uint32_t>(std::fmod(ptr->getBlueOffset(), 4294967296.0));
        boost::uint32_t rgb = (r << 16) + (g << 8) + b;

        return as_value(rgb);
    }

    // Setter

    boost::uint32_t rgb = fn.arg(0).to_int();
    ptr->setRedOffset((rgb & 0xFF0000) >> 16);
    ptr->setGreenOffset((rgb & 0x00FF00) >> 8);
    ptr->setBlueOffset(rgb & 0x0000FF);
    ptr->setRedMultiplier(0);
    ptr->setGreenMultiplier(0);
    ptr->setBlueMultiplier(0);

	return as_value();
}

// Arguments passed to the constructor are converted to a number;
// objects without a valueOf method are therefore NaN.
// There must be a minimum of 8 arguments, or the default values are
// used. Extra arguments are discarded.
as_value
ColorTransform_ctor(const fn_call& fn)
{

    // Default arguments.
    if (fn.nargs < 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("ColorTransform(%s): not enough arguments (need 8). "
                        "Constructing with default values", ss.str());
        );

	    boost::intrusive_ptr<as_object> obj =
	                new ColorTransform_as(1, 1, 1, 1, 0, 0, 0, 0);

	    return as_value(obj.get());
        
    }

    if (fn.nargs > 8)
    {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("ColorTransform(%s): discarding extra arguments", ss.str());
        );
    }

	boost::intrusive_ptr<as_object> obj = new ColorTransform_as(
	                        fn.arg(0).to_number(),
	                        fn.arg(1).to_number(),
	                        fn.arg(2).to_number(),
	                        fn.arg(3).to_number(),
	                        fn.arg(4).to_number(),
	                        fn.arg(5).to_number(),
	                        fn.arg(6).to_number(),
	                        fn.arg(7).to_number());

    return as_value(obj.get());
}


as_function* getFlashGeomColorTransformConstructor()
{
    static builtin_function* cl = NULL;
    if ( ! cl )
    {
        cl=new builtin_function(&ColorTransform_ctor, getColorTransformInterface());
        VM::get().addStatic(cl);
    }
    return cl;
}


static as_value
get_flash_geom_color_transform_constructor(const fn_call& /*fn*/)
{
    log_debug("Loading flash.geom.ColorTransform class");

    return getFlashGeomColorTransformConstructor();
}


ColorTransform_as::ColorTransform_as(double rm, double gm,
                                     double bm, double am,
                                     double ro, double go,
                                     double bo, double ao)
		:
		as_object(getColorTransformInterface()),
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
void colortransform_class_init(as_object& where)
{
    // This is the ColorTransform "class"/"function"
    // in the 'where' package
    string_table& st = getStringTable(where);

    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
    where.init_destructive_property(st.find("ColorTransform"),
            get_flash_geom_color_transform_constructor, flags);
}

} // end of gnash namespace
