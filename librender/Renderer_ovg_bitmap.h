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
//#include "BitmapInfo.h"
#include "GnashImage.h"
#include "Renderer.h"

namespace gnash {


/// The class itself uses a template. Currently this is unnecessary and it may
/// be removed but an older implementation required this method and it may be
/// necessary again when the last missing parts of the renderer will be
/// implemented. And when might that be? I don't think I'll wait.
class bitmap_info_ovg // : public BitmapInfo
{
public:
  
    /// Set line and fill styles for mesh & line_strip rendering.
    enum bitmap_wrap_mode
    {
        WRAP_REPEAT,
        WRAP_CLAMP
    };
    
    bitmap_info_ovg(image::GnashImage* img, VGImageFormat pixelformat, VGPaint paint);
    ~bitmap_info_ovg();

    void apply(const gnash::SWFMatrix& bitmap_matrix,
               bitmap_wrap_mode wrap_mode) const;

    int _width;
    int _height;

private:
    
    mutable image::GnashImage *_img;
    VGImageFormat   _pixel_format;
    mutable VGImage _image;
    VGPaint         _paint;
};

} // namespace gnash

#endif // __RENDER_HANDLER_OVG_BITMAP_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
