// BitmapData_as.cpp:  ActionScript "BitmapData" class, for Gnash.
//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#include "BitmapData_as.h"

#include <vector>
#include <sstream>
#include <algorithm>

#include "MovieClip.h"
#include "GnashImage.h"
#include "DisplayObject.h"
#include "as_object.h" // for inheritance
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" // for boost intrusive_ptr
#include "builtin_function.h" // need builtin_function
#include "GnashException.h" // for ActionException
#include "VM.h" // for addStatics
#include "Renderer.h"
#include "RunResources.h"
#include "namedStrings.h"

namespace gnash {

namespace {

    as_value bitmapdata_applyFilter(const fn_call& fn);
    as_value bitmapdata_clone(const fn_call& fn);
    as_value bitmapdata_colorTransform(const fn_call& fn);
    as_value bitmapdata_copyChannel(const fn_call& fn);
    as_value bitmapdata_copyPixels(const fn_call& fn);
    as_value bitmapdata_dispose(const fn_call& fn);
    as_value bitmapdata_draw(const fn_call& fn);
    as_value bitmapdata_fillRect(const fn_call& fn);
    as_value bitmapdata_floodFill(const fn_call& fn);
    as_value bitmapdata_generateFilterRect(const fn_call& fn);
    as_value bitmapdata_getColorBoundsRect(const fn_call& fn);
    as_value bitmapdata_getPixel(const fn_call& fn);
    as_value bitmapdata_getPixel32(const fn_call& fn);
    as_value bitmapdata_hitTest(const fn_call& fn);
    as_value bitmapdata_merge(const fn_call& fn);
    as_value bitmapdata_noise(const fn_call& fn);
    as_value bitmapdata_paletteMap(const fn_call& fn);
    as_value bitmapdata_perlinNoise(const fn_call& fn);
    as_value bitmapdata_pixelDissolve(const fn_call& fn);
    as_value bitmapdata_scroll(const fn_call& fn);
    as_value bitmapdata_setPixel(const fn_call& fn);
    as_value bitmapdata_setPixel32(const fn_call& fn);
    as_value bitmapdata_threshold(const fn_call& fn);
    as_value bitmapdata_height(const fn_call& fn);
    as_value bitmapdata_rectangle(const fn_call& fn);
    as_value bitmapdata_transparent(const fn_call& fn);
    as_value bitmapdata_width(const fn_call& fn);
    as_value bitmapdata_loadBitmap(const fn_call& fn);
    as_value bitmapdata_ctor(const fn_call& fn);

    void attachBitmapDataInterface(as_object& o);
    void attachBitmapDataStaticProperties(as_object& o);
    as_value get_flash_display_bitmap_data_constructor(const fn_call& fn);

}

BitmapData_as::BitmapData_as(as_object* owner, std::auto_ptr<GnashImage> im,
        boost::uint32_t fillColor)
    :
    _owner(owner),
    _cachedBitmap(0)
{
    assert(im->width() <= 2880);
    assert(im->width() <= 2880);

    std::fill(im->argb_begin(), im->argb_end(), fillColor | (0xff << 24));
    
    // If there is a renderer, cache the image there, otherwise we store it.
    Renderer* r = getRunResources(*_owner).renderer();
    if (r) _cachedBitmap = r->createCachedBitmap(im);
    else _image.reset(im.release());
}
    
void
BitmapData_as::setReachable() 
{
    std::for_each(_attachedObjects.begin(), _attachedObjects.end(),
            std::mem_fun(&DisplayObject::setReachable));
    _owner->setReachable();
    log_debug("BitmapData_as::setReachable");
}

void
BitmapData_as::setPixel32(size_t x, size_t y, boost::uint32_t color)
{
    if (disposed()) return;
    if (x >= width() || y >= height()) return;

    GnashImage::argb_iterator it = data()->argb_begin() + x * width() + y;
    *it = color;
}

void
BitmapData_as::setPixel(size_t x, size_t y, boost::uint32_t color)
{
    if (disposed()) return;
    if (x >= width() || y >= height()) return;

    GnashImage::argb_iterator it = data()->argb_begin() + x * width() + y;
    const boost::uint32_t val = it.toARGB();
    *it = (color & 0xffffff) | (val & 0xff000000);
}

void
BitmapData_as::updateObjects()
{
    log_debug("Updating %d attached objects", _attachedObjects.size());
    std::for_each(_attachedObjects.begin(), _attachedObjects.end(),
            std::mem_fun(&DisplayObject::update));
}

boost::uint32_t
BitmapData_as::getPixel(size_t x, size_t y) const
{
    if (disposed()) return 0;
    if (x >= width() || y >= height()) return 0;

    const size_t pixelIndex = y * width() + x;
    return (data()->argb_begin() + pixelIndex).toARGB();

}

void
BitmapData_as::fillRect(int x, int y, int w, int h, boost::uint32_t color)
{

    if (disposed()) return;

    if (w < 0 || h < 0) return;
    if (x >= static_cast<int>(width()) || y >= static_cast<int>(height())) {
        return;
    }

    // If x or y is less than 0, make a rectangle of the
    // intersection with the bitmap.    
    if (x < 0) {
        w += x;
        x = 0;
    }

    if (y < 0) {
        h += y;
        y = 0;
    }

    // Make sure that the rectangle has some area in the 
    // bitmap and that its bottom corner is within the
    // the bitmap.    
    if (w <= 0 || h <= 0) return;

    w = std::min<size_t>(width() - x, w);
    h = std::min<size_t>(height() - y, h);
    
    GnashImage::argb_iterator it = data()->argb_begin() + y * width();
    
    GnashImage::argb_iterator e = it + width() * h;
    
    while (it != e) {
        // Fill from x for the width of the rectangle.
        std::fill_n(it + x, w, color);
        it += width();
    }

    updateObjects();

}

void
BitmapData_as::dispose()
{
    if (_cachedBitmap) _cachedBitmap->dispose();
    _cachedBitmap = 0;
    _image.reset();
    updateObjects();
}

// extern 
void
bitmapdata_class_init(as_object& where, const ObjectURI& uri)
{
    // TODO: this may not be correct, but it should be enumerable.
    const int flags = 0;
	where.init_destructive_property(uri,
            get_flash_display_bitmap_data_constructor, flags);
}


namespace {

as_value
bitmapdata_applyFilter(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_clone(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_colorTransform(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_copyChannel(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_copyPixels(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_dispose(const fn_call& fn)
{
    // Should free the memory storing the bitmap.
    // All properties afterwards are -1 (even the rectangle)
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    ptr->dispose();
	return as_value();
}

as_value
bitmapdata_draw(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    UNUSED(ptr);

    std::ostringstream os;
    fn.dump_args(os);
    log_unimpl("BitmapData.draw(%s)", os.str());

    if (!fn.nargs) {
        //log error
        return as_value();
    }

    MovieClip* mc = fn.arg(0).toMovieClip();

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

    //ptr->update(im->data());

	return as_value();
}

as_value
bitmapdata_fillRect(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2) return as_value();
    
    const as_value& arg = fn.arg(0);
    
    if (!arg.is_object()) {
        /// Isn't an object...
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("Matrix.deltaTransformPoint(%s): needs an object",
                ss.str());
        );
        return as_value();
    }

    // This can be any object with the right properties.   
    as_object* obj = arg.to_object(getGlobal(fn));
    assert(obj);
    
    as_value x, y, w, h;
    
    obj->get_member(NSV::PROP_X, &x);
    obj->get_member(NSV::PROP_Y, &y);
    obj->get_member(NSV::PROP_WIDTH, &w);
    obj->get_member(NSV::PROP_HEIGHT, &h);    

    const boost::uint32_t color = toInt(fn.arg(1));
       
    ptr->fillRect(toInt(x), toInt(y), toInt(w), toInt(h), color);
    
	return as_value();
}


// Fills the bitmap with a colour starting at point x, y.
as_value
bitmapdata_floodFill(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_generateFilterRect(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_getColorBoundsRect(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_getPixel(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("getPixel called on disposed BitmapData!");
        );
        return as_value();
    }
    
    const int x = toInt(fn.arg(0));
    const int y = toInt(fn.arg(1));
    
    // Will return 0 if the pixel is outside the image or the image has
    // been disposed.
    return static_cast<boost::int32_t>(ptr->getPixel(x, y) & 0xffffff);
}

as_value
bitmapdata_getPixel32(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 2) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("getPixel32 called on disposed BitmapData!");
        );
        return as_value();
    }
    
    // TODO: what happens when the pixel is outside the image?
    const int x = toInt(fn.arg(0));
    const int y = toInt(fn.arg(1));
    
    return static_cast<boost::int32_t>(ptr->getPixel(x, y));
}


as_value
bitmapdata_hitTest(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_merge(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_noise(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_paletteMap(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_perlinNoise(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_pixelDissolve(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_scroll(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_setPixel(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 3) {
        return as_value();
    }

    const double x = fn.arg(0).to_number();
    const double y = fn.arg(1).to_number();
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // Ignore any transparency here.
    const boost::uint32_t color = toInt(fn.arg(2));

    ptr->setPixel(x, y, color);

	return as_value();
}

as_value
bitmapdata_setPixel32(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (fn.nargs < 3) {
        return as_value();
    }

    const double x = fn.arg(0).to_number();
    const double y = fn.arg(1).to_number();
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // TODO: multiply.
    const boost::uint32_t color = toInt(fn.arg(2));

    ptr->setPixel32(x, y, color);

	return as_value();
}

as_value
bitmapdata_threshold(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}

as_value
bitmapdata_height(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns the immutable height of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->height());
}

as_value
bitmapdata_rectangle(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Returns the immutable rectangle of the bitmap or -1 if dispose()
    // has been called.
    if (ptr->disposed()) return -1;

    // If it's not found construction will fail.
    as_value rectangle(fn.env().find_object("flash.geom.Rectangle"));
    boost::intrusive_ptr<as_function> rectCtor = rectangle.to_function();

    if (!rectCtor) {
        log_error("Failed to construct flash.geom.Rectangle!");
        return -1;
    }

    fn_call::Args args;
    args += 0.0, 0.0, ptr->width(), ptr->height();

    boost::intrusive_ptr<as_object> newRect =
            constructInstance(*rectCtor, fn.env(), args);

    return as_value(newRect.get());
}

as_value
bitmapdata_transparent(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns whether bitmap is transparent or -1 if dispose() has been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->transparent());
}

as_value
bitmapdata_width(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Read-only
    if (fn.nargs) return as_value();
    
    // Returns the immutable width of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->width());
}


as_value
bitmapdata_loadBitmap(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
	return as_value();
}


as_value
get_flash_display_bitmap_data_constructor(const fn_call& fn)
{
    log_debug("Loading flash.display.BitmapData class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = gl.createObject();
    attachBitmapDataInterface(*proto);
    as_object* cl = gl.createClass(&bitmapdata_ctor, proto);
    attachBitmapDataStaticProperties(*cl);
    return cl;
}

as_value
bitmapdata_ctor(const fn_call& fn)
{
    as_object* ptr = ensure<ValidThis>(fn);
	if (fn.nargs < 2) {
        IF_VERBOSE_ASCODING_ERRORS(
             log_aserror("BitmapData constructor requires at least two "
                 "arguments. Will not construct a BitmapData");
        );
        throw ActionTypeError();
	}

    size_t width = toInt(fn.arg(0));
    size_t height = toInt(fn.arg(1));
    bool transparent = fn.nargs > 2 ? fn.arg(2).to_bool() : true;
    boost::uint32_t fillColor = fn.nargs > 3 ? toInt(fn.arg(3)) : 0xffffff;
    
    if (width > 2880 || height > 2880 || width < 1 || height < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
             log_aserror("BitmapData width and height must be between "
                 "1 and 2880. Will not construct a BitmapData");
        );
        throw ActionTypeError();
    }

    std::auto_ptr<GnashImage> im;
    if (transparent) {
        im.reset(new ImageRGBA(width, height));
    }
    else {
        im.reset(new ImageRGB(width, height));
    }

    ptr->setRelay(new BitmapData_as(ptr, im, fillColor));

	return as_value(); 
}

void
attachBitmapDataInterface(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("applyFilter", gl.createFunction(bitmapdata_applyFilter));
    o.init_member("clone", gl.createFunction(bitmapdata_clone));
    o.init_member("colorTransform", gl.createFunction(
                bitmapdata_colorTransform));
    o.init_member("copyChannel", gl.createFunction(bitmapdata_copyChannel));
    o.init_member("copyPixels", gl.createFunction(bitmapdata_copyPixels));
    o.init_member("dispose", gl.createFunction(bitmapdata_dispose));
    o.init_member("draw", gl.createFunction(bitmapdata_draw));
    o.init_member("fillRect", gl.createFunction(bitmapdata_fillRect));
    o.init_member("floodFill", gl.createFunction(bitmapdata_floodFill));
    o.init_member("generateFilterRect", gl.createFunction(
                bitmapdata_generateFilterRect));
    o.init_member("getColorBoundsRect", gl.createFunction(
                bitmapdata_getColorBoundsRect));
    o.init_member("getPixel", gl.createFunction(bitmapdata_getPixel));
    o.init_member("getPixel32", gl.createFunction(bitmapdata_getPixel32));
    o.init_member("hitTest", gl.createFunction(bitmapdata_hitTest));
    o.init_member("merge", gl.createFunction(bitmapdata_merge));
    o.init_member("noise", gl.createFunction(bitmapdata_noise));
    o.init_member("paletteMap", gl.createFunction(bitmapdata_paletteMap));
    o.init_member("perlinNoise", gl.createFunction(bitmapdata_perlinNoise));
    o.init_member("pixelDissolve", gl.createFunction(
                bitmapdata_pixelDissolve));
    o.init_member("scroll", gl.createFunction(bitmapdata_scroll));
    o.init_member("setPixel", gl.createFunction(bitmapdata_setPixel));
    o.init_member("setPixel32", gl.createFunction(bitmapdata_setPixel32));
    o.init_member("threshold", gl.createFunction(bitmapdata_threshold));
    o.init_property("height", bitmapdata_height, bitmapdata_height);
    o.init_property("rectangle", bitmapdata_rectangle, bitmapdata_rectangle);
    o.init_property("transparent", bitmapdata_transparent,
            bitmapdata_transparent);
    o.init_property("width", bitmapdata_width, bitmapdata_width);

}

void
attachBitmapDataStaticProperties(as_object& o)
{
    Global_as& gl = getGlobal(o);
    o.init_member("loadBitmap", gl.createFunction(bitmapdata_loadBitmap));
}

} // anonymous namespace
} // end of gnash namespace
