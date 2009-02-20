// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

// This include file used only to make render_handler_agg more readable.


// TODO: Instead of re-creating AGG fill styles again and again, they should
// be cached somewhere.


// Enable this DEFINE to limit the alpha value of all colors to 50% at most.
// This works only with solid and gradient fills (not bitmaps) and is used
// for debugging hidden characters.
//#define DEBUG_LIMIT_COLOR_ALPHA 


namespace gnash {

/// Internal style class that represents a fill style. Roughly speaking, AGG 
/// computes the fill areas of a flash composite shape and calls generate_span
/// to generate small horizontal pixel rows. generate_span provides whatever
/// fill pattern for that coordinate. 
class agg_style_base 
{
public:

  agg_style_base(bool solid, const agg::rgba8& color = agg::rgba8(0,0,0,0))
    :
    _solid(solid),
    _color(color)
  {
  }
  
  // Everytime a class has a virtual method it should
  // also have a virtual destructor. This will ensure
  // that the destructor for the *derived* class is invoked
  // when deleting a pointer to base class !!
  virtual ~agg_style_base() {}

  bool solid() const { return _solid; }

  agg::rgba8 color() const { return _color; }

  // for non-solid styles:
  virtual void generate_span(agg::rgba8* span, int x, int y, unsigned len) = 0;

private:

  // for solid styles:
  bool _solid;

  agg::rgba8 _color; 
  
};


/// Solid AGG fill style. generate_span is not used in this case as AGG does
/// solid fill styles internally.
class agg_style_solid : public agg_style_base 
{
public:

  agg_style_solid(const agg::rgba8& color)
    :
    agg_style_base(true, color)
  {
#ifdef DEBUG_LIMIT_COLOR_ALPHA
    m_color.a = m_color.a>127 ? 127 : m_color.a;
#endif    
  }

  void generate_span(agg::rgba8* /*span*/, int /*x*/, int /*y*/,
        unsigned /*len*/)
  {
    abort(); // never call generate_span for solid fill styles
  }
};


#define image_accessor_clip_transp agg::image_accessor_clone


/// AGG bitmap fill style. There are quite a few combinations possible and so
/// the class types are defined outside. The bitmap can be tiled or clipped.
/// It can have any transformation SWFMatrix and color transform. Any pixel format
/// can be used, too. 
template <class PixelFormat, class span_allocator_type, class img_source_type,
       class interpolator_type, class sg_type>
class agg_style_bitmap : public agg_style_base
{
public:
    
  agg_style_bitmap(int width, int height, int rowlen, boost::uint8_t* data, 
    const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    :
    agg_style_base(false),
    m_cx(cx),
    m_rbuf(data, width, height, rowlen),  
    m_pixf(m_rbuf),
    m_img_src(m_pixf),
    m_tr(mat.sx / 65535.0, mat.shx / 65535.0, mat.shy / 65535.0,
            mat.sy / 65535.0, mat.tx, mat.ty),
    m_interpolator(m_tr),
    m_sg(m_img_src, m_interpolator)
  {
    
    // Convert the transformation SWFMatrix to AGG's class. It's basically the
    // same and we could even use gnash::SWFMatrix since AGG does not require
    // a real AGG descendant (templates!). However, it's better to use AGG's
    // class as this should be faster (avoid type conversion).
  }
  
  virtual ~agg_style_bitmap() {
  }
    
    void generate_span(agg::rgba8* span, int x, int y, unsigned len)
    {
        m_sg.generate(span, x, y, len);
        // Apply color transform
        // TODO: Check if this can be optimized
        if (m_cx.is_identity()) return;
        for (unsigned int i=0; i<len; i++) {
            m_cx.transform(span->r, span->g, span->b, span->a);
            span->premultiply();
            ++span;
        }  
    }
  
private:

  // Color transform
  gnash::cxform m_cx;

  // Pixel access
  agg::rendering_buffer m_rbuf;
  PixelFormat m_pixf;
  
  // Span allocator
  span_allocator_type m_sa;
  
  // Image accessor
  img_source_type m_img_src;
  
  // Transformer
  agg::trans_affine m_tr;
  
  // Interpolator
  interpolator_type m_interpolator;
  
  // Span generator
  sg_type m_sg;  
};


/// AGG gradient fill style. Don't use Gnash texture bitmaps as this is slower
/// and less accurate. Even worse, the bitmap fill would need to be tweaked
/// to have non-repeating gradients (first and last color stops continue 
/// forever on each side). This class can be used for any kind of gradient, so
/// even focal gradients should be possible. 
template <class color_type, class span_allocator_type, class interpolator_type, 
  class gradient_func_type, class gradient_adaptor_type, class color_func_type, 
  class sg_type>
class agg_style_gradient : public agg_style_base
{
public:

  agg_style_gradient(const gnash::fill_style& fs,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx,
        int norm_size)
    :
    agg_style_base(false),
    m_cx(cx),
    m_tr(mat.sx / 65536.0, mat.shx/65536.0, mat.shy / 65536.0,
            mat.sy / 65536.0, mat.tx, mat.ty),
    m_span_interpolator(m_tr),
    m_gradient_func(),
    m_gradient_adaptor(m_gradient_func),
    m_sg(m_span_interpolator, m_gradient_adaptor, m_gradient_lut, 0, norm_size),
    m_need_premultiply(false)
  {
    // Build gradient lookup table
    m_gradient_lut.remove_all(); 
    
    for (int i = 0, e = fs.get_color_stop_count(); i != e; ++i) {
    
      const gradient_record& gr = fs.get_color_stop(i); 
      rgba trans_color = m_cx.transform(gr.m_color);
      
#ifdef DEBUG_LIMIT_COLOR_ALPHA
      trans_color.m_a = trans_color.m_a>127 ? 127 : trans_color.m_a;
#endif

      if (trans_color.m_a < 255) m_need_premultiply = true;    
      
      m_gradient_lut.add_color(gr.m_ratio/255.0, agg::rgba8(trans_color.m_r, 
        trans_color.m_g, trans_color.m_b, trans_color.m_a));
        
    } // for
    
    m_gradient_lut.build_lut();
    
  } // agg_style_gradient constructor


  virtual ~agg_style_gradient() 
  {
  }


  void generate_span(color_type* span, int x, int y, unsigned len) 
  {
    m_sg.generate(span, x, y, len);
    
    if (!m_need_premultiply) return;
      
    while (len--) {
        span->premultiply();
        ++span;
    }
  }
  
  // Provide access to our gradient adaptor to allow re-initialization of
  // focal gradients. I wanted to do this using partial template specialization
  // but it became too complex for something that can be solved in a very easy
  // (slightly unelegant) way. 
  gradient_adaptor_type& get_gradient_adaptor() {
    return m_gradient_adaptor;
  }
  
protected:
  
  // Color transform
  gnash::cxform m_cx;
  
  // Span allocator
  span_allocator_type m_sa;
  
  // Transformer
  agg::trans_affine m_tr;
  
  // Span interpolator
  interpolator_type m_span_interpolator;
  
  gradient_func_type m_gradient_func;
  
  // Gradient adaptor
  gradient_adaptor_type m_gradient_adaptor;  
  
  // Gradient LUT
  color_func_type m_gradient_lut;
  
  // Span generator
  sg_type m_sg;  

  // premultiplication necessary?
  bool m_need_premultiply;
  
}; // agg_style_gradient




// --- AGG HELPER CLASSES ------------------------------------------------------

/// Style handler for AGG's compound rasterizer. This is the class which is
/// called by AGG itself. It provides an interface to the various fill style
/// classes defined above.
class agg_style_handler
{
public:

    agg_style_handler() : 
        m_transparent(0, 0, 0, 0)        
    {}
    
    ~agg_style_handler() {
      int styles_size = m_styles.size(); 
      for (int i=0; i<styles_size; i++)
        delete m_styles[i]; 
    }

    /// Called by AGG to ask if a certain style is a solid color
    bool is_solid(unsigned style) const {
      assert(style < m_styles.size());
      return m_styles[style]->solid(); 
    }
    
    /// Adds a new solid fill color style
    void add_color(const agg::rgba8& color) {
      agg_style_solid *st = new agg_style_solid(color);
      m_styles.push_back(st);
    }
    
    /// Adds a new bitmap fill style
    void add_bitmap(agg_bitmap_info* bi, const gnash::SWFMatrix& mat,
        const gnash::cxform& cx, bool repeat, bool smooth)
    {

      if (!bi) {
        // See server/styles.h comments about when NULL return is possible.
        // Don't warn here, we already warn at parse-time
        //log_debug("WARNING: add_bitmap called with bi=NULL");
        add_color(agg::rgba8_pre(0,0,0,0));
        return;
      }

      // Whew! There are 8 bitmap combinations (bpp, smooth, repeat) possible
      // and AGG uses templates, so... 
      // I'd need to pass "span_image_filter_rgba_nn" (because span_image_xxx
      // dependends on the pixel format) without passing the template 
      // parameters, but AFAIK this can't be done. But hey, this is my first 
      // C++ code (the whole AGG backend) and I immediately had to start with 
      // templates. I'm giving up and write eight versions of add_bitmap_xxx. 
      // So, if anyone has a better solution, for heaven's sake, implement it!! 
        
      if (repeat) {
        if (smooth) {
        
          if (bi->get_bpp()==24)
            add_bitmap_repeat_aa_rgb24 (bi, mat, cx);      
          else
          if (bi->get_bpp()==32)
            add_bitmap_repeat_aa_rgba32 (bi, mat, cx);      
          else
            abort();
            
        } else {
        
          if (bi->get_bpp()==24)
            add_bitmap_repeat_nn_rgb24 (bi, mat, cx);      
          else
          if (bi->get_bpp()==32)
            add_bitmap_repeat_nn_rgba32 (bi, mat, cx);      
          else
            abort();
            
        } // if smooth
      } else {
        if (smooth) {
        
          if (bi->get_bpp()==24)
            add_bitmap_clip_aa_rgb24 (bi, mat, cx);      
          else
          if (bi->get_bpp()==32)
            add_bitmap_clip_aa_rgba32 (bi, mat, cx);      
          else
            abort();
            
        } else {
        
          if (bi->get_bpp()==24)
            add_bitmap_clip_nn_rgb24 (bi, mat, cx);      
          else
          if (bi->get_bpp()==32)
            add_bitmap_clip_nn_rgba32 (bi, mat, cx);      
          else
            abort();
            
        } // if smooth
      } // if repeat
      
    } // add_bitmap 


    // === RGB24 ===
    

    void add_bitmap_repeat_nn_rgb24(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // tiled, nearest neighbor method (faster)   

      typedef agg::pixfmt_rgb24_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef agg::wrap_mode_repeat wrap_type;
      typedef agg::image_accessor_wrap<PixelFormat, wrap_type, wrap_type> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgb_nn<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
        
        
    
    
    void add_bitmap_clip_nn_rgb24(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // clipped, nearest neighbor method (faster)   

      typedef agg::pixfmt_rgb24_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef image_accessor_clip_transp<PixelFormat> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgb_nn<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
    
    
    
    void add_bitmap_repeat_aa_rgb24(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // tiled, bilinear method (better quality)   

      typedef agg::pixfmt_rgb24_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef agg::wrap_mode_repeat wrap_type;
      typedef agg::image_accessor_wrap<PixelFormat, wrap_type, wrap_type> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgb_bilinear<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
        
    
    void add_bitmap_clip_aa_rgb24(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // clipped, bilinear method (better quality)   

      typedef agg::pixfmt_rgb24_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef image_accessor_clip_transp<PixelFormat> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgb_bilinear<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
    
       
    
    // === RGBA32 ===    

    void add_bitmap_repeat_nn_rgba32(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {
    
      // tiled, nearest neighbor method (faster)   

      typedef agg::pixfmt_rgba32_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef agg::wrap_mode_repeat wrap_type;
      typedef agg::image_accessor_wrap<PixelFormat, wrap_type, wrap_type> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);
          
      m_styles.push_back(st);
    }
        
        
    
    
    void add_bitmap_clip_nn_rgba32(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // clipped, nearest neighbor method (faster)   

      typedef agg::pixfmt_rgba32_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef image_accessor_clip_transp<PixelFormat> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgba_nn<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);
          
      m_styles.push_back(st);
    }
    
    
    
    void add_bitmap_repeat_aa_rgba32(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // tiled, bilinear method (better quality)   

      typedef agg::pixfmt_rgba32_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef agg::wrap_mode_repeat wrap_type;
      typedef agg::image_accessor_wrap<PixelFormat, wrap_type, wrap_type> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
        
    
    void add_bitmap_clip_aa_rgba32(agg_bitmap_info* bi,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {

      // clipped, bilinear method (better quality)   

      typedef agg::pixfmt_rgba32_pre PixelFormat;
      typedef agg::span_allocator<PixelFormat> span_allocator_type;
      typedef image_accessor_clip_transp<PixelFormat> img_source_type; 
      typedef agg::span_interpolator_linear_subdiv<agg::trans_affine> interpolator_type;
      typedef agg::span_image_filter_rgba_bilinear<img_source_type, interpolator_type> sg_type;
       
      typedef agg_style_bitmap<PixelFormat, span_allocator_type, img_source_type, 
        interpolator_type, sg_type> st_type;
      
      st_type* st = new st_type(bi->get_width(), bi->get_height(),
          bi->get_rowlen(), bi->get_data(), mat, cx);       
        
      m_styles.push_back(st);
    }
    
    
    // === GRADIENT ===

    void add_gradient_linear(const gnash::fill_style& fs,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {
    
      typedef agg::rgba8 color_type;            
      typedef agg::span_allocator<color_type> span_allocator_type;
      typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
      typedef agg::gradient_x gradient_func_type;
      //typedef agg::gradient_repeat_adaptor<gradient_func_type> gradient_adaptor_type;
      typedef gradient_func_type gradient_adaptor_type;
      typedef agg::gradient_lut<agg::color_interpolator<color_type>, 256> color_func_type;
      typedef agg::span_gradient<color_type,
                                 interpolator_type,
                                 gradient_adaptor_type,
                                 color_func_type> sg_type;
       
      typedef agg_style_gradient<color_type, span_allocator_type, 
        interpolator_type, gradient_func_type, gradient_adaptor_type, 
        color_func_type, sg_type> st_type;
      
      st_type* st = new st_type(fs, mat, cx, 256);
      
      // NOTE: The value 256 is based on the bitmap texture used by other
      // Gnash renderers which is normally 256x1 pixels for linear gradients.       
        
      m_styles.push_back(st);
    }
    

    void add_gradient_radial(const gnash::fill_style& fs,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {
    
      typedef agg::rgba8 color_type;            
      typedef agg::span_allocator<color_type> span_allocator_type;
      typedef agg::span_interpolator_linear<agg::trans_affine> 
          interpolator_type;
      typedef agg::gradient_radial gradient_func_type;
      typedef gradient_func_type gradient_adaptor_type;
      typedef agg::gradient_lut<agg::color_interpolator<color_type>, 256> 
          color_func_type;
      typedef agg::span_gradient<color_type,
                                 interpolator_type,
                                 gradient_adaptor_type,
                                 color_func_type> sg_type;
       
      typedef agg_style_gradient<color_type, span_allocator_type, 
        interpolator_type, gradient_func_type, gradient_adaptor_type, 
        color_func_type, sg_type> st_type;
      
      // move the center of the radial fill to where it should be
      gnash::SWFMatrix transl;
      transl.set_translation(-32, -32);
      transl.concatenate(mat);    

      // div 2 because we need radius, not diameter      
      st_type* st = new st_type(fs, transl, cx, 64/2); 
        
      // NOTE: The value 64 is based on the bitmap texture used by other
      // Gnash renderers which is normally 64x64 pixels for radial gradients.       
        
      m_styles.push_back(st);
    }

    void add_gradient_focal(const gnash::fill_style& fs,
        const gnash::SWFMatrix& mat, const gnash::cxform& cx)
    {
      typedef agg::rgba8 color_type;
      typedef agg::span_allocator<color_type> span_allocator_type;
      typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
      typedef agg::gradient_radial_focus gradient_func_type;
      typedef gradient_func_type gradient_adaptor_type;
      typedef agg::gradient_lut<agg::color_interpolator<color_type>, 256> color_func_type;
      typedef agg::span_gradient<color_type, interpolator_type,
        gradient_adaptor_type, color_func_type> sg_type;
    
      typedef agg_style_gradient<color_type, span_allocator_type,
        interpolator_type, gradient_func_type, gradient_adaptor_type,
        color_func_type, sg_type> st_type;
            
      // move the center of the focal fill (not its focal point) to where it 
      // should be.
      gnash::SWFMatrix transl;      
      transl.set_translation(-32, -32);
      transl.concatenate(mat);
      
      st_type* st = new st_type(fs, transl, cx, 64/2); 
      
      // re-initialize focal gradient settings
      gradient_adaptor_type& adaptor = st->get_gradient_adaptor();
      adaptor.init(32.0, fs.get_focal_point()*32.0, 0.0);
    
      m_styles.push_back(st);
    }

    /// Returns the color of a certain fill style (solid)
    agg::rgba8 color(unsigned style) const 
    {
        if (style < m_styles.size())
            return m_styles[style]->color();

        return m_transparent;
    }

    /// Called by AGG to generate a scanline span for non-solid fills 
    void generate_span(agg::rgba8* span, int x, int y,
        unsigned len, unsigned style)
    {
      m_styles[style]->generate_span(span,x,y,len);
    }


private:
    std::vector<agg_style_base*> m_styles;
    agg::rgba8          m_transparent;
};  // class agg_style_handler



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


} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_AGG_STYLE_H
