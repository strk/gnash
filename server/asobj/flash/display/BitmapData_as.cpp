// BitmapData_as.cpp:  ActionScript "BitmapData" class, for Gnash.
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

#include "BitmapData_as.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance

#include <sstream>

namespace gnash {

static as_value BitmapData_applyFilter(const fn_call& fn);
static as_value BitmapData_clone(const fn_call& fn);
static as_value BitmapData_colorTransform(const fn_call& fn);
static as_value BitmapData_copyChannel(const fn_call& fn);
static as_value BitmapData_copyPixels(const fn_call& fn);
static as_value BitmapData_dispose(const fn_call& fn);
static as_value BitmapData_draw(const fn_call& fn);
static as_value BitmapData_fillRect(const fn_call& fn);
static as_value BitmapData_floodFill(const fn_call& fn);
static as_value BitmapData_generateFilterRect(const fn_call& fn);
static as_value BitmapData_getColorBoundsRect(const fn_call& fn);
static as_value BitmapData_getPixel(const fn_call& fn);
static as_value BitmapData_getPixel32(const fn_call& fn);
static as_value BitmapData_hitTest(const fn_call& fn);
static as_value BitmapData_merge(const fn_call& fn);
static as_value BitmapData_noise(const fn_call& fn);
static as_value BitmapData_paletteMap(const fn_call& fn);
static as_value BitmapData_perlinNoise(const fn_call& fn);
static as_value BitmapData_pixelDissolve(const fn_call& fn);
static as_value BitmapData_scroll(const fn_call& fn);
static as_value BitmapData_setPixel(const fn_call& fn);
static as_value BitmapData_setPixel32(const fn_call& fn);
static as_value BitmapData_threshold(const fn_call& fn);
static as_value BitmapData_height_getset(const fn_call& fn);
static as_value BitmapData_rectangle_getset(const fn_call& fn);
static as_value BitmapData_transparent_getset(const fn_call& fn);
static as_value BitmapData_width_getset(const fn_call& fn);

static as_value BitmapData_loadBitmap(const fn_call& fn);

as_value BitmapData_ctor(const fn_call& fn);

static void
attachBitmapDataInterface(as_object& o)
{
    o.init_member("applyFilter", new builtin_function(BitmapData_applyFilter));
    o.init_member("clone", new builtin_function(BitmapData_clone));
    o.init_member("colorTransform", new builtin_function(BitmapData_colorTransform));
    o.init_member("copyChannel", new builtin_function(BitmapData_copyChannel));
    o.init_member("copyPixels", new builtin_function(BitmapData_copyPixels));
    o.init_member("dispose", new builtin_function(BitmapData_dispose));
    o.init_member("draw", new builtin_function(BitmapData_draw));
    o.init_member("fillRect", new builtin_function(BitmapData_fillRect));
    o.init_member("floodFill", new builtin_function(BitmapData_floodFill));
    o.init_member("generateFilterRect", new builtin_function(BitmapData_generateFilterRect));
    o.init_member("getColorBoundsRect", new builtin_function(BitmapData_getColorBoundsRect));
    o.init_member("getPixel", new builtin_function(BitmapData_getPixel));
    o.init_member("getPixel32", new builtin_function(BitmapData_getPixel32));
    o.init_member("hitTest", new builtin_function(BitmapData_hitTest));
    o.init_member("merge", new builtin_function(BitmapData_merge));
    o.init_member("noise", new builtin_function(BitmapData_noise));
    o.init_member("paletteMap", new builtin_function(BitmapData_paletteMap));
    o.init_member("perlinNoise", new builtin_function(BitmapData_perlinNoise));
    o.init_member("pixelDissolve", new builtin_function(BitmapData_pixelDissolve));
    o.init_member("scroll", new builtin_function(BitmapData_scroll));
    o.init_member("setPixel", new builtin_function(BitmapData_setPixel));
    o.init_member("setPixel32", new builtin_function(BitmapData_setPixel32));
    o.init_member("threshold", new builtin_function(BitmapData_threshold));
    o.init_property("height", BitmapData_height_getset, BitmapData_height_getset);
    o.init_property("rectangle", BitmapData_rectangle_getset, BitmapData_rectangle_getset);
    o.init_property("transparent", BitmapData_transparent_getset, BitmapData_transparent_getset);
    o.init_property("width", BitmapData_width_getset, BitmapData_width_getset);
}

static void
attachBitmapDataStaticProperties(as_object& o)
{
   
    o.init_member("loadBitmap", new builtin_function(BitmapData_loadBitmap));
}

static as_object*
getBitmapDataInterface()
{
	boost::intrusive_ptr<as_object> o;
	// TODO: check if this class should inherit from Object
	//       or from a different class
	o = new as_object(getObjectInterface());
	attachBitmapDataInterface(*o);
	return o.get();
}

class BitmapData_as: public as_object
{

public:

	BitmapData_as()
		:
		as_object(getBitmapDataInterface())
	{}

	// override from as_object ?
	//std::string get_text_value() const { return "BitmapData"; }

	// override from as_object ?
	//double get_numeric_value() const { return 0; }
};


static as_value
BitmapData_applyFilter(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_clone(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_colorTransform(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_copyChannel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_copyPixels(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_dispose(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_draw(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_fillRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_floodFill(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_generateFilterRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_getColorBoundsRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_getPixel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_getPixel32(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_hitTest(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_merge(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_noise(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_paletteMap(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_perlinNoise(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_pixelDissolve(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_scroll(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_setPixel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_setPixel32(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_threshold(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_height_getset(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_rectangle_getset(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_transparent_getset(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

static as_value
BitmapData_width_getset(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


static as_value
BitmapData_loadBitmap(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


as_value
BitmapData_ctor(const fn_call& fn)
{
	boost::intrusive_ptr<as_object> obj = new BitmapData_as;

	if ( fn.nargs )
	{
		std::stringstream ss;
		fn.dump_args(ss);
		LOG_ONCE( log_unimpl("BitmapData(%s): %s", ss.str(), _("arguments discarded")) );
	}

	return as_value(obj.get()); // will keep alive
}

// extern 
void BitmapData_class_init(as_object& where)
{
	// This is going to be the BitmapData "class"/"function"
	// in the 'where' package
	boost::intrusive_ptr<builtin_function> cl;
	cl=new builtin_function(&BitmapData_ctor, getBitmapDataInterface());
	attachBitmapDataStaticProperties(*cl);

	// Register _global.BitmapData
	where.init_member("BitmapData", cl.get());
}

} // end of gnash namespace
