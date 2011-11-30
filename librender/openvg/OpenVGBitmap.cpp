// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
//   Free Software Foundation, Inc.
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

#include <boost/scoped_ptr.hpp>
#include <iostream>

#include "Geometry.h"
#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "FillStyle.h"
#include "SWFCxForm.h"
#include "openvg/OpenVGRenderer.h"
#include "openvg/OpenVGBitmap.h"
#include "VG/openvg.h"

namespace gnash {

namespace renderer {

namespace openvg {

static const int NUM_STOPS = 10;

/// @param vgpaint the VG paint context
OpenVGBitmap::OpenVGBitmap(VGPaint paint)
    :
#ifdef BUILD_X11_DEVICE
    _pixel_format(VG_sARGB_8888),
#else
    _pixel_format(VG_sRGB_565),
#endif
    _vgimage(VG_INVALID_HANDLE),
    _vgpaint(paint),
    _aspect_ratio(0.75)       // 4:3 aspect ratio
{
    // GNASH_REPORT_FUNCTION;
}

/// Construct a new bitmap
///
/// @param bitmap A CachedBitmap
/// @param vgpaint the VG paint context
OpenVGBitmap::OpenVGBitmap(CachedBitmap *bitmap, VGPaint vgpaint)
    :
#ifdef BUILD_X11_DEVICE
      _pixel_format(VG_sARGB_8888),
#else
      _pixel_format(VG_sRGB_565),
#endif
      _vgimage(VG_INVALID_HANDLE),
      _vgpaint(vgpaint),
      _aspect_ratio(0.75)       // 4:3 aspect ratio
{
    // GNASH_REPORT_FUNCTION;

    // extract a reference to the image from the cached bitmap
    image::GnashImage &im = bitmap->image();

    // Store the reference so it's available to applyPatternBitmap()
    _image.reset(&im);

    // Create a VG image
    _vgimage = vgCreateImage(_pixel_format, im.width(), im.height(),
                             VG_IMAGE_QUALITY_FASTER);    
    if (_vgimage == VG_INVALID_HANDLE) {
        log_error("Failed to create VG image! %s",
                  Renderer_ovg::getErrorString(vgGetError()));
    }
    
    switch (im.type()) {
    case image::TYPE_RGB:
        log_debug("Image has RGB Pixel Format, Stride is %d, width is %d, height is %d",
                  im.stride(), im.width(), im.height());
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBX_8888,
                   0, 0, im.width(), im.height());
        break;
    case image::TYPE_RGBA:
        log_debug("Image has RGBA Pixel Format, Stride is %d, width is %d, height is %d",
                  im.stride(), im.width(), im.height());
        // Copy the image data into the VG image container
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    default:
        std::abort();
    }
}

/// Construct a new bitmap
///
/// @param A pointer to a GnashImage
/// @param vgpaint the VG paint context

/// @note This is usually only called by createCachedBitmap() when a bitmap
/// is loaded from a swf file. As the renderer isn't initialized yet,
/// only the GnashImage is cached.
OpenVGBitmap::OpenVGBitmap(image::GnashImage *image, VGPaint vgpaint)
    : _image(image),
#ifdef BUILD_X11_DEVICE
      _pixel_format(VG_sARGB_8888),
#else
      _pixel_format(VG_sRGB_565),
#endif
      _vgimage(VG_INVALID_HANDLE),
      _vgpaint(vgpaint),
      _aspect_ratio(0.75)       // 4:3 aspect ratio      
{
    // GNASH_REPORT_FUNCTION;
} 

OpenVGBitmap::~OpenVGBitmap()
{
    // GNASH_REPORT_FUNCTION;
    
    vgDestroyPaint(_vgpaint);
    vgDestroyImage(_vgimage);
}

/// Create a radial gradient and paint it to the context
///
/// @param x0 The X coordinate of the origin point
/// @param y0 The Y coordinate of the origin point
/// @param x1 The X coordinate of the opposite corner point
/// @param y1 The Y coordinate of the opposite corner point
/// @param incolor The base color of the gradient
/// @param paint The VG paint context
/// @return A pointer to the new Bitmap

/// @note
/// OpenVG supports creating linear and gradient fills in hardware, so
/// we want to use that instead of the existing way of calculating the
/// gradient in software.

// A Radial Gradient Bitmap uses two points and a radius in the paint
// coordinate system. The gradient starts at x0,y0 as the center, and
// x1,y1 is the focal point that is forced to be in the circle.
OpenVGBitmap *
OpenVGBitmap::createRadialBitmap(float cx, float cy, float fx, float fy,
                                 float radial, const SWFCxForm& /* cxform */,
                                 const GradientFill::GradientRecords &records,
                                 VGPaint paint)
{
    GNASH_REPORT_FUNCTION;

    VGfloat rgParams[] = { cx, cy, fx, fy, radial };
    std::cerr << "Radial params are: " << cx << ", " << cy << ", "
              << fx << ", " << fy << std::endl;
    
    // Paint Type 
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_RADIAL_GRADIENT);

    // Gradient Parameters
    vgSetParameterfv(paint, VG_PAINT_RADIAL_GRADIENT, 5, rgParams);

    // Color Ramp is the same as for linear gradient
    size_t entries = records.size() * 5;
    boost::scoped_array<VGfloat> ramps(new VGfloat[entries]);
    
    int j = 0;
    for (size_t i=0; i!= records.size(); ++i) {
        std::cerr << "The record is: " << records[i].ratio/255.0f
                  << "(" <<  records[i].ratio << ")";
        rgba c = records[i].color;
        std::cerr << ", " << c.m_r/255.0f << ", " << c.m_g/255.0f << ", "
                  << c.m_b/255.0f << ", " << c.m_a/255.0f << std::endl;
        // The set of 5 for each record is simply: { Offset, R, G, B, A }
        ramps[j++] = records[i].ratio/255.0f;
        ramps[j++] = c.m_r / 255.0f;
        ramps[j++] = c.m_g / 255.0f;
        ramps[j++] = c.m_b / 255.0f;
        ramps[j++] = c.m_a / 255.0f;
    }
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, entries, ramps.get());

    return this;
}

/// Create a linear gradient and paint it to the context
///
/// @param x0 The X coordinate of the origin point
/// @param y0 The Y coordinate of the origin point
/// @param x1 The X coordinate of the opposite corner point
/// @param y1 The Y coordinate of the opposite corner point
/// @param incolor The base color of the gradient
/// @param paint The VG paint context
/// @return A pointer to the new Bitmap
///
/// @note A Linear Gradient Bitmap uses two points, x0,y0 and x1,y1 in the paint
/// coordinate system. The gradient starts at x0,y0 and goes to x1,y1. If
/// x1 and y1 are outside the boundaries of the shape, then the gradient gets
/// clipped at the boundary instead of x1,y1.
OpenVGBitmap *
OpenVGBitmap::createLinearBitmap(float x0, float y0, float x1, float y1,
                                 const SWFCxForm& cxform,
                                 const GradientFill::GradientRecords &records,
                                 const VGPaint paint)
{
    GNASH_REPORT_FUNCTION;
    
    vgSetParameteri(paint, VG_PAINT_TYPE, VG_PAINT_TYPE_LINEAR_GRADIENT);
    vgSetParameteri(paint, VG_PAINT_COLOR_RAMP_SPREAD_MODE,
                           VG_COLOR_RAMP_SPREAD_PAD);

    // This is the origin and size of the gradient
    VGfloat linearGradient[4] = { x0, y0, x1, y1 };
    vgSetParameterfv(paint, VG_PAINT_LINEAR_GRADIENT, 4, linearGradient);
    std::cerr << "Linear params are: " << x0 << ", " << y0 << ", "
              << x1 << ", " << y1 << std::endl;
    
    // The application defines the non-premultiplied sRGBA color and alpha value
    // associated with each of a number of values, called stops.
    // A stop is defined by an offset between 0 and 1, inclusive, and a color value.
    
    // an array of floating-point values giving the offsets and colors
    // of the stops. Each stop is defined by a floating-point offset
    // value and four floating-point values containing the sRGBA color
    // and alpha value associated with each stop, in the form of a
    // non-premultiplied (R, G, B, α) quad. The vgSetParameter
    // function will generate an error if the number of values
    // submitted is not a multiple of 5 (zero is acceptable)
    size_t entries = records.size() * 5;
    boost::scoped_array<VGfloat> ramps(new VGfloat[entries]);    
    int j = 0;
    for (size_t i=0; i!= records.size(); ++i) {
        std::cerr << "The record ratio is: " << records[i].ratio/255.0f;
        rgba c = cxform.transform(records[i].color);
        std::cerr << "; color: " << c.m_r/255.0f << ", " << c.m_g/255.0f << ", "
                  << c.m_b/255.0f << ", " << c.m_a/255.0f << std::endl;
        // The set of 5 for each record is simply: { Offset, R, G, B, A }
        ramps[j++] = records[i].ratio/255.0f;
        ramps[j++] = c.m_r / 255.0f;
        ramps[j++] = c.m_g / 255.0f;
        ramps[j++] = c.m_b / 255.0f;
        ramps[j++] = c.m_a / 255.0f;
    }
#if 1
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, entries, ramps.get());
#else
    VGfloat rampStop[] = {0.00f, 1.0f, 1.0f, 1.0f, 1.0f,
                          0.33f, 1.0f, 0.0f, 0.0f, 1.0f,
                          0.66f, 0.0f, 1.0f, 0.0f, 1.0f,
                          1.00f, 0.0f, 0.0f,  1.0f, 1.0f};
    vgSetParameterfv(paint, VG_PAINT_COLOR_RAMP_STOPS, 20, rampStop);
#endif
    
    return this;
}

/// Create and fill pattern image
///
/// @param matrix The transformation matrix
/// @param mode The mode used to pain the image
/// @param paint The VG paint context
/// @return A pointer to the new Bitmap
OpenVGBitmap *
OpenVGBitmap::applyPatternBitmap(const gnash::SWFMatrix& mat,
                                 bitmap_wrap_mode mode,
                                 CachedBitmap *bitmap, VGPaint /* paint */)
{
    // GNASH_REPORT_FUNCTION;

    // extract a reference to the image from the cached bitmap
    image::GnashImage &im = bitmap->image();
    
    // Create a VG image
    _vgimage = vgCreateImage(_pixel_format, im.width(), im.height(),
                             VG_IMAGE_QUALITY_FASTER);    
    if (_vgimage == VG_INVALID_HANDLE) {
        log_error("Failed to create VG image! %s",
                  Renderer_ovg::getErrorString(vgGetError()));
    }
    
    switch (im.type()) {
    case image::TYPE_RGB:
        log_debug("Image has RGB Pixel Format, Stride is %d, width is %d, height is %d",
                  im.stride(), im.width(), im.height());
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBX_8888,
                   0, 0, im.width(), im.height());
        break;
    case image::TYPE_RGBA:
        log_debug("Image has RGBA Pixel Format, Stride is %d, width is %d, height is %d",
                  im.stride(), im.width(), im.height());
        // Copy the image data into the VG image container
        vgImageSubData(_vgimage, im.begin(), im.stride(), VG_sRGBA_8888,
                   0, 0, im.width(), im.height());
        break;
    default:
        std::abort();
    }
    
    vgSetParameteri (_vgpaint, VG_PAINT_TYPE, VG_PAINT_TYPE_PATTERN);

    VGfloat     vmat[9];
    
    // Adjust the X scale by the aspect ratio or images get squished
    //sm.a() = sm.a() * _aspect_ratio;
    SWFMatrix nsm(mat.a() * 0.75, mat.b(), mat.c(),
                  mat.d(), mat.tx(), mat.ty());
    nsm.invert();
    // Renderer_ovg::printVGMatrix(matrix);
    
    memset(vmat, 0, sizeof(vmat));
    // Convert from fixed point to floating point
    vmat[0] = nsm.a() / 65536.0f;
    vmat[1] = nsm.b() / 65536.0f;
    vmat[3] = nsm.c() / 65536.0f;
    vmat[4] = nsm.d() / 65536.0f;
    vmat[6] = nsm.tx();
    vmat[7] = nsm.ty();
    
    // Renderer_ovg::printVGMatrix(vmat);
    
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_FILL_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_STROKE_PAINT_TO_USER);
    vgLoadMatrix (vmat);
    vgSeti (VG_MATRIX_MODE, VG_MATRIX_PATH_USER_TO_SURFACE);
    
    switch (mode) {
      case WRAP_FILL:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_FILL);
          break;
      case WRAP_PAD:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_PAD);
          break;
      case WRAP_REPEAT:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REPEAT);
          break;
      case WRAP_REFLECT:
          vgSetParameteri (_vgpaint, VG_PAINT_PATTERN_TILING_MODE, VG_TILE_REFLECT);
          break;
      default:
          log_error("No supported wrap mode specified!");
          break;
    }

    vgPaintPattern(_vgpaint, _vgimage);
    vgDestroyImage(_vgimage);
    
    return this;
}

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
