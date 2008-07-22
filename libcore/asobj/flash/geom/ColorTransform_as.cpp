// ColorTransform_as.cpp:  ActionScript "ColorTransform" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
    o.init_member("concat", new builtin_function(ColorTransform_concat));
    o.init_member("toString", new builtin_function(ColorTransform_toString));
    o.init_property("alphaMultiplier", ColorTransform_alphaMultiplier_getset, ColorTransform_alphaMultiplier_getset);
    o.init_property("alphaOffset", ColorTransform_alphaOffset_getset, ColorTransform_alphaOffset_getset);
    o.init_property("blueMultiplier", ColorTransform_blueMultiplier_getset, ColorTransform_blueMultiplier_getset);
    o.init_property("blueOffset", ColorTransform_blueOffset_getset, ColorTransform_blueOffset_getset);
    o.init_property("greenMultiplier", ColorTransform_greenMultiplier_getset, ColorTransform_greenMultiplier_getset);
    o.init_property("greenOffset", ColorTransform_greenOffset_getset, ColorTransform_greenOffset_getset);
    o.init_property("redMultiplier", ColorTransform_redMultiplier_getset, ColorTransform_redMultiplier_getset);
    o.init_property("redOffset", ColorTransform_redOffset_getset, ColorTransform_redOffset_getset);
    o.init_property("rgb", ColorTransform_rgb_getset, ColorTransform_rgb_getset);
}

static void
attachColorTransformStaticProperties(as_object& /*o*/)
{
   
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

class ColorTransform_as: public as_object
{

public:

	ColorTransform_as()
		:
		as_object(getColorTransformInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "ColorTransform"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


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
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_alphaMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_alphaOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_blueMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_blueOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_greenMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_greenOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_redMultiplier_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_redOffset_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
ColorTransform_rgb_getset(const fn_call& fn)
{
	boost::intrusive_ptr<ColorTransform_as> ptr = ensureType<ColorTransform_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}



as_value
ColorTransform_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new ColorTransform_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("ColorTransform(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void ColorTransform_class_init(as_object& where)
{
	// This is going to be the ColorTransform "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&ColorTransform_ctor, getColorTransformInterface());
	attachColorTransformStaticProperties(*cl);

	// Register _global.ColorTransform
	where.init_member("ColorTransform", cl.get());
}

} // end of gnash namespace
