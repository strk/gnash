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
#include "GnashImage.h"
#include "Bitmap.h"
#include "flash/geom/Rectangle_as.h" // for BitmapData.rectangle
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "Object.h" // for AS inheritance
#include "VM.h" // for addStatics

#include "BitmapMovieInstance.h"

#include <vector>
#include <sstream>
#include <algorithm>

namespace gnash {

namespace {

    as_value BitmapData_applyFilter(const fn_call& fn);
    as_value BitmapData_clone(const fn_call& fn);
    as_value BitmapData_colorTransform(const fn_call& fn);
    as_value BitmapData_copyChannel(const fn_call& fn);
    as_value BitmapData_copyPixels(const fn_call& fn);
    as_value BitmapData_dispose(const fn_call& fn);
    as_value BitmapData_draw(const fn_call& fn);
    as_value BitmapData_fillRect(const fn_call& fn);
    as_value BitmapData_floodFill(const fn_call& fn);
    as_value BitmapData_generateFilterRect(const fn_call& fn);
    as_value BitmapData_getColorBoundsRect(const fn_call& fn);
    as_value BitmapData_getPixel(const fn_call& fn);
    as_value BitmapData_getPixel32(const fn_call& fn);
    as_value BitmapData_hitTest(const fn_call& fn);
    as_value BitmapData_merge(const fn_call& fn);
    as_value BitmapData_noise(const fn_call& fn);
    as_value BitmapData_paletteMap(const fn_call& fn);
    as_value BitmapData_perlinNoise(const fn_call& fn);
    as_value BitmapData_pixelDissolve(const fn_call& fn);
    as_value BitmapData_scroll(const fn_call& fn);
    as_value BitmapData_setPixel(const fn_call& fn);
    as_value BitmapData_setPixel32(const fn_call& fn);
    as_value BitmapData_threshold(const fn_call& fn);
    as_value BitmapData_height(const fn_call& fn);
    as_value BitmapData_rectangle(const fn_call& fn);
    as_value BitmapData_transparent(const fn_call& fn);
    as_value BitmapData_width(const fn_call& fn);
    as_value BitmapData_loadBitmap(const fn_call& fn);
    as_value BitmapData_ctor(const fn_call& fn);

    void attachBitmapDataInterface(as_object& o);
    void attachBitmapDataStaticProperties(as_object& o);
    as_object* getBitmapDataInterface();
    as_value get_flash_display_bitmap_data_constructor(const fn_call& fn);

}


BitmapData_as::BitmapData_as(size_t width, size_t height,
	              bool transparent, boost::uint32_t fillColor)
		:
    as_object(getBitmapDataInterface()),
    _width(width),
    _height(height),
    _transparent(transparent),
    _bitmapData(width * height, fillColor + (0xff << 24))
{
}

void
BitmapData_as::markReachableResources() const
{
    std::for_each(_attachedBitmaps.begin(), _attachedBitmaps.end(),
            std::mem_fun(&character::setReachable));
}

void
BitmapData_as::update(const boost::uint8_t* data)
{
    for (size_t i = 0; i < _width * _height; ++i) {
        boost::uint32_t pixel = (*(data++) << 16);
        pixel |= (*(data++) << 8);
        pixel |= (*(data++));
        pixel |= (0xff << 24);
        _bitmapData[i] = pixel;
    }
}
   

void
BitmapData_as::updateAttachedBitmaps()
{
    log_debug("Updating %d attached bitmaps", _attachedBitmaps.size());
    std::for_each(_attachedBitmaps.begin(), _attachedBitmaps.end(),
            std::mem_fun(&Bitmap::update));
}

boost::int32_t
BitmapData_as::getPixel(int x, int y, bool transparency) const
{

    // A value of 0, 0 is inside the bitmap.
    if (x < 0 || y < 0) return 0;
    
    // A value of _width, _height is outside the bitmap.
    if (static_cast<size_t>(x) >= _width || static_cast<size_t>(y) >= _height) {
        return 0;
    }

    const size_t pixelIndex = y * _width + x;

    assert ( pixelIndex < _bitmapData.size());
    
    const boost::uint32_t pixel = _bitmapData[pixelIndex];
    
    if (transparency)
    {
        return static_cast<boost::int32_t>(pixel);
    }
    
    // Without transparency
    return static_cast<boost::int32_t>(pixel & 0x00ffffff);

}

void
BitmapData_as::fillRect(int x, int y, int w, int h, boost::uint32_t color)
{

    GNASH_REPORT_FUNCTION;

    // The bitmap has been "disposed".
    if (_bitmapData.empty()) return;
    assert (_bitmapData.size() == _width * _height);

    if (w < 0 || h < 0) return;

    // Nothing to do if x or y are outside the image (negative height
    // or width are not allowed). The cast to int is fine as neither
    // dimension can be more than 2880 pixels.
    if (x >= static_cast<int>(_width) || y >= static_cast<int>(_height)) return;

    // If x or y is less than 0, make a rectangle of the
    // intersection with the bitmap.    
    if (x < 0)
    {
        w += x;
        x = 0;
    }

    if (y < 0)
    {
        h += y;
        y = 0;
    }

    // Make sure that the rectangle has some area in the 
    // bitmap and that its bottom corner is within the
    // the bitmap.    
    if (w <= 0 || h <= 0) return;

    w = std::min<size_t>(_width - x, w);
    h = std::min<size_t>(_height - y, h);
    
    BitmapArray::iterator it = _bitmapData.begin() + y * _width;
    
    // This cannot be past .end() because h + y is no larger than the
    // height of the image.
    const BitmapArray::iterator e = it + _width * h;
    
    // Make colour non-transparent if the image doesn't support it.
    if (!_transparent) color |= 0xff000000;
    
    while (it != e)
    {

        // Fill from x for the width of the rectangle.
        std::fill_n(it + x, w, color);

        // Move to the next line
        std::advance(it, _width);

    }

    updateAttachedBitmaps();

}

void
BitmapData_as::dispose()
{
    _bitmapData.clear();
    updateAttachedBitmaps();
}

as_function*
getFlashDisplayBitmapDataConstructor()
{
    static builtin_function* cl = NULL;
    if ( ! cl )
    {
        cl=new builtin_function(&BitmapData_ctor, getBitmapDataInterface());
        VM::get().addStatic(cl);
        attachBitmapDataStaticProperties(*cl);
    }
    return cl;
}


// extern 
void BitmapData_class_init(as_object& where)
{

    string_table& st = where.getVM().getStringTable();
	// Register _global.BitmapData
	where.init_destructive_property(st.find("BitmapData"),
	                get_flash_display_bitmap_data_constructor);
}


namespace {

as_value
BitmapData_applyFilter(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_clone(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_colorTransform(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_copyChannel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_copyPixels(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_dispose(const fn_call& fn)
{
    // Should free the memory storing the bitmap.
    // All properties afterwards are -1 (even the rectangle)
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
    ptr->dispose();
	return as_value();
}

as_value
BitmapData_draw(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr =
        ensureType<BitmapData_as>(fn.this_ptr);

    std::ostringstream os;
    fn.dump_args(os);
    log_unimpl("BitmapData.draw(%s) called", os.str());

    if (!fn.nargs) {
        //log error
        return as_value();
    }

    MovieClip* mc = fn.arg(0).to_sprite();

    if (!mc) {
        // log error
        return as_value();
    }

    /// TODO: pass the other arguments for transform.
    std::auto_ptr<GnashImage> im = mc->drawToBitmap();

    if (!im.get()) { 
        // log error
        return as_value();
    }

    const size_t width = im->width();
    const size_t height = im->height();

    if (width > 2880 || height > 2880) {
        log_error("Height (%d) or width (%d) exceed 2880", height, width);
        return as_value();
    }

    ptr->update(im->data());

	return as_value();
}

as_value
BitmapData_fillRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);

    if (fn.nargs < 2) return as_value();
    
    const as_value& arg = fn.arg(0);
    
    if ( ! arg.is_object() )
    {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.deltaTransformPoint(%s): needs an object", ss.str());
        );
        return as_value();
    }

    // This can be any object with the right properties.   
    as_object* obj = arg.to_object().get();
    assert(obj);
    
    as_value x, y, w, h;
    
    obj->get_member(NSV::PROP_X, &x);
    obj->get_member(NSV::PROP_Y, &y);
    obj->get_member(NSV::PROP_WIDTH, &w);
    obj->get_member(NSV::PROP_HEIGHT, &h);    

    boost::uint32_t color = fn.arg(1).to_int();
       
    ptr->fillRect(x.to_int(), y.to_int(), w.to_int(), h.to_int(), color);
    
	return as_value();
}


// Fills the bitmap with a colour starting at point x, y.
as_value
BitmapData_floodFill(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_generateFilterRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_getColorBoundsRect(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_getPixel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr =
        ensureType<BitmapData_as>(fn.this_ptr);

    if (fn.nargs < 2)
    {
        return as_value();
    }
    
    // TODO: what happens when the pixel is outside the image?
    
    int x = fn.arg(0).to_int();
    int y = fn.arg(1).to_int();
    
    return ptr->getPixel(x, y, false);
}

as_value
BitmapData_getPixel32(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr =
        ensureType<BitmapData_as>(fn.this_ptr);

    if (fn.nargs < 2)
    {
        return as_value();
    }
    
    // TODO: what happens when the pixel is outside the image?
    
    int x = fn.arg(0).to_int();
    int y = fn.arg(1).to_int();
    
    return ptr->getPixel(x, y, true);
}


as_value
BitmapData_hitTest(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_merge(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_noise(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_paletteMap(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_perlinNoise(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_pixelDissolve(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_scroll(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_setPixel(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_setPixel32(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_threshold(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
BitmapData_height(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns the immutable height of the bitmap or -1 if dispose() has been called.
    if (ptr->getBitmapData().empty()) return -1;
	return as_value(ptr->getHeight());
}

as_value
BitmapData_rectangle(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);

    // Returns the immutable rectangle of the bitmap or -1 if dispose()
    // has been called.
    if (ptr->getBitmapData().empty()) return -1;

	boost::intrusive_ptr<as_object> obj = init_Rectangle_instance();

	obj->set_member(NSV::PROP_X, 0.0);
	obj->set_member(NSV::PROP_Y, 0.0);
	obj->set_member(NSV::PROP_WIDTH, ptr->getWidth());
	obj->set_member(NSV::PROP_HEIGHT, ptr->getHeight());

	return as_value(obj.get()); // will keep alive
}

as_value
BitmapData_transparent(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns whether bitmap is transparent or -1 if dispose() has been called.
    if (ptr->getBitmapData().empty()) return -1;
	return as_value(ptr->isTransparent());
}

as_value
BitmapData_width(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns the immutable width of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->getBitmapData().empty()) return -1;
	return as_value(ptr->getWidth());
}


as_value
BitmapData_loadBitmap(const fn_call& fn)
{
	boost::intrusive_ptr<BitmapData_as> ptr = ensureType<BitmapData_as>(fn.this_ptr);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


as_value
get_flash_display_bitmap_data_constructor(const fn_call& /*fn*/)
{
    log_debug("Loading flash.display.BitmapData class");
    return getFlashDisplayBitmapDataConstructor();
}

as_value
BitmapData_ctor(const fn_call& fn)
{

	if ( fn.nargs < 2)
	{
	    // TODO: should fail if not enough arguments are passed.
        return as_value();
	}

    size_t width, height;
    bool transparent = true;
    boost::uint32_t fillColor = 0xffffff;

    switch (fn.nargs)
    {
        default:
            // log AS coding error
        case 4:
            fillColor = fn.arg(3).to_int();
        case 3:
            transparent = fn.arg(2).to_bool();
        case 2:
            // Is to_int correct?
            height = fn.arg(1).to_int();
            width = fn.arg(0).to_int();
            break;
    }
    
    // FIXME: Should fail to construct the object.
    if (width > 2880 || height > 2880) return as_value();
    if (width < 1 || height < 1) return as_value();

    boost::intrusive_ptr<BitmapData_as> obj =
                new BitmapData_as(width, height, transparent, fillColor);

	return as_value(obj.get()); // will keep alive
}

void
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
    o.init_property("height", BitmapData_height, BitmapData_height);
    o.init_property("rectangle", BitmapData_rectangle, BitmapData_rectangle);
    o.init_property("transparent", BitmapData_transparent, BitmapData_transparent);
    o.init_property("width", BitmapData_width, BitmapData_width);

}

void
attachBitmapDataStaticProperties(as_object& o)
{
   
    o.init_member("loadBitmap", new builtin_function(BitmapData_loadBitmap));
}

as_object*
getBitmapDataInterface()
{
	static boost::intrusive_ptr<as_object> o;

	if ( ! o )
	{
		// TODO: check if this class should inherit from Object
		//       or from a different class
		o = new as_object(getObjectInterface());
		VM::get().addStatic(o.get());

		attachBitmapDataInterface(*o);

	}

	return o.get();
}

} // anonymous namespace
} // end of gnash namespace
