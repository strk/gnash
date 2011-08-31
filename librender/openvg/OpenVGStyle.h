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

#ifndef GNASH_OPENVG_STYLE_H
#define GNASH_OPENVG_STYLE_H

#include "CachedBitmap.h"
#include "GnashImage.h"
#include "Renderer.h"
#include "FillStyle.h"
#include "SWFCxForm.h"
#include "SWFMatrix.h"
#include "OpenVGBitmap.h"

namespace gnash {

// Forward declarations.
class SolidFill;
class GradientFill;
class BitmapFill;
class rgba; 
class StyleHandler;

namespace renderer {

namespace openvg {

#if 0
// Forward declarations.
namespace {
    /// Creates 8 bitmap functions
    template<typename FillMode, typename Pixel>
            void storeBitmap(StyleHandler& st, const OpenVGBitmap* bi,
            const SWFMatrix& mat, const SWFCxForm& cx,
            bool smooth);
    template<typename FillMode> void storeBitmap(StyleHandler& st,
            const OpenVGBitmap* bi, const SWFMatrix& mat, const SWFCxForm& cx,
            bool smooth);

    /// Creates many (should be 18) gradient functions.
    void storeGradient(StyleHandler& st, const GradientFill& fs,
            const SWFMatrix& mat, const SWFCxForm& cx);
    template<typename Spread> void storeGradient(StyleHandler& st,
            const GradientFill& fs, const SWFMatrix& mat, const SWFCxForm& cx);
    template<typename Spread, typename Interpolation>
            void storeGradient(StyleHandler& st, const GradientFill& fs,
            const SWFMatrix& mat, const SWFCxForm& cx);
}
#endif

/// @note These helper functions are used by the boost::variant used
/// for fill styles. A variant is a C++ style version of the C union.
/// Before accessing any of the data of the variant, we have to use
/// boost::apply_visitor() to bind one of these classes to the style
/// to extract the data.

/// Transfer FillStyles to OpenVG styles.
struct StyleHandler : boost::static_visitor<>
{
    StyleHandler(const SWFMatrix& mat, const SWFCxForm& cx,
                 const VGPaint &p, float x, float y)
        : _matrix(mat),
          _cxform(cx),
          _vgpaint(p),
          _x(x),
          _y(y)
        {
            GNASH_REPORT_FUNCTION;
        }

                   
    void operator()(const GradientFill& g) const {
        GNASH_REPORT_FUNCTION;
        SWFMatrix mat = g.matrix();
        Renderer_ovg::printVGMatrix(mat);
        //      from OpenVG specification PDF
        //
        //          dx(x - x0) + dy((y - y0)
        // g(x,y) = ------------------------
        //                dx^2 + dy^2
        // where dx = x1 - x0, dy = y1 - y0
        //
        int width = 800;
        int height = 480;
#if 0
        float inv_width = 1.0f / width;
        float inv_height = 1.0f / height;
        float p[4] = { 0, 0, 0, 0 };
        p[0] = mat.a() / 65536.0f * inv_width;
        p[1] = mat.c() / 65536.0f * inv_width;
        p[3] = mat.tx() * inv_width;
#endif
        std::cerr << "X=" << _x << ", Y=" << _y << std::endl;

        const GradientFill::Type fill_type = g.type();
        OpenVGBitmap* binfo = new OpenVGBitmap(_vgpaint);
        if (fill_type ==  GradientFill::LINEAR) {
            // All positions are specified in twips, which are 20 to the
            // pixel. Use the display size for the extent of the shape, 
            // as it'll get clipped by OpenVG at the end of the
            // shape that is being filled with the gradient.
            const std::vector<gnash::GradientRecord> &records = g.getRecords();
            log_debug("Fill Style Type: Linear Gradient, %d records", records.size());
            binfo->createLinearBitmap(_x, _y, width, height, _cxform, records,  _vgpaint);
        }
        if (fill_type == GradientFill::RADIAL) {
            float focalpt = g.focalPoint();
            const std::vector<gnash::GradientRecord> &records = g.getRecords();
            log_debug("Fill Style Type: Radial Gradient: focal is: %d, %d:%d",
                      focalpt, 200.0f, 200.0f);
            // All positions are specified in twips, which are 20 to the
            // pixel. Use the display size for the extent of the shape, 
            // as it'll get clipped by OpenVG at the end of the
            // shape that is being filled with the gradient.
            binfo->createRadialBitmap(200.0f, 200.0f, 200.0f, 200.0f, 100,
                                      _cxform, records, _vgpaint);
        }
    }

    void operator()(const SolidFill& f) const {
        const rgba color = _cxform.transform(f.color());

        // add the color to our self-made style handler (basically
        // just a list)
        // _sh.add_color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b,
        //           color.m_a));
    }

    void operator()(const BitmapFill& f) const {
        // OpenVGBitmap *binfo = new OpenVGBitmap(cb, _fillpaint);          
        SWFMatrix m = f.matrix();
        
        const bool tiled = (f.type() == BitmapFill::TILED);

        const CachedBitmap* bm = f.bitmap(); 

        if (!bm) {
            // See misc-swfmill.all/missing_bitmap.swf
            // _sh.add_color(agg::rgba8_pre(255,0,0,255));
        } else if ( bm->disposed() ) {
            // See misc-ming.all/BeginBitmapFill.swf
            // _sh.add_color(agg::rgba8_pre(0,0,0,0));
        // } else {
            // _sh.add_bitmap(dynamic_cast<const agg_bitmap_info*>(bm),
            //                m, _cx, tiled, smooth);
        }
    }
    
private:
    const SWFMatrix& _matrix;
    const SWFCxForm& _cxform;
    const VGPaint&   _vgpaint;
    float            _x;
    float            _y;
};
    
/// Get the color of a style from the variant
class GetColor : public boost::static_visitor<rgba>
{
public:
    rgba operator()(const SolidFill& f) const {
        return f.color();
    }
    rgba operator()(const GradientFill&) const {
        return rgba();
    }
    rgba operator()(const BitmapFill&) const {
        return rgba();
    }
};

/// Get the fill type. Each fill type has it's own sub types,
/// so we map the sub type name to the fill type name.
class GetType : public boost::static_visitor<SWF::FillType>
{
public:
    SWF::FillType operator()(const SolidFill&) const {
        return SWF::FILL_SOLID;
    }
    SWF::FillType operator()(const GradientFill& g) const {
        switch (g.type()) {
          case GradientFill::LINEAR:
              return SWF::FILL_LINEAR_GRADIENT;
              break;
          case GradientFill::RADIAL:
              return SWF::FILL_RADIAL_GRADIENT;
              break;
          default:
              break;              
        }
    }
    SWF::FillType operator()(const BitmapFill& b) const {
        switch (b.type()) {
          case BitmapFill::TILED:
              if (b.smoothingPolicy() == BitmapFill::SMOOTHING_OFF) {
                  return SWF::FILL_TILED_BITMAP_HARD;
              } else {
                  return SWF::FILL_TILED_BITMAP;
              }
              break;
          case BitmapFill::CLIPPED:
              if (b.smoothingPolicy() == BitmapFill::SMOOTHING_OFF) {
                  return SWF::FILL_CLIPPED_BITMAP_HARD;
              } else {
                  return SWF::FILL_CLIPPED_BITMAP;
              }
              break;
          default:
              break;
        }
    }
};

/// Get the bitmap data of a style from the variant
class GetBitmap : public boost::static_visitor<CachedBitmap *>
{
public:
    CachedBitmap *operator()(const SolidFill&) const {
        return 0;
    }
    CachedBitmap *operator()(const GradientFill&) const {
        return 0;
    }
    CachedBitmap *operator()(const BitmapFill& b) const {
        return const_cast<CachedBitmap *>(b.bitmap());
    }
};

/// Get the image from style variant
class GetImage : public boost::static_visitor<image::GnashImage *>
{
public:
    image::GnashImage *operator()(const SolidFill&) const {
        return 0;
    }
    image::GnashImage *operator()(const GradientFill&) const {
        return 0;
    }
    image::GnashImage *operator()(const BitmapFill& b) const {
        CachedBitmap *cb = const_cast<CachedBitmap *>(b.bitmap());
        image::GnashImage &im = cb->image();
//      image::GnashImage::const_iterator it = const_cast<CachedBitmap *>(cb)->image().begin();
        return &im;
    }
};

/// Get the matrix of a style from the variant
class GetMatrix : public boost::static_visitor<SWFMatrix>
{
public:
    SWFMatrix operator()(const SolidFill&) const {
    }
    SWFMatrix operator()(const GradientFill& g) const {
        return g.matrix();
    }
    SWFMatrix operator()(const BitmapFill& b) const {
        return b.matrix();
    }
};

/// GradientFills have more data we need to construct the gradient.

/// Return the focal point of a radial gradient
class GetFocalPoint : public boost::static_visitor<double>
{
public:
    double operator()(const SolidFill&) const {
        return 0.0f;
    }
    double operator()(const GradientFill& g) const {
        return g.focalPoint();
    }
    double operator()(const BitmapFill&) const {
        return 0.0f;
    }
};

/// Return the records in the gradient
class GetGradientRecords : public boost::static_visitor<const GradientFill::GradientRecords &>
{
public:
    const GradientFill::GradientRecords &operator()(const SolidFill&) const {
    }
    const GradientFill::GradientRecords &operator()(const GradientFill& fs) const {
        return fs.getRecords();
    }
    const GradientFill::GradientRecords &operator()(const BitmapFill&) const {
    }
};

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_OPENVG_STYLE_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
