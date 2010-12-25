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

///
/// Author: Visor <cutevisor@gmail.com>
///

#ifndef GNASH_RENDER_HANDLER_OVG_BITMAP_H
#define GNASH_RENDER_HANDLER_OVG_BITMAP_H

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"

namespace gnash {

namespace renderer {

namespace openvg {

class bitmap_info_ovg : public CachedBitmap
{
public:
  
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode
    {
        WRAP_REPEAT,
        WRAP_CLAMP
    };
    
    bitmap_info_ovg(std::auto_ptr<image::GnashImage> img,
                    VGImageFormat pixelformat, VGPaint paint);
    ~bitmap_info_ovg();

    void dispose() {
        _img.reset();
    }

    bool disposed() const {
        return !_img.get();
    }

    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;

    virtual image::GnashImage& image() {
        GNASH_REPORT_FUNCTION;
        if (_cache.get()) {
            return *_cache;
        }
        log_error("Image not cached!");
#if 0
        switch (_pixel_format) {
            case GL_RGB:
                _cache.reset(new image::ImageRGB(_orig_width, _orig_height));
                break;
            case GL_RGBA:
                _cache.reset(new image::ImageRGBA(_orig_width, _orig_height));
                break;
            default:
                std::abort();
        }
#endif
        std::fill(_cache->begin(), _cache->end(), 0xff);

        return *_cache;
    }
    
    int _width;
    int _height;

private:
    
    mutable boost::scoped_ptr<image::GnashImage> _img;
    mutable boost::scoped_ptr<image::GnashImage> _cache;
    VGImageFormat   _pixel_format;
    mutable VGImage _image;
    VGPaint         _paint;
    size_t          _orig_width;
    size_t          _orig_height;
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_OVG_BITMAP_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
