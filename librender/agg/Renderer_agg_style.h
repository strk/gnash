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

#ifndef BACKEND_RENDER_HANDLER_AGG_STYLE_H
#define BACKEND_RENDER_HANDLER_AGG_STYLE_H

// TODO: Instead of re-creating AGG fill styles again and again, they should
// be cached somewhere.

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>
#include <agg_gradient_lut.h>
#include <agg_color_rgba.h>
#include <agg_color_gray.h>
#include <agg_image_accessors.h>
#include <agg_span_allocator.h>
#include <agg_span_gradient.h>
#include <agg_span_interpolator_linear.h>
#include <agg_image_filters.h>
#include <agg_span_image_filter_rgb.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_pixfmt_rgb.h>
#include <agg_pixfmt_rgba.h>
#include <iostream>

#include "LinearRGB.h"
#include "Renderer_agg_bitmap.h"
#include "GnashAlgorithm.h"
#include "FillStyle.h"
#include "SWFCxForm.h"
#include "SWFMatrix.h"

namespace gnash {

class StyleHandler;

// Forward declarations.
namespace {

    /// Creates 8 bitmap functions
    template<typename FillMode, typename Pixel>
            void storeBitmap(StyleHandler& st, const agg_bitmap_info* bi,
            const SWFMatrix& mat, const SWFCxForm& cx,
            bool smooth);
    template<typename FillMode> void storeBitmap(StyleHandler& st,
            const agg_bitmap_info* bi, const SWFMatrix& mat, const SWFCxForm& cx,
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

/// Internal style class that represents a fill style. Roughly speaking, AGG 
/// computes the fill areas of a flash composite shape and calls generate_span
/// to generate small horizontal pixel rows. generate_span provides whatever
/// fill pattern for that coordinate. 
class AggStyle 
{
public:
    AggStyle(bool solid, const agg::rgba8& color = agg::rgba8(0,0,0,0))
      :
      _solid(solid),
      _color(color)
    {
    }
    
    // Everytime a class has a virtual method it should
    // also have a virtual destructor. This will ensure
    // that the destructor for the *derived* class is invoked
    // when deleting a pointer to base class !!
    virtual ~AggStyle() {}
    bool solid() const { return _solid; }
    agg::rgba8 color() const { return _color; }
  
    // for non-solid styles:
    virtual void generate_span(agg::rgba8* span, int x, int y,
            unsigned len) = 0;
  
private:
    // for solid styles:
    const bool _solid;
    const agg::rgba8 _color; 
};

namespace {

/// Tile bitmap fills.
struct Tile
{
    template<typename P> struct Type {
        typedef agg::wrap_mode_repeat Wrap;
        typedef agg::image_accessor_wrap<P, Wrap, Wrap> type; 
    };
};

/// Clip bitmap fills.
struct Clip
{
    template<typename P> struct Type {
        typedef agg::image_accessor_clone<P> type; 
    };
};

/// Base class for filter types.
template<typename P, typename W>
struct FilterType
{
    typedef typename P::PixelFormat PixelFormat;
    typedef typename W::template Type<PixelFormat>::type SourceType;
    typedef agg::span_allocator<PixelFormat> Allocator;
    typedef agg::span_interpolator_linear<agg::trans_affine>
        Interpolator;
};

/// Class with typedefs for RGBA operations.
struct RGBA 
{
    typedef agg::pixfmt_rgba32_pre PixelFormat;

    template<typename SourceType, typename Interpolator>
    struct Simple {
        typedef agg::span_image_filter_rgba_nn<SourceType, Interpolator> type;
    };

    template<typename SourceType, typename Interpolator>
    struct AntiAlias {
        typedef agg::span_image_filter_rgba_bilinear<SourceType, Interpolator>
            type;
    };
};

/// Class with typedefs for RGB operations.
struct RGB 
{
    typedef agg::pixfmt_rgb24_pre PixelFormat;

    template<typename SourceType, typename Interpolator>
    struct Simple {
        typedef agg::span_image_filter_rgb_nn<SourceType, Interpolator> type;
    };

    template<typename SourceType, typename Interpolator>
    struct AntiAlias {
        typedef agg::span_image_filter_rgb_bilinear<SourceType, Interpolator>
            type;
    };
};

/// Nearest Neighbour filter type for quick, lower quality scaling.
template<typename P, typename W>
struct NN : public FilterType<P, W>
{
    typedef FilterType<P, W> BaseType;
    typedef typename P::template Simple<
                typename BaseType::SourceType,
                typename BaseType::Interpolator>::type Generator;
};

/// Bilinear filter type for higher quality scaling.
template<typename P, typename W>
struct AA : public FilterType<P, W>
{
    typedef FilterType<P, W> BaseType;
    typedef typename P::template AntiAlias<
                typename BaseType::SourceType,
                typename BaseType::Interpolator>::type Generator;
};

/// A reflecting adaptor for Gradients.
struct Reflect
{
    template<typename T> struct Type {
        typedef agg::gradient_reflect_adaptor<T> type;
    };
};

/// A repeating adaptor for Gradients.
struct Repeat
{
    template<typename T> struct Type {
        typedef agg::gradient_repeat_adaptor<T> type;
    };
};

/// A padding (default) adaptor for Gradients.
struct Pad
{
    template<typename T> struct Type {
        typedef T type;
    };
};

/// The default RGB color interpolator
struct InterpolatorLinearRGB
{
    template<typename Pixel> struct Type {
        typedef agg::gradient_lut<linear_rgb_interpolator<Pixel>, 256> type;
    };
};

/// The default RGB color interpolator
struct InterpolatorRGB
{
    template<typename Pixel> struct Type {
        typedef agg::gradient_lut<agg::color_interpolator<Pixel>, 256> type;
    };
};

/// AGG gradient fill style. Don't use Gnash texture bitmaps as this is slower
/// and less accurate. Even worse, the bitmap fill would need to be tweaked
/// to have non-repeating gradients (first and last color stops continue 
/// forever on each side). This class can be used for any kind of gradient, so
/// even focal gradients should be possible. 
template <class Color, class Allocator, class Interpolator, class GradientType,
         class Adaptor, class ColorInterpolator, class SpanGenerator>
class GradientStyle : public AggStyle
{
public:
  
    GradientStyle(const GradientFill& fs, const SWFMatrix& mat,
            const SWFCxForm& cx, int norm_size, GradientType gr = GradientType())
        :
        AggStyle(false),
        m_cx(cx),
        m_tr(mat.a() / 65536.0, mat.b() / 65536.0, mat.c() / 65536.0,
              mat.d() / 65536.0, mat.tx(), mat.ty()),
        m_span_interpolator(m_tr),
        m_gradient_adaptor(gr),
        m_sg(m_span_interpolator, m_gradient_adaptor, m_gradient_lut, 0,
                norm_size),
      
        m_need_premultiply(false)
    {
        // Build gradient lookup table
        m_gradient_lut.remove_all(); 
        const size_t size = fs.recordCount();
      
        // It is essential that at least two colours are added; otherwise agg
        // will use uninitialized values.
        assert(size > 1);
    
        for (size_t i = 0; i != size; ++i) { 
            const GradientRecord& gr = fs.record(i); 
            const rgba tr = m_cx.transform(gr.color);
            if (tr.m_a < 0xff) m_need_premultiply = true;    
            m_gradient_lut.add_color(gr.ratio / 255.0,
                    agg::rgba8(tr.m_r, tr.m_g, tr.m_b, tr.m_a));
        } 
        m_gradient_lut.build_lut();
        
    } // GradientStyle constructor
  
    virtual ~GradientStyle() { }
  
    void generate_span(Color* span, int x, int y, unsigned len) {
        m_sg.generate(span, x, y, len);
        if (!m_need_premultiply) return;
        
        while (len--) {
            span->premultiply();
            ++span;
        }
    }
    
protected:
    
    // Color transform
    SWFCxForm m_cx;
    
    // Span allocator
    Allocator m_sa;
    
    // Transformer
    agg::trans_affine m_tr;
    
    // Span interpolator
    Interpolator m_span_interpolator;
    
    // Gradient adaptor
    Adaptor m_gradient_adaptor;  
    
    // Gradient LUT
    ColorInterpolator m_gradient_lut;
    
    // Span generator
    SpanGenerator m_sg;  
  
    // premultiplication necessary?
    bool m_need_premultiply;
}; 

/// A set of typedefs for a Gradient
//
/// @tparam G       An agg gradient type
/// @tparam A       The type of Adaptor: see Reflect, Repeat, Pad
/// @tparam I       The type of ColorInterpolator: see InterpolatorRGB
template<typename G, typename A, typename I>
struct Gradient
{
    typedef agg::rgba8 Color;            
    typedef G GradientType;
    typedef typename A::template Type<G>::type Adaptor;
    typedef typename I::template Type<Color>::type ColorInterpolator;
    typedef agg::span_allocator<Color> Allocator;
    typedef agg::span_interpolator_linear<agg::trans_affine> Interpolator;
    typedef agg::span_gradient<Color, Interpolator, Adaptor,
            ColorInterpolator> Generator;
    typedef GradientStyle<Color, Allocator, Interpolator, GradientType,
                             Adaptor, ColorInterpolator, Generator> Type;
};


/// Solid AGG fill style. generate_span is not used in this case as AGG does
/// solid fill styles internally.
class SolidStyle : public AggStyle 
{
public:

  SolidStyle(const agg::rgba8& color)
    :
    AggStyle(true, color)
  {
  }

  void generate_span(agg::rgba8* /*span*/, int /*x*/, int /*y*/,
        unsigned /*len*/)
  {
    abort(); // never call generate_span for solid fill styles
  }
};


/// AGG bitmap fill style. There are quite a few combinations possible and so
/// the class types are defined outside. The bitmap can be tiled or clipped.
/// It can have any transformation SWFMatrix and color transform. Any pixel format
/// can be used, too. 
template <class PixelFormat, class Allocator, class SourceType,
       class Interpolator, class Generator>
class BitmapStyle : public AggStyle
{
public:
    
  BitmapStyle(int width, int height, int rowlen, boost::uint8_t* data, 
    const SWFMatrix& mat, const SWFCxForm& cx)
    :
    AggStyle(false),
    m_cx(cx),
    m_rbuf(data, width, height, rowlen),  
    m_pixf(m_rbuf),
    m_img_src(m_pixf),
    m_tr(mat.a() / 65535.0, mat.b() / 65535.0, mat.c() / 65535.0,
            mat.d() / 65535.0, mat.tx(), mat.ty()),
    m_interpolator(m_tr),
    m_sg(m_img_src, m_interpolator)
  {
  }
  
  virtual ~BitmapStyle() {
  }
    
    void generate_span(agg::rgba8* span, int x, int y, unsigned len)
    {
        m_sg.generate(span, x, y, len);

        const bool transform = (m_cx != SWFCxForm());

        for (size_t i = 0; i < len; ++i) {
            // We must always do this because dynamic bitmaps (BitmapData)
            // can have any values. Loaded bitmaps are handled when loaded.
            span->r = std::min(span->r, span->a);
            span->g = std::min(span->g, span->a);
            span->b = std::min(span->b, span->a);
            if (transform) {
                m_cx.transform(span->r, span->g, span->b, span->a);
                span->premultiply();
            }
            ++span;
        }  
    }
  
private:

    // Color transform
    SWFCxForm m_cx;

    // Pixel access
    agg::rendering_buffer m_rbuf;
    PixelFormat m_pixf;
  
    // Span allocator
    Allocator m_sa;
  
    // Image accessor
    SourceType m_img_src;
  
    // Transformer
    agg::trans_affine m_tr;
  
    // Interpolator
    Interpolator m_interpolator;
  
    // Span generator
    Generator m_sg;  
};

}


// --- AGG HELPER CLASSES ------------------------------------------------------

/// Style handler for AGG's compound rasterizer. This is the class which is
/// called by AGG itself. It provides an interface to the various fill style
/// classes defined above.
class StyleHandler
{
public:

    StyleHandler() : 
        m_transparent(0, 0, 0, 0)        
    {}
    
    ~StyleHandler() {
    }

    /// Called by AGG to ask if a certain style is a solid color
    bool is_solid(unsigned style) const {
      assert(style < _styles.size());
      return _styles[style].solid(); 
    }
    
    /// Adds a new solid fill color style
    void add_color(const agg::rgba8& color) {
      SolidStyle *st = new SolidStyle(color);
      _styles.push_back(st);
    }

    /// Adds a new bitmap fill style
    void add_bitmap(const agg_bitmap_info* bi, const SWFMatrix& mat,
        const SWFCxForm& cx, bool repeat, bool smooth) {

        if (!bi) {
            add_color(agg::rgba8_pre(0,0,0,0));
            return;
        }

        // Tiled
        if (repeat) {
            storeBitmap<Tile>(*this, bi, mat, cx, smooth);
            return;
        }

        storeBitmap<Clip>(*this, bi, mat, cx, smooth);
    } 

    template<typename T>
    void addLinearGradient(const GradientFill& fs, const SWFMatrix& mat,
            const SWFCxForm& cx)
    {
        // NOTE: The value 256 is based on the bitmap texture used by other
        // Gnash renderers which is normally 256x1 pixels for linear gradients.
        typename T::Type* st = new typename T::Type(fs, mat, cx, 256);
        _styles.push_back(st);
    }
    
    template<typename T>
    void addFocalGradient(const GradientFill& fs, const SWFMatrix& mat,
            const SWFCxForm& cx)
    {
        typename T::GradientType gr;
        gr.init(32.0, fs.focalPoint() * 32.0, 0.0);
        
        // div 2 because we need radius, not diameter      
        typename T::Type* st = new typename T::Type(fs, mat, cx, 32.0, gr); 
        
        // NOTE: The value 64 is based on the bitmap texture used by other
        // Gnash renderers which is normally 64x64 pixels for radial gradients.
        _styles.push_back(st);
    }
    
    template<typename T>
    void addRadialGradient(const GradientFill& fs, const SWFMatrix& mat,
            const SWFCxForm& cx)
    {

        // div 2 because we need radius, not diameter      
        typename T::Type* st = new typename T::Type(fs, mat, cx, 64 / 2); 
          
        // NOTE: The value 64 is based on the bitmap texture used by other
        // Gnash renderers which is normally 64x64 pixels for radial gradients.
        _styles.push_back(st);
    }

    /// Returns the color of a certain fill style (solid)
    agg::rgba8 color(unsigned style) const 
    {
        if (style < _styles.size())
            return _styles[style].color();

        return m_transparent;
    }

    /// Called by AGG to generate a scanline span for non-solid fills 
    void generate_span(agg::rgba8* span, int x, int y,
        unsigned len, unsigned style)
    {
      _styles[style].generate_span(span,x,y,len);
    }


    /// Add a bitmap with the specified filter
    //
    /// @tparam Filter      The FilterType to use. This affects scaling
    ///                     quality, pixel type etc.
    template<typename Filter> void
    addBitmap(const agg_bitmap_info* bi, const SWFMatrix& mat,
            const SWFCxForm& cx)
    {
        typedef typename Filter::PixelFormat PixelFormat;
        typedef typename Filter::Generator Generator;
        typedef typename Filter::Allocator Allocator;
        typedef typename Filter::SourceType SourceType;
        typedef typename Filter::Interpolator Interpolator;

        typedef BitmapStyle<PixelFormat, Allocator,
                SourceType, Interpolator, Generator> Style;
      
        Style* st = new Style(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
        _styles.push_back(st);
    }

    boost::ptr_vector<AggStyle> _styles;
    agg::rgba8 m_transparent;

}; 

class agg_mask_style_handler 
{
public:

  agg_mask_style_handler() :
    m_color(255,255)
  {
  }

  bool is_solid(unsigned /*style*/) const
  {
    return true;
  }
  
  const agg::gray8& color(unsigned /*style*/) const 
  {
    return m_color;
  }
  
  void generate_span(agg::gray8* /*span*/, int /*x*/, int /*y*/,
        int /*len*/, unsigned /*style*/)
  {
    abort(); // never call generate_span for solid fill styles
  }

private:
  agg::gray8 m_color;
  
};  // class agg_mask_style_handler

/// Style handler
//
/// Transfer FillStyles to agg styles.
struct AddStyles : boost::static_visitor<>
{
    AddStyles(SWFMatrix stage, SWFMatrix fill, const SWFCxForm& c,
            StyleHandler& sh, Quality q)
        :
        _stageMatrix(stage.invert()),
        _fillMatrix(fill.invert()),
        _cx(c),
        _sh(sh),
        _quality(q)
    {
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
        _sh.add_color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b,
                  color.m_a));
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
                    if (_quality >= QUALITY_BEST) smooth = true;
                    break;
                case BitmapFill::SMOOTHING_ON:
                    smooth = true;
                    break;
                default: break;
            }
        }

        const bool tiled = (f.type() == BitmapFill::TILED);

        _sh.add_bitmap(dynamic_cast<const agg_bitmap_info*>(f.bitmap()),
                m, _cx, tiled, smooth);
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

namespace {

template<typename FillMode, typename Pixel>
void
storeBitmap(StyleHandler& st, const agg_bitmap_info* bi,
        const SWFMatrix& mat, const SWFCxForm& cx, bool smooth)
{
    if (smooth) {
        st.addBitmap<AA<Pixel, FillMode> >(bi, mat, cx);
        return;
    }
    st.addBitmap<NN<Pixel, FillMode> >(bi, mat, cx);
}

template<typename FillMode>
void
storeBitmap(StyleHandler& st, const agg_bitmap_info* bi,
        const SWFMatrix& mat, const SWFCxForm& cx, bool smooth)
{

    if (bi->get_bpp() == 24) {
        storeBitmap<FillMode, RGB>(st, bi, mat, cx, smooth);
        return;
    }
    storeBitmap<FillMode, RGBA>(st, bi, mat, cx, smooth);
}

template<typename Spread, typename Interpolation>
void
storeGradient(StyleHandler& st, const GradientFill& fs, const SWFMatrix& mat,
        const SWFCxForm& cx)
{
      
    typedef agg::gradient_x Linear;
    typedef agg::gradient_radial Radial;
    typedef agg::gradient_radial_focus Focal;

    typedef Gradient<Linear, Spread, Interpolation> LinearGradient;
    typedef Gradient<Focal, Spread, Interpolation> FocalGradient;
    typedef Gradient<Radial, Spread, Interpolation> RadialGradient;

    switch (fs.type()) {
        case GradientFill::LINEAR:
            st.addLinearGradient<LinearGradient>(fs, mat, cx);
            return;
      
        case GradientFill::RADIAL:
            if (fs.focalPoint()) {
                st.addFocalGradient<FocalGradient>(fs, mat, cx);
                return;
            }
            st.addRadialGradient<RadialGradient>(fs, mat, cx);
    }
}

template<typename Spread>
void
storeGradient(StyleHandler& st, const GradientFill& fs, const SWFMatrix& mat,
        const SWFCxForm& cx)
{
    switch (fs.interpolation) {
        case SWF::GRADIENT_INTERPOLATION_NORMAL:
            storeGradient<Spread, InterpolatorRGB>(st, fs, mat, cx);
            break;
        case SWF::GRADIENT_INTERPOLATION_LINEAR:
            storeGradient<Spread, InterpolatorLinearRGB>(st, fs, mat, cx);
            break;
    }

}

void
storeGradient(StyleHandler& st, const GradientFill& fs, const SWFMatrix& mat,
        const SWFCxForm& cx)
{   

      switch (fs.spreadMode) {
          case GradientFill::PAD:
              storeGradient<Pad>(st, fs, mat, cx);
              break;
          case GradientFill::REFLECT:
              storeGradient<Reflect>(st, fs, mat, cx);
              break;
          case GradientFill::REPEAT:
              storeGradient<Repeat>(st, fs, mat, cx);
              break;
      }
}

}

} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_STYLE_H
