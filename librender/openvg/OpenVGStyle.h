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

namespace gnash {

class SolidFill;
class GradientFill;
class BitmapFill;
class rgba; 
class StyleHandler;
 
namespace renderer {

namespace openvg {

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

/// @note These helper functions are used by the boost::variant used
/// for fill styles. A variant is a C++ style version of the C union.
/// Before accessing any of the data of the variant, we have to use
/// boost::apply_visitor() to bind one of these classes to the style
/// to extract the data.

/// Transfer FillStyles to OpenVG styles.
struct OpenVGStyles : boost::static_visitor<>
{
    OpenVGStyles(SWFMatrix stage, SWFMatrix fill, const SWFCxForm& c,
                 StyleHandler& sh, Quality q)
        : _stageMatrix(stage.invert()),
          _fillMatrix(fill.invert()),
          _cx(c),
          _sh(sh),
          _quality(q)
        {
            GNASH_REPORT_FUNCTION;
        }
    
    void operator()(const GradientFill& f) const {
        SWFMatrix m = f.matrix();
        m.concatenate(_fillMatrix);
        m.concatenate(_stageMatrix);
        storeGradient(_sh, f, m, _cx);
    }

    void operator()(const SolidFill& f) const {
        const rgba color = _cx.transform(f.color());

        // add the color to our self-made style handler (basically
        // just a list)
        // _sh.add_color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b,
        //           color.m_a));
    }

    void operator()(const BitmapFill& f) const {
        SWFMatrix m = f.matrix();
        m.concatenate(_fillMatrix);
        m.concatenate(_stageMatrix);

        // Smoothing policy:
        //
        // - If unspecified, smooth when _quality >= BEST
        // - If ON or forced, smooth when _quality > LOW
        // - If OFF, don't smooth
        //
        // TODO: take a forceBitmapSmoothing parameter.
        //       which should be computed by the VM looking
        //       at MovieClip.forceSmoothing.
        bool smooth = false;
        if (_quality > QUALITY_LOW) {
            // TODO: if forceSmoothing is true, smooth !
            switch (f.smoothingPolicy()) {
                case BitmapFill::SMOOTHING_UNSPECIFIED:
                    if (_quality >= QUALITY_BEST) {
                        smooth = true;
                    }
                    break;
                case BitmapFill::SMOOTHING_ON:
                    smooth = true;
                    break;
                default: break;
            }
        }

        const bool tiled = (f.type() == BitmapFill::TILED);

        const CachedBitmap* bm = f.bitmap(); 

#if 0
        if (!bm) {
            // See misc-swfmill.all/missing_bitmap.swf
            _sh.add_color(agg::rgba8_pre(255,0,0,255));
        } else if ( bm->disposed() ) {
            // See misc-ming.all/BeginBitmapFill.swf
            _sh.add_color(agg::rgba8_pre(0,0,0,0));
        } else {
            _sh.add_bitmap(dynamic_cast<const agg_bitmap_info*>(bm),
                           m, _cx, tiled, smooth);
        }
#endif
    }
    
private:
    /// The inverted stage matrix.
    const SWFMatrix _stageMatrix;
    
    /// The inverted fill matrix.
    const SWFMatrix _fillMatrix;
    const SWFCxForm& _cx;
    StyleHandler& _sh;
    const Quality _quality;
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
