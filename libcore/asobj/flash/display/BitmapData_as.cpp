// BitmapData_as.cpp:  ActionScript "BitmapData" class, for Gnash.
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

#include "BitmapData_as.h"

#include <vector>
#include <sstream>
#include <algorithm>
#include <queue>

#include "MovieClip.h"
#include "GnashImage.h"
#include "DisplayObject.h"
#include "as_object.h"
#include "log.h"
#include "fn_call.h"
#include "Global_as.h"
#include "smart_ptr.h" 
#include "GnashException.h" 
#include "VM.h" 
#include "Renderer.h"
#include "RunResources.h"
#include "namedStrings.h"
#include "Movie.h"
#include "movie_definition.h"
#include "Transform.h"
#include "ASConversions.h"
#include "flash/geom/ColorTransform_as.h"
#include "NativeFunction.h"

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
    as_value bitmapdata_compare(const fn_call& fn);
    as_value bitmapdata_ctor(const fn_call& fn);

    void attachBitmapDataInterface(as_object& o);
    void attachBitmapDataStaticProperties(as_object& o);
    as_value get_flash_display_bitmap_data_constructor(const fn_call& fn);

    BitmapData_as::iterator pixelAt(const BitmapData_as& bd, size_t x,
            size_t y);

    /// Get the overlapping part of a rectangle and a Bitmap
    //
    /// The values are adjusted so that the rectangle is wholly inside the
    /// bitmap. If no part of the rectangle overlaps, either w or h
    /// will be 0.
    //
    /// @param x    The x co-ordinate of the top left corner.
    /// @param y    The y co-ordinate of the top left corner.
    /// @param w    The width of the rectangle.
    /// @param h    The height of the rectangle.
    void adjustRect(int& x, int& y, int& w, int& h, BitmapData_as& b);

    boost::uint32_t setChannel(boost::uint32_t targ, boost::uint8_t bitmask,
            boost::uint8_t value);

    boost::uint8_t getChannel(boost::uint32_t src, boost::uint8_t bitmask);

    inline bool oneBitSet(boost::uint8_t mask) {
        return mask == (mask & -mask);
    }
}

BitmapData_as::BitmapData_as(as_object* owner,
        std::auto_ptr<image::GnashImage> im)
   
    :
    _owner(owner),
    _cachedBitmap(0)
{
    assert(im->width() <= 2880);
    assert(im->width() <= 2880);
    
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
}

void
BitmapData_as::setPixel32(size_t x, size_t y, boost::uint32_t color) const
{
    if (disposed()) return;
    if (x >= width() || y >= height()) return;

    iterator it = pixelAt(*this, x, y);
    *it = color;
}

void
BitmapData_as::setPixel(size_t x, size_t y, boost::uint32_t color) const
{
    if (disposed()) return;
    if (x >= width() || y >= height()) return;

    iterator it = pixelAt(*this, x, y);
    const boost::uint32_t val = *it;
    *it = (color & 0xffffff) | (val & 0xff000000);
}

void
BitmapData_as::updateObjects()
{
    std::for_each(_attachedObjects.begin(), _attachedObjects.end(),
            std::mem_fun(&DisplayObject::update));
}

boost::uint32_t
BitmapData_as::getPixel(size_t x, size_t y) const
{
    if (disposed()) return 0;
    if (x >= width() || y >= height()) return 0;
    return *pixelAt(*this, x, y);
}


void
BitmapData_as::fillRect(int x, int y, int w, int h, boost::uint32_t color)
{
    if (disposed()) return;

    adjustRect(x, y, w, h, *this);

    // Make sure that the rectangle has some area in the 
    // bitmap and that its bottom corner is within the
    // the bitmap.    
    if (w == 0 || h == 0) return;
    
    iterator it = begin() + y * width();
    iterator e = it + width() * h;
    
    assert(e <= end());

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

void
BitmapData_as::draw(MovieClip& mc, const Transform& transform)
{
    if (disposed()) return;

    image::GnashImage& im = *data();

    Renderer* base = getRunResources(*_owner).renderer();
    if (!base) {
        log_debug(_("BitmapData.draw() called without an active renderer"));
        return;
    }

    Renderer::Internal in(*base, im);
    Renderer* internal = in.renderer();
    if (!internal) {
        log_debug(_("Current renderer does not support internal rendering"));
        return;
    }

    mc.draw(*internal, transform);
    updateObjects();
}

void
BitmapData_as::floodFill(size_t startx, size_t starty, boost::uint32_t old,
        boost::uint32_t fill)
{
    if (startx >= width() || starty >= height()) return;

    // We never compare alpha for RGB images.
    if (!transparent()) fill |= 0xff000000;
    if (old == fill) return;

    std::queue<std::pair<size_t, size_t> > pixelQueue;
    pixelQueue.push(std::make_pair(startx, starty));

    while (!pixelQueue.empty()) {

        const std::pair<size_t, size_t>& p = pixelQueue.front();
        const size_t x = p.first;
        const size_t y = p.second;

        pixelQueue.pop();

        iterator pix = pixelAt(*this, x, y);
        assert(pix != end());

        if (*pix != old) continue;

        // Go east!
        iterator east(pix);
        if (x + 1 < width()) {
            ++east;
            const iterator eaststop(pix + (width() - x));
            while (east != eaststop && *east == old) ++east;
            std::fill(pix, east, fill);
        }
        size_t edone = (east - pix);
        if (!edone) ++edone;

        // Add north pixels
        if (y > 0) {
            const size_t ny = y - 1;
            for (size_t nx = x; nx != (x + edone); ++nx) {
                if (*pixelAt(*this, nx, ny) == old) {
                    pixelQueue.push(std::make_pair(nx, ny));
                }
            }
        }

        // Go west!
        iterator west(pix);
        if (x > 0) {
            --west;
            const iterator weststop(pix - x);
            while (west != weststop && *west == old) --west;
            std::fill(west + 1, pix, fill);
        }
        size_t wdone = (pix - west);
        if (!wdone) ++wdone;
         
        // Add south pixels
        if (y + 1 < height()) {
            const size_t sy = y + 1;
            for (size_t sx = x; sx != x - wdone; --sx) {
                if (*pixelAt(*this, sx, sy) == old) {
                    pixelQueue.push(std::make_pair(sx, sy));
                }
            }
        }

    }

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

void
registerBitmapDataNative(as_object& global)
{
    VM& vm = getVM(global);
    vm.registerNative(bitmapdata_getPixel, 1100, 1);
    vm.registerNative(bitmapdata_setPixel, 1100, 2);
    vm.registerNative(bitmapdata_fillRect, 1100, 3);
    vm.registerNative(bitmapdata_copyPixels, 1100, 4);
    vm.registerNative(bitmapdata_applyFilter, 1100, 5);
    vm.registerNative(bitmapdata_scroll, 1100, 6);
    vm.registerNative(bitmapdata_threshold, 1100, 7);
    vm.registerNative(bitmapdata_draw, 1100, 8);
    vm.registerNative(bitmapdata_pixelDissolve, 1100, 9);
    vm.registerNative(bitmapdata_getPixel32, 1100, 10);
    vm.registerNative(bitmapdata_setPixel32, 1100, 11);
    vm.registerNative(bitmapdata_floodFill, 1100, 12);
    vm.registerNative(bitmapdata_getColorBoundsRect, 1100, 13);
    vm.registerNative(bitmapdata_perlinNoise, 1100, 14);
    vm.registerNative(bitmapdata_colorTransform, 1100, 15);
    vm.registerNative(bitmapdata_hitTest, 1100, 16);
    vm.registerNative(bitmapdata_paletteMap, 1100, 17);
    vm.registerNative(bitmapdata_merge, 1100, 18);
    vm.registerNative(bitmapdata_noise, 1100, 19);
    vm.registerNative(bitmapdata_copyChannel, 1100, 20);
    vm.registerNative(bitmapdata_clone, 1100, 21);
    vm.registerNative(bitmapdata_dispose, 1100, 22);
    vm.registerNative(bitmapdata_generateFilterRect, 1100, 23);
    vm.registerNative(bitmapdata_compare, 1100, 24);
    
    vm.registerNative(bitmapdata_width, 1100, 100);
    vm.registerNative(bitmapdata_height, 1100, 101);
    vm.registerNative(bitmapdata_rectangle, 1100, 102);
    vm.registerNative(bitmapdata_transparent, 1100, 103);
    
    vm.registerNative(bitmapdata_loadBitmap, 1100, 40);

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
	as_object* obj = ensure<ValidThis>(fn);
	BitmapData_as* bm = ensure<ThisIsNative<BitmapData_as> >(fn);
    if (bm->disposed()) return as_value();

    const size_t width = bm->width();
    const size_t height = bm->height();

    std::auto_ptr<image::GnashImage> im;
    if (bm->transparent()) {
        im.reset(new image::ImageRGBA(width, height));
    }
    else {
        im.reset(new image::ImageRGB(width, height));
    }
    // Note that it would be much faster to copy the pixels, but BitmapData
    // currently doesn't expose a way to do this.
    std::copy(bm->begin(), bm->end(), image::begin<image::ARGB>(*im));

    Global_as& gl = getGlobal(fn);
    as_object* ret = createObject(gl);
    const as_value& proto = getMember(*obj, NSV::PROP_uuPROTOuu);
    if (proto.is_object()) {
        ret->set_member(NSV::PROP_uuPROTOuu, proto);
    }

    ret->setRelay(new BitmapData_as(ret, im));

	return as_value(ret);
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

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 5) {
        // log error
        return as_value();
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* source;
    if (!isNativeType(o, source) || source->disposed()) {
        // First argument is not a BitmapData or is disposed.
        return as_value();
    }

    as_object* rect = toObject(fn.arg(1), getVM(fn));
    if (!rect) {
        // Second argument is not an object
        return as_value();
    }

    as_value x, y, w, h;
    
    rect->get_member(NSV::PROP_X, &x);
    rect->get_member(NSV::PROP_Y, &y);
    rect->get_member(NSV::PROP_WIDTH, &w);
    rect->get_member(NSV::PROP_HEIGHT, &h);    
    
    as_object* destpoint = toObject(fn.arg(2), getVM(fn));
    as_value px, py;
    
    destpoint->get_member(NSV::PROP_X, &px);
    destpoint->get_member(NSV::PROP_Y, &py);

    // TODO: check what should happen if the argument overflows or
    // is negative (currently it is truncated and made positive.

    // The source channel mask
    const boost::uint8_t srcchans = 
        std::abs(toInt(fn.arg(3), getVM(fn))) & 15;

    // The destination channel mask
    const boost::uint8_t destchans = 
        std::abs(toInt(fn.arg(4), getVM(fn))) & 15;

    // If more than one destination channel is specified,
    // nothing happens.
    if (!oneBitSet(destchans)) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.copyChannel(). Multiple "
                "destination channels are not supported");
        );
        return as_value();
    }

    const bool multiple = !oneBitSet(srcchans);

    // Find true source rect and true dest rect.
    int sourceX = toInt(x, getVM(fn));
    int sourceY = toInt(y, getVM(fn));
    int sourceW = toInt(w, getVM(fn));
    int sourceH = toInt(h, getVM(fn));

    int destX = toInt(px, getVM(fn));
    int destY = toInt(py, getVM(fn));

    // Any part of the source rect that is not in the image (i.e.
    // above or left) is concatenated to the destination offset.
    if (sourceX < 0) destX -= sourceX;
    if (sourceY < 0) destY -= sourceY;

    adjustRect(sourceX, sourceY, sourceW, sourceH, *source);
    if (sourceW == 0 || sourceH == 0) {
        // The source rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.copyChannel(): no part of source rectangle"
                "overlaps with the source BitmapData");
        );
        return as_value();
    }

    // The dest width starts the same as the adjusted source width.
    int destW = sourceW;
    int destH = sourceH;

    adjustRect(destX, destY, destW, destH, *ptr);
    if (destW == 0 || destH == 0) {
        // The target rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.copyPixels(): destination area is "
                "wholly outside the destination BitmapData");
        );
        return as_value();
    }

    BitmapData_as::iterator targ = pixelAt(*ptr, destX, destY);
    BitmapData_as::iterator src = pixelAt(*source, sourceX, sourceY);

    // Just being careful...
    assert(sourceX + destW <= static_cast<int>(source->width()));
    assert(sourceY + destH <= static_cast<int>(source->height()));
    assert(destX + destW <= static_cast<int>(ptr->width()));
    assert(destY + destH <= static_cast<int>(ptr->height()));

    // Copy for the width and height of the *dest* image.
    // We have already ensured that the copied area
    // is inside both bitmapdatas.
    for (int i = 0; i < destH; ++i) {

        BitmapData_as::iterator s = src;
        BitmapData_as::iterator d = targ;
        for (int j = 0; j < destW; ++j, ++s, ++d) {

            // If multiple source channels, we set the destination channel
            // to black. Else to the value of the requested channel.
            const boost::uint8_t val = multiple ? 0 : getChannel(*s, srcchans);
            *d = setChannel(*d, destchans, val);

        }
        targ += ptr->width();
        src += source->width();
    }

    ptr->updateObjects();

	return as_value();
}

boost::uint8_t
getChannel(boost::uint32_t src, boost::uint8_t bitmask)
{
    if (bitmask & 1) {
        // Red
        return (src >> 16) & 0xff;
    }
    if (bitmask & 2) {
        // Green
        return (src >> 8) & 0xff;
    }
    if (bitmask & 4) {
        // Blue
        return src & 0xff;
    }
    if (bitmask & 8) {
        // Alpha
        return src >> 24;
    }
    return 0;
}

boost::uint32_t
setChannel(boost::uint32_t targ, boost::uint8_t bitmask, boost::uint8_t value)
{
    boost::uint32_t bytemask = 0;
    boost::uint32_t valmask = 0;
    if (bitmask & 1) {
        // Red
        bytemask = 0xff0000;
        valmask = value << 16;
    }
    else if (bitmask & 2) {
        // Green
        bytemask = 0xff00;
        valmask = value << 8;
    }
    else if (bitmask & 4) {
        // Blue
        bytemask = 0xff;
        valmask = value;
    }
    else if (bitmask & 8) {
        // Alpha
        bytemask = 0xff000000;
        valmask = value << 24;
    }
    targ &= ~bytemask;
    targ |= valmask;
    return targ;
}


// sourceBitmap: BitmapData,
// sourceRect: Rectangle,
// destPoint: Point,
// [alphaBitmap: BitmapData],
// [alphaPoint: Point],
// [mergeAlpha: Boolean]
as_value
bitmapdata_copyPixels(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    if (ptr->disposed()) return as_value();

    if (fn.nargs < 3) {
        // Require three arguments? (TODO: check).
        return as_value();
    }
    if (fn.nargs > 3) {
        LOG_ONCE(log_unimpl("BitmapData.copyPixels(): arguments after "
                    "the first three are discarded"));
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    BitmapData_as* source;
    if (!isNativeType(o, source) || source->disposed()) {
        // First argument is not a BitmapData or is disposed.
        return as_value();
    }

    as_object* rect = toObject(fn.arg(1), getVM(fn));
    if (!rect) {
        // Second argument is not an object
        return as_value();
    }

    as_value x, y, w, h;
    
    rect->get_member(NSV::PROP_X, &x);
    rect->get_member(NSV::PROP_Y, &y);
    rect->get_member(NSV::PROP_WIDTH, &w);
    rect->get_member(NSV::PROP_HEIGHT, &h);    
    
    as_object* destpoint = toObject(fn.arg(2), getVM(fn));
    as_value px, py;
    
    destpoint->get_member(NSV::PROP_X, &px);
    destpoint->get_member(NSV::PROP_Y, &py);

    // TODO: remaining arguments.

    // Find true source rect and true dest rect.
    int sourceX = toInt(x, getVM(fn));
    int sourceY = toInt(y, getVM(fn));
    int sourceW = toInt(w, getVM(fn));
    int sourceH = toInt(h, getVM(fn));

    int destX = toInt(px, getVM(fn));
    int destY = toInt(py, getVM(fn));

    // Any part of the source rect that is not in the image (i.e.
    // above or left) is concatenated to the destination offset.
    if (sourceX < 0) destX -= sourceX;
    if (sourceY < 0) destY -= sourceY;

    adjustRect(sourceX, sourceY, sourceW, sourceH, *source);
    if (sourceW == 0 || sourceH == 0) {
        // The source rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.copyPixels(): no part of source rectangle"
                "overlaps with the source BitmapData");
        );
        return as_value();
    }

    // The dest width starts the same as the adjusted source width.
    int destW = sourceW;
    int destH = sourceH;

    adjustRect(destX, destY, destW, destH, *ptr);
    if (destW == 0 || destH == 0) {
        // The target rect does not overlap with source bitmap
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.copyPixels(): destination area is "
                "wholly outside the destination BitmapData");
        );
        return as_value();
    }

    BitmapData_as::iterator targ = pixelAt(*ptr, destX, destY);
    BitmapData_as::iterator src = pixelAt(*source, sourceX, sourceY);

    // Just being careful...
    assert(sourceX + destW <= static_cast<int>(source->width()));
    assert(sourceY + destH <= static_cast<int>(source->height()));
    assert(destX + destW <= static_cast<int>(ptr->width()));
    assert(destY + destH <= static_cast<int>(ptr->height()));

    // Copy for the width and height of the *dest* image.
    // We have already ensured that the copied area
    // is inside both bitmapdatas.
    for (int i = 0; i < destH; ++i) {
        std::copy(src, src + destW, targ);
        targ += ptr->width();
        src += source->width();
    }

    ptr->updateObjects();

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

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("BitmapData.draw(%s) requires at least one argument",
                ss.str());
        );
        return as_value();
    }

    as_object* o = toObject(fn.arg(0), getVM(fn));
    MovieClip* mc = get<MovieClip>(o);
    if (!mc) {
        IF_VERBOSE_ASCODING_ERRORS(
            std::ostringstream ss;
            fn.dump_args(ss);
            log_aserror("BitmapData.draw(%s): first argument must be a "
                "MovieClip", ss.str());
        );
        return as_value();
    }

    Transform t;
    if (fn.nargs > 1) {
        as_object* o = toObject(fn.arg(1), getVM(fn));
        if (o) t.matrix = toSWFMatrix(*o);
    }
    if (fn.nargs > 2) {
        as_object* o = toObject(fn.arg(2), getVM(fn));
        ColorTransform_as* tr;
        if (isNativeType(o, tr)) {
            t.colorTransform = toCxForm(*tr);
        }
    }

    ptr->draw(*mc, t);
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
            log_aserror("BitmapData.fillRect(%s): needs an object", ss.str());
        );
        return as_value();
    }

    // This can be any object with the right properties.   
    as_object* obj = toObject(arg, getVM(fn));
    assert(obj);
    
    as_value x, y, w, h;
    
    obj->get_member(NSV::PROP_X, &x);
    obj->get_member(NSV::PROP_Y, &y);
    obj->get_member(NSV::PROP_WIDTH, &w);
    obj->get_member(NSV::PROP_HEIGHT, &h);    

    const boost::uint32_t color = toInt(fn.arg(1), getVM(fn));
       
    ptr->fillRect(toInt(x, getVM(fn)), toInt(y, getVM(fn)),
            toInt(w, getVM(fn)), toInt(h, getVM(fn)), color);
    
	return as_value();
}


// Fills the bitmap with a colour starting at point x, y.
as_value
bitmapdata_floodFill(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    if (fn.nargs < 3) {
        return as_value();
    }

    if (ptr->disposed()) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("floodFill called on disposed BitmapData!");
        );
        return as_value();
    }
    
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));

    if (x < 0 || y < 0) {
        return as_value();
    }

    const boost::uint32_t fill = toInt(fn.arg(2), getVM(fn));
    const boost::uint32_t old = *pixelAt(*ptr, x, y);

    // This checks whether the colours are the same.
    ptr->floodFill(x, y, old, fill);
    
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
    
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));
    
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
    const int x = toInt(fn.arg(0), getVM(fn));
    const int y = toInt(fn.arg(1), getVM(fn));
    
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

    const double x = toNumber(fn.arg(0), getVM(fn));
    const double y = toNumber(fn.arg(1), getVM(fn));
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // Ignore any transparency here.
    const boost::uint32_t color = toInt(fn.arg(2), getVM(fn));

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

    const double x = toNumber(fn.arg(0), getVM(fn));
    const double y = toNumber(fn.arg(1), getVM(fn));
    if (isNaN(x) || isNaN(y) || x < 0 || y < 0) return as_value();
    if (x >= ptr->width() || y >= ptr->height()) {
        return as_value();
    }

    // TODO: multiply.
    const boost::uint32_t color = toInt(fn.arg(2), getVM(fn));

    ptr->setPixel32(x, y, color);

	return as_value();
}

as_value
bitmapdata_compare(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
	UNUSED(ptr);
	LOG_ONCE( log_unimpl (__FUNCTION__) );
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
bitmapdata_width(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns the immutable width of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->width());
}

as_value
bitmapdata_height(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns the immutable height of the bitmap or -1 if dispose() has
    // been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->height());
}

as_value
bitmapdata_transparent(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);
    
    // Returns whether bitmap is transparent or -1 if dispose() has been called.
    if (ptr->disposed()) return -1;
	return as_value(ptr->transparent());
}

as_value
bitmapdata_rectangle(const fn_call& fn)
{
	BitmapData_as* ptr = ensure<ThisIsNative<BitmapData_as> >(fn);

    // Returns the immutable rectangle of the bitmap or -1 if dispose()
    // has been called.
    if (ptr->disposed()) return -1;

    // If it's not found construction will fail.
    as_value rectangle(findObject(fn.env(), "flash.geom.Rectangle"));
    as_function* rectCtor = rectangle.to_function();

    if (!rectCtor) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("Failed to construct flash.geom.Rectangle!");
        );
        return -1;
    }

    fn_call::Args args;
    args += 0.0, 0.0, ptr->width(), ptr->height();

    as_object* newRect = constructInstance(*rectCtor, fn.env(), args);

    return as_value(newRect);
}

as_value
bitmapdata_loadBitmap(const fn_call& fn)
{
    // This is a static method, but still requires a parent object for
    // the prototype.
	as_object* ptr = ensure<ValidThis>(fn);

    if (!fn.nargs) {
        IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("BitmapData.loadBitmap requires one argument");
        );
        return as_value();
    }

    const std::string linkage = fn.arg(0).to_string();
    DisplayObject* tgt = fn.env().target();
    if (!tgt) return as_value();

    Movie* root = tgt->get_root();
    assert(root);

    const movie_definition* def = root->definition();

    const boost::uint16_t id = def->exportID(linkage);
    CachedBitmap* bit = def->getBitmap(id);

    if (!bit) return as_value();

    image::GnashImage& im = bit->image();
    const size_t width = im.width();
    const size_t height = im.height();

    if (width > 2880 || height > 2880) {
        return as_value();
    }
 
    std::auto_ptr<image::GnashImage> newImage;
    if (im.type() == image::TYPE_RGBA) {
        newImage.reset(new image::ImageRGBA(width, height));
    }
    else {
        newImage.reset(new image::ImageRGB(width, height));
    }
    
    // The properties come from the 'this' object.
    Global_as& gl = getGlobal(fn);
    as_object* ret = createObject(gl);
    ret->set_member(NSV::PROP_uuPROTOuu, getMember(*ptr, NSV::PROP_PROTOTYPE));
    
    newImage->update(im.begin());
    ret->setRelay(new BitmapData_as(ret, newImage));

	return as_value(ret);
}


as_value
get_flash_display_bitmap_data_constructor(const fn_call& fn)
{
    log_debug("Loading flash.display.BitmapData class");
    Global_as& gl = getGlobal(fn);
    as_object* proto = createObject(gl);
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

    size_t width = toInt(fn.arg(0), getVM(fn));
    size_t height = toInt(fn.arg(1), getVM(fn));
    const bool transparent = fn.nargs > 2 ? toBool(fn.arg(2), getVM(fn)) : true;
    boost::uint32_t fillColor = fn.nargs > 3 ? toInt(fn.arg(3), getVM(fn)) : 0xffffffff;
    
    if (width > 2880 || height > 2880 || width < 1 || height < 1) {
        IF_VERBOSE_ASCODING_ERRORS(
             log_aserror("BitmapData width and height must be between "
                 "1 and 2880. Will not construct a BitmapData");
        );
        throw ActionTypeError();
    }

    std::auto_ptr<image::GnashImage> im;
    if (transparent) {
        im.reset(new image::ImageRGBA(width, height));
    }
    else {
        im.reset(new image::ImageRGB(width, height));
    }

    // There is one special case for completely transparent colours. This
    // might be a part of a more general pre-treatment as other colours
    // vary slightly, but we haven't worked it out yet.
    if (transparent && !(fillColor & 0xff000000)) fillColor = 0;

    std::fill(image::begin<image::ARGB>(*im), image::end<image::ARGB>(*im),
            fillColor);

    ptr->setRelay(new BitmapData_as(ptr, im));

	return as_value(); 
}

void
attachBitmapDataInterface(as_object& o)
{
    const int flags = PropFlags::onlySWF8Up;

    VM& vm = getVM(o);
    o.init_member("getPixel", vm.getNative(1100, 1));
    o.init_member("setPixel", vm.getNative(1100, 2));
    o.init_member("fillRect", vm.getNative(1100, 3));
    o.init_member("copyPixels", vm.getNative(1100, 4));
    o.init_member("applyFilter", vm.getNative(1100, 5));
    o.init_member("scroll", vm.getNative(1100, 6));
    o.init_member("threshold", vm.getNative(1100, 7));
    o.init_member("draw", vm.getNative(1100, 8));
    o.init_member("pixelDissolve", vm.getNative(1100, 9));
    o.init_member("getPixel32", vm.getNative(1100, 10));
    o.init_member("setPixel32", vm.getNative(1100, 11));
    o.init_member("floodFill", vm.getNative(1100, 12));
    o.init_member("getColorBoundsRect", vm.getNative(1100, 13));
    o.init_member("perlinNoise", vm.getNative(1100, 14));
    o.init_member("colorTransform", vm.getNative(1100, 15));
    o.init_member("hitTest", vm.getNative(1100, 16));
    o.init_member("paletteMap", vm.getNative(1100, 17));
    o.init_member("merge", vm.getNative(1100, 18));
    o.init_member("noise", vm.getNative(1100, 19));
    o.init_member("copyChannel", vm.getNative(1100, 20));
    o.init_member("clone", vm.getNative(1100, 21));
    o.init_member("dispose", vm.getNative(1100, 22));
    o.init_member("generateFilterRect", vm.getNative(1100, 23));
    o.init_member("compare", vm.getNative(1100, 24));
    o.init_readonly_property("width", *vm.getNative(1100, 100), flags);
    o.init_readonly_property("height", *vm.getNative(1100, 101), flags);
    o.init_readonly_property("rectangle", *vm.getNative(1100, 102), flags);
    o.init_readonly_property("transparent", *vm.getNative(1100, 103), flags);

}

void
attachBitmapDataStaticProperties(as_object& o)
{
    VM& vm = getVM(o);

    o.init_member("loadBitmap", vm.getNative(1100, 40));

    o.init_member("RED_CHANNEL", 1.0);
    o.init_member("GREEN_CHANNEL", 2.0);
    o.init_member("BLUE_CHANNEL", 4.0);
    o.init_member("ALPHA_CHANNEL", 8.0);
}
    
BitmapData_as::iterator
pixelAt(const BitmapData_as& bd, size_t x, size_t y)
{
    if (x >= bd.width() || y >= bd.height()) return bd.end();
    return (bd.begin() + y * bd.width() + x);
}

void
adjustRect(int& x, int& y, int& w, int& h, BitmapData_as& b) 
{
    // No negative width or height
    if (w < 0 || h < 0) {
        w = 0;
        h = 0;
        return;
    }

    // No part of rect in bitmap
    if (x >= static_cast<int>(b.width()) || y >= static_cast<int>(b.height())) {
        w = 0;
        h = 0;
        return;
    }

    // Remove left excess
    if (x < 0) {
        w += x;
        x = 0;
        if (w < 0) w = 0;
    }

    // Remove top excess
    if (y < 0) {
        h += y;
        y = 0;
        if (h < 0) h = 0;
    }

    // Remove right and bottom excess after x and y adjustments.
    w = std::min<int>(b.width() - x, w);
    h = std::min<int>(b.height() - y, h);
}


} // anonymous namespace
} // end of gnash namespace
