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

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "openvg/OpenVGRenderer.h"
#include "openvg/OpenVGBitmap.h"
#include "VG/openvg.h"

namespace gnash {

namespace renderer {

namespace openvg {

static const int NUM_STOPS = 10;

OpenVGBitmap::OpenVGBitmap(VGPaint paint)
{
    GNASH_REPORT_FUNCTION;
    if (paint) {
        _vgpaint = paint;
    }
}

// 
// VG_sRGB_565
// VG_sRGBA_5551
// VG_sRGBA_4444
// VG_A_8
OpenVGBitmap::OpenVGBitmap(std::auto_ptr<image::GnashImage> image, VGPaint paint)
    :
    _image(image),
    _pixel_format(VG_A_4), // was VG_sARGB_8888  VG_sRGB_565
    _vgpaint(paint)
{
    GNASH_REPORT_FUNCTION;

    size_t width = _image->width();
    size_t height = _image->height();

    // Create a VG image, and copy the GnashImage data into it
    _vgimage = vgCreateImage(_pixel_format, width, height,
                             VG_IMAGE_QUALITY_FASTER);    
    
    vgImageSubData(_vgimage, _image->begin(), width * 4, _pixel_format,
                   0, 0, width, height);
    
    _tex_size += width * height * 4;
    log_debug("Add Texture size:%d (%d x %d x %dbpp)", width * height * 4, 
              width, height, 4);
    log_debug("Current Texture size: %d", _tex_size);
} 

OpenVGBitmap::~OpenVGBitmap()
{
    GNASH_REPORT_FUNCTION;
    
    _tex_size -= _image->width() * _image->height() * 4;
    log_debug(_("Remove Texture size:%d (%d x %d x %dbpp)"),
              _image->width() * _image->height() * 4,
              _image->width(), _image->height(), 4);
    log_debug(_("Current Texture size: %d"), _tex_size);

//    vgDestroyPaint(_vgpaint);
    vgDestroyImage(_vgimage);
}

image::GnashImage&
OpenVGBitmap::image()
{
    GNASH_REPORT_FUNCTION;
    if (_image) {
        return *_image;
    }
}    

// 
void
OpenVGBitmap::apply(const gnash::SWFMatrix& bitmap_matrix,
                    bitmap_wrap_mode wrap_mode, VGPaint paint) const
{
    GNASH_REPORT_FUNCTION;
    
    gnash::SWFMatrix mat;
    VGfloat     vmat[9];
    
    mat = bitmap_matrix;
    
    vgSetParameteri (paint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
    vgPaintPattern (paint, _vgimage);
    
    mat.invert();
    memset(vmat, 0, sizeof(vmat));
    vmat[0] = mat.sx  / 65536.0f;
    vmat[1] = mat.shx / 65536.0f;
    vmat[3] = mat.shy / 65536.0f;
    vmat[4] = mat.sy  / 65536.0f;
    vmat[6] = mat.tx;
    vmat[7] = mat.ty;
    
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    
    if (wrap_mode == WRAP_CLAMP) {  
        vgSetParameteri (paint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_PAD);
    } else {
        vgSetParameteri (paint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
    }
}

/// OpenVG supports creating linear and gradient fills in hardware, so
/// we want to use that instead of the existing way of calculating the
/// gradient in software.
OpenVGBitmap *
OpenVGBitmap::createRadialBitmap(float cx, float cy, float fx, float fy, float radial,
                                 VGPaint paint)
{
    GNASH_REPORT_FUNCTION;

    VGfloat rgParams[] = { cx, cy, fx, fy, radial };
    VGfloat stops[5*NUM_STOPS];
    
    // Paint Type 
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);

    // Gradient Parameters
    vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 4, rgParams);

    // Color Ramp is the same as for linear gradient
    return this;
}

OpenVGBitmap *
OpenVGBitmap::createLinearBitmap(float x0, float y0, float x1, float y1, VGPaint paint)
{
    GNASH_REPORT_FUNCTION;

    VGfloat lgParams[] = { x0, y0, x1, y1 };
    VGfloat stops[] = { 0.0, 0.33, 0.66, 1.0};
    
    // Paint Type
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
    // Gradient Parameters
    vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, lgParams);
    // Color Ramp
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 5*NUM_STOPS, stops);
    vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE, VG_COLOR_RAMP_SPREAD_PAD);

    return this;
}

    // Create and fill pattern image
OpenVGBitmap *
OpenVGBitmap::createPatternBitmap(std::auto_ptr<image::GnashImage> im, VGPaint paint)
{
    GNASH_REPORT_FUNCTION;

    if (paint != VG_INVALID_HANDLE) {
        _vgimage = vgCreateImage(_pixel_format, im->width(), im->height(),
                                 VG_IMAGE_QUALITY_FASTER);
        vgImageSubData(_vgimage, im->begin(), 4*im->width(), /* stride */
                       _pixel_format, 0, 0, im->width(), im->height());
        
        vgSetParameteri(_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);
        vgSetParameteri(_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
        vgPaintPattern(_vgpaint, _vgimage);
        vgDrawImage(_vgimage);
        vgFlush();
    }

    return this;
}

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
