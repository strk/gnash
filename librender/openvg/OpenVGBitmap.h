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
    enum bitmap_wrap_mode { WRAP_REPEAT, WRAP_CLAMP };
    
    OpenVGBitmap(std::auto_ptr<image::GnashImage> im);
  
    OpenVGBitmap(std::auto_ptr<image::GnashImage> im,
                 VGImageFormat pixelformat, VGPaint vgpaint);
    ~OpenVGBitmap();

    void dispose()  { _image.reset(); }
    bool disposed() const { return !_image.get(); }

    image::GnashImage& image();
    
    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;
    // Accessors for the GnashImage internal data
    int getWidth() { return _image->width(); };
    int getHeight() { return _image->height(); };
    boost::uint8_t *getData() const { return _image->begin(); };

private:    
    boost::scoped_ptr<image::GnashImage> _image;
    VGImageFormat   _pixel_format;
    mutable VGImage _vgimage;
    VGPaint         _vgpaint;
    int             _tex_size;
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_OPENVG_BITMAP_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
