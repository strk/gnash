// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef GNASH_OPENVG_BITMAP_H
#define GNASH_OPENVG_BITMAP_H

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "openvg/OpenVGRenderer.h"

namespace gnash {

namespace renderer {

namespace openvg {

/// This class is the OpenVG specific representation of a Gnash
/// Cached Bitmap.
class OpenVGBitmap : public CachedBitmap
{
public:
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode { WRAP_REPEAT, WRAP_FILL, WRAP_PAD, WRAP_REFLECT };
    
    OpenVGBitmap(VGPaint paint);
    OpenVGBitmap(CachedBitmap *bitmap, VGPaint vgpaint);
    OpenVGBitmap(image::GnashImage *im, VGPaint vgpaint);
    ~OpenVGBitmap();

    void dispose()  { _image.reset(); }
    bool disposed() const { return !_image.get(); }

    image::GnashImage& image();
    VGPaint &vgimage() { return _vgimage; };
    
    // Accessors for the GnashImage internal data
    VGPaint getFillPaint() const { return _vgpaint; }
    int getWidth() { return _image->width(); }
    int getHeight() { return _image->height(); }
    boost::uint8_t *getData() const { return _image->begin(); }

    /// OpenVG supports creating linear and gradient fills in hardware, so
    /// we want to use that instead of the existing way of calculating the
    /// gradient in software.
    ///
    /// @param x0 The X coordinate of the origin point
    /// @param y0 The Y coordinate of the origin point
    /// @param x1 The X coordinate of the opposite corner point
    /// @param y1 The Y coordinate of the opposite corner point
    /// @param incolor The base color of the gradient
    /// @param paint The VG paint surface
    OpenVGBitmap *createRadialBitmap(float x0, float y0, float x1,
                                     float y1, float radial,
                                     const rgba &incolor, VGPaint paint);
    OpenVGBitmap *createLinearBitmap(float x0, float y0, float x1, float y1,
                                     const rgba &incolor, const VGPaint paint);

    OpenVGBitmap *applyPatternBitmap(const gnash::SWFMatrix& matrix,
                                      bitmap_wrap_mode mode, VGPaint paint);
    
private:
    boost::scoped_ptr<image::GnashImage> _image;
    VGImageFormat   _pixel_format;
    VGImage         _vgimage;
    VGPaint         _vgpaint;
//    int             _tex_size;
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_OPENVG_BITMAP_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
