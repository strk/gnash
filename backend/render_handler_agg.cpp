// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

 


// Original version by Udo Giacomozzi and Hannes Mayr, 
// INDUNET GmbH (www.indunet.it)


/// A render_handler that uses the Anti-Grain Geometry Toolkit (antigrain.com)
/// and renders directly to a buffer (for example to the framebuffer). This 
/// backend is *completely* independent of any hardware. It can be used for
/// rendering to the Linux FrameBuffer device, or be blitted inside a 
/// window (regardless of what operating system). It should also be no problem
/// to render into a file...
/// This file uses *very* heavily templates and is optimized mainly for speed,
/// meaning that the compiler generates very, very, very much specialized 
/// code. That's good for speed but bloats up the resulting machine code. 

/*

Status
------

  outlines:
    solid             COMPLETE
    patterns          don't exist (they're converted at compile time by Flash!)
    widths            COMPLETE
    colors, alpha     COMPLETE
    cap styles        DONE, but end cap style is ignored
    join styles       COMPLETE
    no-close flag     COMPLETE        
  
    
  fills:
    solid fills       COMPLETE
    linear gradients  COMPLETE
    radial gradients  COMPLETE
    focal gradients   COMPLETE
    ext. spread modes NOT IMPLEMENTED *
    linear RGB mode   NOT IMPLEMENTED *
    bitmaps, tiled    COMPLETE
    bitmaps, clipped  COMPLETE
    bitmaps, smooth   COMPLETE
    bitmaps, hard     COMPLETE    
    color xform       COMPLETE
    
    * special fill styles are not yet supported by Gnash itself AFAIK, but AGG 
    supports them and it should be easy to add them.    
    
  fonts               COMPLETE
    
  masks               COMPLETE
  
  caching             NONE IMPLEMENTED
  
  video               COMPLETE
  
  Currently the renderer should be able to render everything correctly.
  
  
What could and should be /optimized/
------------------------------------  
  
  - EASY: Do not even start rendering shapes that are abviously out of the
    invalidated bounds!  
    
  - The alpha mask buffers (masks) are allocated and freed for each mask which
    results in many large-size buffer allocations during a second. Maybe this
    should be optimized.
    
  - Converted fill styles (for AGG) are recreated for each sub-shape, even if
    they never change for a shape. This should be changed.
    
  - Matrix-transformed paths (generated before drawing a shape) should be cached
    and re-used to avoid recalculation of the same coordinates.
    
  - Characters (or sprites) may be cached as bitmaps with alpha channel (RGBA).
    The mechanism could be automatically activated when the same character is
    being rendered the 3rd or 5th time in a row with the same transformations
    (regardless of the instance itself!). It's not a good idea to always
    render into a bitmap buffer because this eats up memory and adds an 
    additional pass in rendering (blitting the bitmap buffer). This may be
    tricky to implement anyway.
    
  - Masks are a very good candidate for bitmap caching as they do not change
    that often. With other words, the alpha mask should not be discarded after
    rendering and should be reused if possible.
    
  - there are also a few TODO comments in the code!
  
  
AGG ressources
--------------
  http://www.antigrain.com/    
  http://haiku-os.org/node/86

*/

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_PTHREADS
#include <pthread.h>
#endif

#include <vector>
#include <cmath>

#include "gnash.h"
#include "types.h"
#include "image.h"
#include "utility.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h" 
#include "Range2d.h"

#include "shape_character_def.h" 
#include "generic_character.h"

#include <agg_rendering_buffer.h>
#include <agg_renderer_base.h>
#include <agg_pixfmt_rgb.h>
#include <agg_pixfmt_rgb_packed.h>
#include <agg_pixfmt_rgba.h>
#include <agg_pixfmt_gray.h>
#include <agg_color_rgba.h>
#include <agg_color_gray.h>
#include <agg_ellipse.h>
#include <agg_conv_transform.h>
#include <agg_trans_affine.h>
#include <agg_scanline_u.h>
#include <agg_scanline_bin.h>
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
// must only include if render_scanlines_compound_layered is not defined
//#if ! HAVE_AGG_SCANLINES_COMPOUND_LAYERED
//#warning including compound
//#include "render_handler_agg_compat.h"
//#endif
#include <agg_rasterizer_scanline_aa.h>
#include <agg_rasterizer_compound_aa.h>
#include <agg_span_allocator.h>
#include <agg_path_storage.h>
#include <agg_conv_curve.h>
#include <agg_conv_stroke.h>
#include <agg_vcgen_stroke.h>
#include <agg_bezier_arc.h>
#include <agg_renderer_primitives.h>
#include <agg_gamma_functions.h>
#include <agg_math_stroke.h>
#include <agg_image_filters.h>
#include <agg_image_accessors.h>
#include <agg_span_image_filter_rgb.h>
#include <agg_span_image_filter_rgba.h>
#include <agg_span_interpolator_linear.h>
#include <agg_span_gradient.h>
#include <agg_gradient_lut.h>
#include <agg_alpha_mask_u8.h>

#include "render_handler_agg_bitmap.h"
#include "render_handler_agg_style.h"

//#include <boost/foreach.hpp>

#ifndef round
	#define round(x) rint(x)
#endif

#define TWIPS_TO_SHIFTED_PIXELS(x) (x*0.05f + 0.5f) 

#include <boost/numeric/conversion/converter.hpp>

using namespace gnash;


namespace gnash {


// --- CACHE -------------------------------------------------------------------
/// This class holds a completely transformed path (fixed position). Speeds
/// up characters that stay fixed on a certain position on the stage. 
// ***CURRENTLY***NOT***USED***

class agg_transformed_path 
{
public:
  /// Original transformation matrix 
  matrix m_mat;  
  
  /// Normal or rounded coordinates?
  bool m_rounded;
  
  /// Number of cache hits 
  int m_hits;
  
  /// Contents of this cache item (AGG path).  
  std::vector <agg::path_storage> m_data;
};

class agg_cache_manager : private render_cache_manager
{

  std::vector <agg_transformed_path> m_items;

  /// Looks for a matching pre-computed path in the cache list
  /// Returns NULL if no cache item matches 
  std::vector <agg::path_storage>* search(const matrix& mat, bool rounded) {
  
    const size_t ccount = m_items.size();
    
    for (size_t cno=0; cno<ccount; ++cno) {    
      agg_transformed_path& item = m_items[cno];
          
      if ((item.m_mat == mat) && (item.m_rounded == rounded)) {
      
        // Found it!
        return &item.m_data;
      
      }    
    }
    
    // could not find a matching item
    return NULL;
  
  }
  

};


// --- ALPHA MASK BUFFER CONTAINER ---------------------------------------------
// How masks are implemented: A mask is basically a full alpha buffer. Each 
// pixel in the alpha buffer defines the fraction of color values that are
// copied to the main buffer. The alpha mask buffer has 256 alpha levels per
// pixel, which is good as it allows anti-aliased masks. A full size buffer
// is allocated for each mask even if the invalidated bounds may be much 
// smaller. The advantage of this is that the alpha mask adaptor does not need
// to do any clipping which results in better performance.
// Masks can be nested, which means the intersection of all masks should be 
// visible (logical AND). To allow this we hold a stack of alpha masks and the 
// topmost mask is used itself as a mask to draw any new mask. When rendering 
// visible shapes only the topmost mask must be used and when a mask should not 
// be used anymore it's simply discarded so that the next mask becomes active 
// again.
// To be exact, Flash is a bit restrictive regarding to what can be a mask
// (dynamic text, shapes, ...) but our rebderer can build a mask from everything 
// we can draw otherwise (except lines, which are excluded explicitely).    

class agg_alpha_mask 
{

  typedef agg::renderer_base<agg::pixfmt_gray8> renderer_base;
  typedef agg::alpha_mask_gray8 amask_type;

public:

  agg_alpha_mask(int width, int height) :
    m_rbuf(NULL, width, height, width),    // *
    m_pixf(m_rbuf),
    m_rbase(m_pixf),
    m_amask(m_rbuf)
  {
  
    // * = m_rbuf is first initialized with a NULL buffer so that m_pixf and
    // m_rbase initialize with the correct buffer extents
  
    m_buffer = new boost::uint8_t[width*height];
    
    m_rbuf.attach(m_buffer, width, height, width);
    
    // NOTE: The buffer is *not* cleared. The clear() function must be called
    // to clear the buffer (alpha=0). The reason is to avoid clearing the 
    // whole mask when only a small portion is really used.

  }
  
  ~agg_alpha_mask() 
  {
    delete [] m_buffer;
  }
  
  void clear(const geometry::Range2d<int>& region)
  {
    if (region.isNull()) return;
    assert ( region.isFinite() );

    const agg::gray8 black(0);
        
    // region can't be world as it should be intersected with 
    // the visible rect
    assert (! region.isWorld() );

    unsigned int left=region.getMinX();
    unsigned int width=region.width()+1;

    const unsigned int max_y = region.getMaxY();
          for (unsigned int y=region.getMinY(); y<=max_y; ++y) 
    {
       m_pixf.copy_hline(left, y, width, black);
    }
  }
  
  renderer_base& get_rbase() {
    return m_rbase;
  }
  
  amask_type& get_amask() {
    return m_amask;
  }  
  
  
private:
  // in-memory buffer
  boost::uint8_t* m_buffer;
  
  // agg class to access the buffer
  agg::rendering_buffer m_rbuf;
  
  // pixel access
  agg::pixfmt_gray8 m_pixf;  
  
  // renderer base
  renderer_base m_rbase;
  
  // alpha mask
  amask_type m_amask;
};



// --- RENDER HANDLER ----------------------------------------------------------
// The class is implemented using templates so that it supports any kind of
// pixel format. LUT (look up tables) are not supported, however.

// Real AGG handler
template <class PixelFormat>
class render_handler_agg : public render_handler_agg_base
{
private:
  typedef agg::renderer_base<PixelFormat> renderer_base;
  
  typedef agg::conv_stroke< agg::conv_curve< agg::path_storage > > stroke_type;
  
  // TODO: Change these!!
  unsigned char *memaddr;
  int memsize;
  int xres;
  int yres;
  int bpp;  // bits per pixel
  // double xscale, yscale;  <-- deprecated, to be removed
  gnash::matrix stage_matrix;  // conversion from TWIPS to pixels
  bool scale_set;
  
  


public:

  // Enable/disable antialiasing.
  bool  m_enable_antialias;

  // Output size.
  float m_display_width;
  float m_display_height;

  void set_antialiased(bool enable) {
    // enable=false *forces* all bitmaps to be rendered in low quality
    m_enable_antialias = enable;
  }

  // Style state.
  enum style_index
  {
    LEFT_STYLE = 0,
    RIGHT_STYLE,
    LINE_STYLE,
    STYLE_COUNT
  };


  gnash::bitmap_info* create_bitmap_info_rgb(image::ImageRGB* im)
  // Given an image, returns a pointer to a bitmap_info class
  // that can later be passed to fill_styleX_bitmap(), to set a
  // bitmap fill style.
  {    
    return new agg_bitmap_info<agg::pixfmt_rgb24_pre> (im->width(), im->height(),
      im->pitch(), im->data(), 24);
    abort(); 
  }


  gnash::bitmap_info* create_bitmap_info_rgba(image::ImageRGBA* im)
  // Given an image, returns a pointer to a bitmap_info class
  // that can later be passed to fill_style_bitmap(), to set a
  // bitmap fill style.
  //
  // This version takes an image with an alpha channel.
  {
    return new agg_bitmap_info<agg::pixfmt_rgba32_pre> (im->width(), im->height(),
      im->pitch(), im->data(), 32); 
  }


  gnash::bitmap_info* create_bitmap_info_empty()
  // Create a placeholder bitmap_info.  Used when
  // DO_NOT_LOAD_BITMAPS is set; then later on the host program
  // can use movie_definition::get_bitmap_info_count() and
  // movie_definition::get_bitmap_info() to stuff precomputed
  // textures into these bitmap infos.
  {
    // bitmaps currently not supported! - return dummy for fontlib
    unsigned char dummy=0;
    return new agg_bitmap_info<agg::pixfmt_rgb24_pre> (0, 0, 0, &dummy, 24);
  }

  void drawVideoFrame(image::ImageBase* frame, const matrix* source_mat, 
    const rect* bounds) {
  
    // NOTE: Assuming that the source image is RGB 8:8:8
    
    // TODO: Currently only nearest-neighbor scaling is implemented here, since
    // it's the fastest and Flash apparently uses this method most of the time.
    // It would be easy to add other scaling methods (bilinear, bicubic, 
    // whatever), but we'd need some way to tell the renderer the desired
    // quality.
    
    // TODO: keep heavy instances alive accross frames for performance!
    
    // TODO: Maybe implement specialization for 1:1 scaled videos
    
    if (frame->type() == GNASH_IMAGE_RGBA)
    {
        LOG_ONCE(log_error(_("Can't render videos with alpha")));
        return;
    }
      
    typedef agg::pixfmt_rgb24_pre baseformat;

    assert(frame->type() == GNASH_IMAGE_RGB);
    
    matrix mat = stage_matrix;
    mat.concatenate(*source_mat);
    
    // compute video scaling relative to video obejct size
    double vscaleX = TWIPS_TO_PIXELS(bounds->width())  / frame->width();
    double vscaleY = TWIPS_TO_PIXELS(bounds->height()) / frame->height();
    
    // convert Gnash matrix to AGG matrix and scale down to pixel coordinates
    // while we're at it
    agg::trans_affine img_mtx(
      mat.sx  / 65536.0, mat.shx / 65536.0, 
      mat.shy / 65536.0, mat.sy / 65536.0, 
      mat.tx, mat.ty
    );    
    
    // invert matrix since this is used for the image source
    img_mtx.invert();
    
    // convert TWIPS to pixels and apply video scale
    img_mtx *= agg::trans_affine_scaling(1.0/(20.0*vscaleX), 1.0/(20.0*vscaleY));
    
    // span allocator is used to apply the matrix
    agg::span_allocator<agg::rgba8> sa;
        
    typedef agg::span_interpolator_linear<> interpolator_type;
    interpolator_type interpolator(img_mtx);
    
    // cloning image accessor is used to avoid disturbing pixels at the edges
    // for rotated video. 
    typedef agg::image_accessor_clone<baseformat> img_source_type;
    
    // rendering buffer is used to access the frame pixels here        
    agg::rendering_buffer img_buf(frame->data(), frame->width(), frame->height(),
      frame->pitch());
         
    baseformat img_pixf(img_buf);
    
    img_source_type img_src(img_pixf);
    
    // renderer base for the stage buffer (not the frame image!)
    renderer_base rbase(*m_pixf);
        
    // nearest neighbor method for scaling
    typedef agg::span_image_filter_rgb_nn<img_source_type, interpolator_type>
      span_gen_type;
    span_gen_type sg(img_src, interpolator);
      
    typedef agg::rasterizer_scanline_aa<> ras_type;
    ras_type ras;
    
    // make a path for the video outline
    point a, b, c, d;
    mat.transform(&a, point(bounds->get_x_min(), bounds->get_y_min()));
    mat.transform(&b, point(bounds->get_x_max(), bounds->get_y_min()));
    mat.transform(&c, point(bounds->get_x_max(), bounds->get_y_max()));
    mat.transform(&d, point(bounds->get_x_min(), bounds->get_y_max()));
    
    agg::path_storage path;
    path.move_to(a.x, a.y);
    path.line_to(b.x, b.y);
    path.line_to(c.x, c.y);
    path.line_to(d.x, d.y);
    path.line_to(a.x, a.y);

    if (m_alpha_mask.empty()) {
    
      // No mask active

      agg::scanline_u8 sl;
  
      for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {    
      
        const geometry::Range2d<int>& cbounds = _clipbounds[cno];
        apply_clip_box<ras_type> (ras, cbounds);
  
        // <Udo>: AFAIK add_path() rewinds the vertex list (clears previous
        // path), so there should be no problem with multiple clipbounds.      
        ras.add_path(path);     
           
        agg::render_scanlines_aa(ras, sl, rbase, sa, sg);
      }
      
    } else {
    
      // Mask is active!
      
      // **** UNTESTED!!! ****
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      scanline_type sl(m_alpha_mask.back()->get_amask());
  
      for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {    
      
        const geometry::Range2d<int>& cbounds = _clipbounds[cno];
        apply_clip_box<ras_type> (ras, cbounds);
  
        // <Udo>: AFAIK add_path() rewinds the vertex list (clears previous
        // path), so there should be no problem with multiple clipbounds.      
        ras.add_path(path);     
           
        agg::render_scanlines_aa(ras, sl, rbase, sa, sg);
      }
      
    
    } // if alpha mask
    
  } // drawVideoFrame
  

  // Constructor
  render_handler_agg(int bits_per_pixel)
      :
      // Initialization list
      memaddr(NULL),
      memsize(0),
      xres(1),
      yres(1),
      bpp(bits_per_pixel),
      /*xscale(1.0/20.0),
      yscale(1.0/20.0),*/
      scale_set(false),
      m_enable_antialias(true),
      m_display_width(0.0),
      m_display_height(0.0),
      m_drawing_mask(false)
  {
    // TODO: we really don't want to set the scale here as the core should
    // tell us the right values before rendering anything. However this is
    // currently difficult to implement. Removing the next call will
    // lead to an assertion failure in begin_display() because we check
    // whether the scale is known there.
    set_scale(1.0f, 1.0f);
  }   

  // Destructor
  ~render_handler_agg()
  {
  }

  /// Initializes the rendering buffer. The memory pointed by "mem" is not
  /// owned by the renderer and init_buffer() may be called multiple times
  /// when the buffer size changes, for example. However, bits_per_pixel must
  /// remain the same. 
  /// rowstride is the size, in bytes, of one row.
  /// This method *must* be called prior to any other method of the class!
  void init_buffer(unsigned char *mem, int size, int x, int y, int rowstride)
  {
        assert(x > 0);
        assert(y > 0);

    memaddr = mem;
    memsize = size;
    xres    = x;
    yres    = y;
    
    m_rbuf.attach(memaddr, xres, yres, rowstride);

    // allocate pixel format accessor   
    m_pixf.reset(new PixelFormat(m_rbuf));
    //m_rbase = new renderer_base(*m_pixf);  --> does not work!!??
    
    // by default allow drawing everywhere
    set_invalidated_region_world();
    
    log_debug(_("Initialized AGG buffer <%p>, %d bytes, %dx%d, rowsize is %d bytes"), 
      (void*)mem, size, x, y, rowstride);
  }
  

  void begin_display(
  const gnash::rgba& background_color,
  int /*viewport_x0*/, int /*viewport_y0*/,
  int /*viewport_width*/, int /*viewport_height*/,
  float /*x0*/, float /*x1*/, float /*y0*/, float /*y1*/)
  // Set up to render a full frame from a movie and fills the
  // background.  Sets up necessary transforms, to scale the
  // movie to fit within the given dimensions.  Call
  // end_display() when you're done.
  //
  // The rectangle (viewport_x0, viewport_y0, viewport_x0 +
  // viewport_width, viewport_y0 + viewport_height) defines the
  // window coordinates taken up by the movie.
  //
  // The rectangle (x0, y0, x1, y1) defines the pixel
  // coordinates of the movie that correspond to the viewport
  // bounds.
  {
    assert(m_pixf.get());
    
    assert(scale_set);
    // clear the stage using the background color
    for (unsigned int i=0; i<_clipbounds.size(); ++i) 
      clear_framebuffer(_clipbounds[i], agg::rgba8_pre(background_color.m_r,
        background_color.m_g, background_color.m_b,
        background_color.m_a));        
    
    // reset status variables
    m_drawing_mask = false;
  }
  
  /// renderer_base.clear() does no clipping which clears the whole framebuffer
  /// even if we update just a small portion of the screen. The result would be
  /// still correct, but slower. 
  /// This function clears only a certain portion of the screen, while /not/ 
  /// being notably slower for a fullscreen clear. 
  void clear_framebuffer(const geometry::Range2d<int>& region,
        agg::rgba8 color)
  {
      assert(region.isFinite());
      
      // add 1 to width since we have still to draw a pixel when 
      // getMinX==getMaxX     
      unsigned int width=region.width()+1;
      
      // <Udo> Note: We don't need to check for width/height anymore because
      // Range2d will take care that getMinX <= getMaxX and it's okay when
      // region.width()==0 because in that case getMinX==getMaxX and we have
      // still a pixel to draw. 

      unsigned int left=region.getMinX();

      for (unsigned int y=region.getMinY(), maxy=region.getMaxY();
        y<=maxy; ++y) 
      {
        m_pixf->copy_hline(left, y, width, color);
      }
  }

  void  end_display()
  // Clean up after rendering a frame.  Client program is still
  // responsible for calling glSwapBuffers() or whatever.
  {
  
    if (m_drawing_mask) 
      log_debug(_("Warning: rendering ended while drawing a mask"));
      
    while (! m_alpha_mask.empty()) {
      log_debug(_("Warning: rendering ended while masks were still active"));
      disable_mask();      
    }
  
    // nothing to do
  }

  template <class ras_type>
  void apply_clip_box(ras_type& ras, 
    const geometry::Range2d<int>& bounds)
  {
    assert(bounds.isFinite());
    ras.clip_box(
      (double)bounds.getMinX(),
      (double)bounds.getMinY(),
      (double)bounds.getMaxX()+1,
      (double)bounds.getMaxY()+1);  
  }


  
  void  draw_line_strip(const boost::int16_t* coords, int vertex_count, const rgba& color,
                  const matrix& line_mat)
  // Draw the line strip formed by the sequence of points.
  {
    assert(m_pixf.get());

    matrix mat = stage_matrix;
    mat.concatenate(line_mat);    

    if ( _clipbounds.empty() ) return;

    point pnt;
    
    renderer_base rbase(*m_pixf);
    
    typedef agg::rasterizer_scanline_aa<> ras_type;

    ras_type ras;    
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase);
      
    // -- create path --
    agg::path_storage path;
    agg::conv_stroke<agg::path_storage> stroke(path);
    stroke.width(1);
    stroke.line_cap(agg::round_cap);
    stroke.line_join(agg::round_join);
    path.remove_all(); // Not obligatory in this case

    const boost::int16_t *vertex = coords;
    
    mat.transform(&pnt, point(vertex[0], vertex[1]));
    path.move_to(pnt.x, pnt.y);

    for (vertex += 2;  vertex_count > 1;  vertex_count--, vertex += 2) {
      mat.transform(&pnt, point(vertex[0], vertex[1]));
      path.line_to(pnt.x, pnt.y);
    }
    
    // -- render --
    
    if (m_alpha_mask.empty()) {
    
      // No mask active
      
      agg::scanline_p8 sl;      
      
      for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {
      
        const geometry::Range2d<int>& bounds = _clipbounds[cno];
              
        apply_clip_box<ras_type> (ras, bounds);
        
        // The vectorial pipeline
        ras.add_path(stroke);
    
        // Set the color and render the scanlines
        ren_sl.color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b, color.m_a));
        
        agg::render_scanlines(ras, sl, ren_sl);     
        
      }
      
    } else {
    
      // Mask is active!

      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> sl_type;
      
      sl_type sl(m_alpha_mask.back()->get_amask());      
      
      for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {
      
        const geometry::Range2d<int>& bounds = _clipbounds[cno];
              
        apply_clip_box<ras_type> (ras, bounds);
        
        // The vectorial pipeline
        ras.add_path(stroke);
    
        // Set the color and render the scanlines
        ren_sl.color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b, color.m_a));
        
        agg::render_scanlines(ras, sl, ren_sl);     
        
      }
    
    }

  } // draw_line_strip


  void begin_submit_mask()
  {
    // Set flag so that rendering of shapes is simplified (only solid fill) 
    m_drawing_mask = true;
    
    agg_alpha_mask* new_mask = new agg_alpha_mask(xres, yres);
    
    for (unsigned int cno=0; cno<_clipbounds.size(); ++cno)  
      new_mask->clear(_clipbounds[cno]);
    
    m_alpha_mask.push_back(new_mask);
    
  }

  void end_submit_mask()
  {
    m_drawing_mask = false;
  }

  void disable_mask()
  {
      assert( ! m_alpha_mask.empty() );
      delete m_alpha_mask.back();
      m_alpha_mask.pop_back();
  }
  

  void draw_glyph(shape_character_def *def,
      const matrix& mat, const rgba& color) 
  {
    std::vector<path> paths;
    apply_matrix_to_path(def->get_paths(), paths, mat);
    // convert gnash paths to agg paths.
    std::vector<agg::path_storage> agg_paths;    
    build_agg_paths(agg_paths, paths);
 
    // make sure m_single_fill_styles contains the required color 
    need_single_fill_style(color);

    // prepare style handler
    agg_style_handler sh;
    build_agg_styles(sh, m_single_fill_styles, mat, m_neutral_cxform);
    
    // select relevant clipping bounds
    if (def->get_bound().is_null())   // can happen (spaces? to be investigated..)
      select_all_clipbounds();
    else
      select_clipbounds(def, mat);
    
    
    if (_clipbounds_selected.empty()) return; // nothing to draw
      
    // draw the shape
    if (m_drawing_mask)
      draw_mask_shape(paths, false);
    else
      draw_shape(-1, paths, agg_paths, sh, false);
    
    // NOTE: Do not use even-odd filling rule for glyphs!
    
    // clear clipping ranges to ease debugging
    _clipbounds_selected.clear();
  }


  /// Fills _clipbounds_selected with pointers to _clipbounds members who
  /// intersect with the given character (transformed by mat). This avoids
  /// rendering of characters outside a particular clipping range.
  /// "_clipbounds_selected" is used by draw_shape() and draw_outline() and
  /// *must* be initialized prior to using those function.
  void select_clipbounds(const shape_character_def *def, 
    const matrix& source_mat) {
    
    matrix mat = stage_matrix;
    mat.concatenate(source_mat);
  
    _clipbounds_selected.clear();
    _clipbounds_selected.reserve(_clipbounds.size());
    
    rect ch_bounds = def->get_bound();

    if (ch_bounds.is_null()) {
      log_debug(_("Warning: select_clipbounds encountered a character definition "
        "with null bounds"));
      return;
    }   

    rect bounds;    
    bounds.set_null();
    bounds.expand_to_transformed_rect(mat, ch_bounds);
    
    const geometry::Range2d<float>& range_float = bounds.getRange();
    
    assert(range_float.isFinite());
    
    geometry::Range2d<int> range_int(
      (int) range_float.getMinX(),
      (int) range_float.getMinY(),
      (int) range_float.getMaxX(),
      (int) range_float.getMaxY()
    );
    
    
    const int count = _clipbounds.size();
    for (int cno=0; cno<count; ++cno) {
          
      if (_clipbounds[cno].intersects(bounds.getRange())) 
        _clipbounds_selected.push_back(&_clipbounds[cno]);

    }  
  }
  
  void select_all_clipbounds() {
  
    const unsigned int count = _clipbounds.size();

    if (_clipbounds_selected.size() == count)
      return; // already all selected
  
    _clipbounds_selected.clear();
    _clipbounds_selected.resize(count);
    
    for (unsigned int cno=0; cno<count; ++cno) 
      _clipbounds_selected[cno] = &_clipbounds[cno];
  }

  void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles) {
    
    bool have_shape, have_outline;

    analyze_paths(def->get_paths(), have_shape, have_outline);

    if (!have_shape && !have_outline)
    {
        return; // invisible character
    }    

    std::vector< path > paths;
    std::vector< agg::path_storage > agg_paths;  
    std::vector< agg::path_storage > agg_paths_rounded;  

    apply_matrix_to_path(def->get_paths(), paths, mat);

    // Flash only aligns outlines. Probably this is done at rendering
    // level.
    if (have_outline)
      build_agg_paths_rounded(agg_paths_rounded, paths, line_styles);   
    if (have_shape)
      build_agg_paths(agg_paths, paths);
    
    if (m_drawing_mask) {
      
      // Shape is drawn inside a mask, skip sub-shapes handling and outlines
      draw_mask_shape(paths, false);   // never use even-odd for masks
    
    } else {
    
      // select ranges
      select_clipbounds(def, mat);
      
      if (_clipbounds_selected.empty()) {
        log_debug(_("Warning: AGG renderer skipping a whole character"));
        return; // nothing to draw!?
      }
    
      // prepare fill styles
      agg_style_handler sh;
      if (have_shape)
        build_agg_styles(sh, fill_styles, mat, cx);
      
      /*
      // prepare strokes
      std::vector<stroke_type*> strokes;
      build_agg_strokes(strokes, agg_paths, paths, line_styles, mat);
      */
      
      // We need to separate sub-shapes during rendering. 
      const unsigned int subshape_count = count_sub_shapes(paths);
     
      for (unsigned int subshape=0; subshape<subshape_count; ++subshape)
      {
        if (have_shape)
        {
          draw_shape(subshape, paths, agg_paths, sh, true);    
        }
        if (have_outline)      
        {
          draw_outlines(subshape, paths, agg_paths_rounded, line_styles, cx, mat);
        }
      }
      
    } // if not drawing mask
    
    // Clear selected clipbounds to ease debugging 
    _clipbounds_selected.clear();
    
  } // draw_shape_character
  
  
  /// Analyzes a set of paths to detect real presence of fills and/or outlines
  /// TODO: This should be something the character tells us and should be 
  /// cached. 
  void analyze_paths(const std::vector<path> &paths, bool& have_shape,
    bool& have_outline) {
    
    have_shape=false;
    have_outline=false;
    
    const int pcount = paths.size();
    
    for (int pno=0; pno<pcount; ++pno) {
    
      const path &the_path = paths[pno];
    
      if ((the_path.m_fill0>0) || (the_path.m_fill1>0)) {
        have_shape=true;
        if (have_outline) return; // have both
      }
    
      if (the_path.m_line>0) {
        have_outline=true;
        if (have_shape) return; // have both
      }
    
    }
    
  }

/// Takes a path and translates it using the given matrix. The new path
/// is stored in paths_out. Both paths_in and paths_out are expected to
/// be in TWIPS.
void apply_matrix_to_path(const std::vector<path> &paths_in, 
      std::vector<path>& paths_out, const matrix &source_mat) 
{

    matrix mat;
    // make sure paths_out is also in TWIPS to keep accuracy.
    mat.concatenate_scale(20.0,  20.0);
    mat.concatenate(stage_matrix);
    mat.concatenate(source_mat);

    size_t pcnt = paths_in.size();
    paths_out.resize(pcnt);
    typedef std::vector<path> PathVect;
    for (PathVect::const_iterator i=paths_in.begin(), e=paths_in.end(); i!=e; ++i)
    {
        path  p = *i;
        p.transform(mat);
        paths_out.push_back( p );
    }
} // apply_matrix


  /// A shape can have sub-shapes. This can happen when there are multiple
  /// layers of the same frame count. Flash combines them to one single shape.
  /// The problem with sub-shapes is, that outlines can be hidden by other
  /// layers so they must be rendered separately. 
  unsigned int count_sub_shapes(const std::vector<path> &path_in)
  {
    unsigned int sscount=1;
    const size_t pcnt = path_in.size();
    
    for (size_t pno=0; pno<pcnt; ++pno) {
      const path& this_path = path_in[pno];
      
      if (this_path.m_new_shape)
        sscount++;
    }
    
    return sscount;
  }

  /// Transposes Gnash paths to AGG paths, which can be used for both outlines
  /// and shapes. Subshapes are ignored (ie. all paths are converted). Converts 
  /// TWIPS to pixels on the fly.
  void build_agg_paths(std::vector<agg::path_storage>& dest, const std::vector<path>& paths) 
  {
    //const double subpixel_offset = 0.5; // unused, should be ?
    size_t pcnt = paths.size();
    dest.resize(pcnt);
    
    for (size_t pno=0; pno<pcnt; ++pno)
    {
        const path& path_in_sub = paths[pno]; 
        agg::path_storage& new_path = dest[pno];

        new_path.move_to(TWIPS_TO_SHIFTED_PIXELS(path_in_sub.ap.x), 
                          TWIPS_TO_SHIFTED_PIXELS(path_in_sub.ap.y));

        const size_t ecnt = path_in_sub.m_edges.size();
        for (size_t eno=0; eno<ecnt; ++eno)
        {
            const edge& this_edge = path_in_sub.m_edges[eno];             
            if (this_edge.is_straight())
            {
                new_path.line_to(TWIPS_TO_SHIFTED_PIXELS(this_edge.ap.x), 
                                  TWIPS_TO_SHIFTED_PIXELS(this_edge.ap.y));
            }
            else
            {
                new_path.curve3(TWIPS_TO_SHIFTED_PIXELS(this_edge.cp.x), 
                                 TWIPS_TO_SHIFTED_PIXELS(this_edge.cp.y),
                                 TWIPS_TO_SHIFTED_PIXELS(this_edge.ap.x), 
                                 TWIPS_TO_SHIFTED_PIXELS(this_edge.ap.y));       
            }
        }// end of for    
    } // end of for  
    
  } //build_agg_paths

  // Version of build_agg_paths that uses rounded coordinates (pixel hinting)
  // for line styles that want it.  
  // This is used for outlines which are aligned to the pixel grid to avoid
  // anti-aliasing problems (a perfect horizontal line being drawn over two
  // lines and looking blurry). The proprietary player does this too.  
  //
  // Not all points are aligned, only those lines that:
  //   - are straight
  //   - are pure horizontal or vertical
  // Also, single segments of a path may be aligned or not depending on 
  // the segment properties (this matches MM player behaviour)
  //
  // This function - in contrast to build_agg_paths() - also checks noClose 
  // flag and automatically closes polygons.
  //
  // TODO: Flash never aligns lines that are wider than 1 pixel on *screen*,
  // but we currently don't check the width.  
  void build_agg_paths_rounded(std::vector<agg::path_storage>& dest, 
    const std::vector< path >& paths, 
    const std::vector<line_style>& line_styles) {

    const float subpixel_offset = 0.5f;
    
    const size_t pcount = paths.size();

    dest.resize(pcount);    
    
    for (size_t pno=0; pno<pcount; ++pno) {
      
      const path& this_path = paths[pno];
      agg::path_storage& new_path = dest[pno];
      
      bool hinting=false, closed=false, hairline=false;
      
      if (this_path.m_line) {
        const line_style& lstyle = line_styles[this_path.m_line-1];
        
        hinting = lstyle.doPixelHinting();
        closed = this_path.isClosed() && !lstyle.noClose();
        
        // check if this line is a hairline ON SCREEN
        // TODO: we currently only check for hairlines per definiton, not
        // for thin lines that become hair lines due to scaling
        if (lstyle.getThickness()<=20)
          hairline = true;
      }
      
      float prev_ax = TWIPS_TO_PIXELS(this_path.ap.x);
      float prev_ay = TWIPS_TO_PIXELS(this_path.ap.y);  
      bool prev_align_x = true;
      bool prev_align_y = true;
      
      size_t ecount = this_path.m_edges.size();

      // avoid extra edge when doing implicit close later
      if (closed && ecount && 
        this_path.m_edges.back().is_straight()) ecount--;      
      
      for (size_t eno=0; eno<ecount; ++eno) {
        
        const edge& this_edge = this_path.m_edges[eno];
        
        float this_ax = TWIPS_TO_PIXELS(this_edge.ap.x);  
        float this_ay = TWIPS_TO_PIXELS(this_edge.ap.y);  
        
        if (hinting || this_edge.is_straight()) {
        
          // candidate for alignment?
          bool align_x = hinting || (hairline && (prev_ax == this_ax));
          bool align_y = hinting || (hairline && (prev_ay == this_ay));
          
          if (align_x) 
            this_ax = round(this_ax);
          
          if (align_y)
            this_ay = round(this_ay);
          
          // first line?
          if (eno==0) {
          
            if (align_x) 
              prev_ax = round(prev_ax);
              
            if (align_y)
              prev_ay = round(prev_ay);
              
            new_path.move_to(prev_ax + subpixel_offset, 
              prev_ay + subpixel_offset);
            
          } else {
          
            // not the first line, but the previous anchor point
            // might belong to a curve and thus may not be aligned.
            // We need to have both anchors of this new line to be
            // aligned, so it may be neccesary to add a line
            if ((align_x && !prev_align_x) || (align_y && !prev_align_y)) {
            
              if (align_x) 
                prev_ax = round(prev_ax);
                
              if (align_y)
                prev_ay = round(prev_ay);
                
              new_path.line_to(prev_ax + subpixel_offset, 
                prev_ay + subpixel_offset);
              
            }
            
            // TODO: (minor flaw) Flash player never aligns anchor points
            // of curves, even if they are attached to straight vertical
            // or horizontal lines. It can be seen easily with rounded
            // rectangles, where the curves are never aligned and all 
            // straight lines are. AGG backend will align the curve anchor
            // point that follows the straight line. It's not a big problem
            // but it's not exact...
          
          }
        
          new_path.line_to(this_ax + subpixel_offset, 
            this_ay + subpixel_offset);
          
          prev_align_x = align_x;
          prev_align_y = align_y;  
          
          
        } else {
          
          // first line?
          if (eno==0) 
            new_path.move_to(prev_ax, prev_ay);
        
          // never align curves!
          new_path.curve3(
            TWIPS_TO_PIXELS(this_edge.cp.x) + subpixel_offset, 
            TWIPS_TO_PIXELS(this_edge.cp.y) + subpixel_offset,
            this_ax + subpixel_offset, 
            this_ay + subpixel_offset);
            
          prev_align_x = false;
          prev_align_y = false;  
            
        }
        
        prev_ax = this_ax;
        prev_ay = this_ay;    
        
      } //for
      
      if (closed)
        new_path.close_polygon();
    
    }
  } //build_agg_paths_rounded
    
  // Initializes the internal styles class for AGG renderer
  void build_agg_styles(agg_style_handler& sh, 
    const std::vector<fill_style>& fill_styles,
    const matrix& fillstyle_matrix,
    const cxform& cx) {
    
    matrix inv_stage_matrix = stage_matrix;
    inv_stage_matrix.invert();
    
    const size_t fcount = fill_styles.size();
    for (size_t fno=0; fno<fcount; ++fno) {
    
      bool smooth=false;
      int fill_type = fill_styles[fno].get_type();
      
      switch (fill_type) {

        case SWF::FILL_LINEAR_GRADIENT:
        {    
          matrix m = fill_styles[fno].get_gradient_matrix();
          matrix cm = fillstyle_matrix;
          cm.invert();
          
          m.concatenate(cm);
          m.concatenate(inv_stage_matrix);
          
          sh.add_gradient_linear(fill_styles[fno], m, cx);
          break;
        } 

        case SWF::FILL_RADIAL_GRADIENT:
        {
          matrix m = fill_styles[fno].get_gradient_matrix();
          matrix cm = fillstyle_matrix;
          cm.invert();
          
          m.concatenate(cm);
          m.concatenate(inv_stage_matrix);
          
          sh.add_gradient_radial(fill_styles[fno], m, cx);
          break;
        } 

        case SWF::FILL_FOCAL_GRADIENT:
        {
          matrix m = fill_styles[fno].get_gradient_matrix();
          matrix cm = fillstyle_matrix;
          cm.invert();
          
          m.concatenate(cm);
          m.concatenate(inv_stage_matrix);
          
          sh.add_gradient_focal(fill_styles[fno], m, cx);
          break;
        }

        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
        smooth=true;  // continue with next case!
        
        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        {    
          matrix m = fill_styles[fno].get_bitmap_matrix();
          matrix cm = fillstyle_matrix;
          cm.invert();
          
          m.concatenate(cm);
          m.concatenate(inv_stage_matrix);

          sh.add_bitmap(dynamic_cast<agg_bitmap_info_base*> 
            (fill_styles[fno].get_bitmap_info()), m, cx, 
            (fill_type==SWF::FILL_TILED_BITMAP) || (fill_type==SWF::FILL_TILED_BITMAP_HARD),
            smooth && m_enable_antialias);
          break;
        } 

        case SWF::FILL_SOLID:
        default:
        {            
          rgba color = cx.transform(fill_styles[fno].get_color());

          // add the color to our self-made style handler (basically just a list)
          sh.add_color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b, color.m_a));
        } 
        
      } // switch
        
    } // for
    
  } //build_agg_styles
  

  /// Draws the given path using the given fill style and color transform.
  //
  /// Normally, Flash shapes are drawn using even-odd filling rule. However,
  /// for glyphs non-zero filling rule should be used (even_odd=0).
  /// Note the paths have already been transformed by the matrix and 
  /// 'subshape_id' defines which sub-shape should be drawn (-1 means all 
  /// subshapes).
  ///
  /// Note the *coordinates* in "paths" are not used because they are 
  /// already prepared in agg_paths. The (nearly ambiguous) "path" parameter
  /// is used to access other properties like fill styles and subshapes.   
  ///
  /// @param subshape_id
  ///    Defines which subshape to draw. -1 means all subshapes.
  ///
  void draw_shape(int subshape_id, const std::vector<path> &paths,
    const std::vector<agg::path_storage>& agg_paths,  
    agg_style_handler& sh, int even_odd) {
    
    if (m_alpha_mask.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_shape_impl<scanline_type> (subshape_id, paths, agg_paths, 
        sh, even_odd, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(m_alpha_mask.back()->get_amask());
      
      draw_shape_impl<scanline_type> (subshape_id, paths, agg_paths, 
        sh, even_odd, sl);
        
    }
    
  }
   
  /// Template for draw_shape(). Two different scanline types are suppored, 
  /// one with and one without an alpha mask. This makes drawing without masks
  /// much faster.  
  template <class scanline_type>
  void draw_shape_impl(int subshape_id, const std::vector<path> &paths,
    const std::vector<agg::path_storage>& agg_paths,
    agg_style_handler& sh, int even_odd, scanline_type& sl) {
    /*
    Fortunately, AGG provides a rasterizer that fits perfectly to the flash
    data model. So we just have to feed AGG with all data and we're done. :-)
    This is also far better than recomposing the polygons as the rasterizer
    can do everything in one pass and it is also better for adjacent edges
    (anti aliasing).
    Thank to Maxim Shemanarev for providing us such a great tool with AGG...
    */
    
    assert(m_pixf.get());
    
    assert(!m_drawing_mask);
    
    if ( _clipbounds.empty() ) return;

    // AGG stuff
    typedef agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> ras_type;
    renderer_base rbase(*m_pixf);
    ras_type rasc;  // flash-like renderer
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::span_allocator<agg::rgba8> alloc;  // span allocator (?)
    

    // activate even-odd filling rule
    if (even_odd)
      rasc.filling_rule(agg::fill_even_odd);
    else
      rasc.filling_rule(agg::fill_non_zero);
      
    
    for (unsigned int cno=0; cno<_clipbounds_selected.size(); ++cno) {
    
      const geometry::Range2d<int>* bounds = _clipbounds_selected[cno];
      
      apply_clip_box<ras_type> (rasc, *bounds);
      
      int current_subshape=0;
        
      // push paths to AGG
      const size_t pcount = paths.size();
  
      for (size_t pno=0; pno<pcount; ++pno) {
          
        const path &this_path_gnash = paths[pno];
        agg::path_storage &this_path_agg = 
          const_cast<agg::path_storage&>(agg_paths[pno]);
        agg::conv_curve< agg::path_storage > curve(this_path_agg);        
        
        if (this_path_gnash.m_new_shape)
          ++current_subshape;
          
        if ((subshape_id>=0) && (current_subshape!=subshape_id)) {
          // Skip this path as it is not part of the requested sub-shape.
          continue;
        }
        
        if ((this_path_gnash.m_fill0==0) && (this_path_gnash.m_fill1==0)) {
          // Skip this path as it contains no fill style
          continue;
        } 
                
        
        // Tell the rasterizer which styles the following path will use.
        // The good thing is, that it already supports two fill styles out of
        // the box. 
        // Flash uses value "0" for "no fill", whereas AGG uses "-1" for that. 
        rasc.styles(this_path_gnash.m_fill0-1, this_path_gnash.m_fill1-1);
                
        // add path to the compound rasterizer
        rasc.add_path(curve);
      
      }
      //log_debug("%d edges\n", edge_count);
      
              
      agg::render_scanlines_compound_layered(rasc, sl, rbase, alloc, sh);
    }
    
  } // draw_shape_impl




  // very similar to draw_shape but used for generating masks. There are no
  // fill styles nor subshapes and such. Just render plain solid shapes.
  void draw_mask_shape(const std::vector<path> &paths, int even_odd) {

    unsigned int mask_count = m_alpha_mask.size();
    
    if (mask_count < 2) {
    
      // This is the first level mask
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_mask_shape_impl<scanline_type> (paths, even_odd, sl);
        
    } else {
    
      // Woohoo! We're drawing a nested mask! Use the previous mask while 
      // drawing the new one, the result will be the intersection.
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(m_alpha_mask[mask_count-2]->get_amask());
      
      draw_mask_shape_impl<scanline_type> (paths, even_odd, sl);
        
    }
    
  }
  
  
  template <class scanline_type>
  void draw_mask_shape_impl(const std::vector<path> &paths, int even_odd,
    scanline_type& sl) {
    
    typedef agg::pixfmt_gray8 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    
    assert(!m_alpha_mask.empty());
    
    // dummy style handler
    typedef agg_mask_style_handler sh_type;
    sh_type sh;                   
       
    // compound rasterizer used for flash shapes
    typedef agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc_type;  
    rasc_type rasc;
    
    // renderer base
    renderer_base& rbase = m_alpha_mask.back()->get_rbase();
    
    // solid fills
    typedef agg::renderer_scanline_aa_solid< renderer_base > ren_sl_type;
    ren_sl_type ren_sl(rbase);
    
    // span allocator
    typedef agg::span_allocator<agg::gray8> alloc_type;
    alloc_type alloc;   // why does gray8 not work?
      

    // activate even-odd filling rule
    if (even_odd)
      rasc.filling_rule(agg::fill_even_odd);
    else
      rasc.filling_rule(agg::fill_non_zero);
      
    
    // push paths to AGG
    agg::path_storage path; // be carefull about this name 
    agg::conv_curve< agg::path_storage > curve(path);

    for (size_t pno=0, pcount=paths.size(); pno < pcount; ++pno) {

      const Path& this_path = paths[pno];

      path.remove_all();
      
      // reduce everything to just one fill style!
      rasc.styles(this_path.m_fill0==0 ? -1 : 0,
                  this_path.m_fill1==0 ? -1 : 0);
                  
      // starting point of path
      path.move_to(TWIPS_TO_PIXELS(this_path.ap.x), 
                   TWIPS_TO_PIXELS(this_path.ap.y));
    
      const unsigned int ecount = this_path.m_edges.size();
      for (unsigned int eno=0; eno<ecount; ++eno) {

        const edge &this_edge = this_path.m_edges[eno];

        if (this_edge.is_straight())
          path.line_to(TWIPS_TO_PIXELS(this_edge.ap.x), 
                       TWIPS_TO_PIXELS(this_edge.ap.y));
        else
          path.curve3(TWIPS_TO_PIXELS(this_edge.cp.x), 
                      TWIPS_TO_PIXELS(this_edge.cp.y),
                      TWIPS_TO_PIXELS(this_edge.ap.x), 
                      TWIPS_TO_PIXELS(this_edge.ap.y));
        
      } // for edge
      
      // add to rasterizer
      rasc.add_path(curve);
    
    } // for path
    
    
    // now render that thing!
    agg::render_scanlines_compound_layered (rasc, sl, rbase, alloc, sh);
    //agg::render_scanlines(rasc, sl, ren_sl);
        
  } // draw_mask_shape



  /// Just like draw_shapes() except that it draws an outline.
  void draw_outlines(int subshape_id, const std::vector<path> &paths,
    const std::vector<agg::path_storage>& agg_paths,
    const std::vector<line_style> &line_styles, const cxform& cx,
    const matrix& linestyle_matrix) {
    
    if (m_alpha_mask.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_outlines_impl<scanline_type> (subshape_id, paths, agg_paths, 
        line_styles, cx, linestyle_matrix, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(m_alpha_mask.back()->get_amask());
      
      draw_outlines_impl<scanline_type> (subshape_id, paths, agg_paths,
        line_styles, cx, linestyle_matrix, sl);
        
    }
    
  } //draw_outlines


  /// Template for draw_outlines(), see draw_shapes_impl().
  template <class scanline_type>
  void draw_outlines_impl(int subshape_id, const std::vector<path> &paths,
    const std::vector<agg::path_storage>& agg_paths,
    const std::vector<line_style> &line_styles, const cxform& cx, 
    const matrix& linestyle_matrix, scanline_type& sl) {
    
    assert(m_pixf.get());
    
    if (m_drawing_mask)    // Flash ignores lines in mask /definitions/
      return;    
    
    if ( _clipbounds.empty() ) return;

    // TODO: While walking the paths for filling them, remember when a path
    // has a line style associated, so that we avoid walking the paths again
    // when there really are no outlines to draw...
    
    // use avg between x and y scale
    const float stroke_scale =
      (fabsf(linestyle_matrix.get_x_scale()) + 
       fabsf(linestyle_matrix.get_y_scale())) 
      / 2.0f
      * get_stroke_scale();
    
    
    // AGG stuff
    typedef agg::rasterizer_scanline_aa<> ras_type; 
    ras_type ras;  // anti alias
    renderer_base rbase(*m_pixf);
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
      
    
    for (unsigned int cno=0; cno<_clipbounds_selected.size(); ++cno) {
    
      const geometry::Range2d<int>* bounds = _clipbounds_selected[cno];
          
      apply_clip_box<ras_type> (ras, *bounds);
      
      int current_subshape=0;

      for (size_t pno=0, pcount=paths.size(); pno<pcount; ++pno) {

        const path& this_path_gnash = paths[pno];

        agg::path_storage &this_path_agg = 
          const_cast<agg::path_storage&>(agg_paths[pno]);
        
        if (this_path_gnash.m_new_shape)
          ++current_subshape;
          
        if ((subshape_id>=0) && (current_subshape!=subshape_id)) {
          // Skip this path as it is not part of the requested sub-shape.
          continue;
        }
        
        if (this_path_gnash.m_line==0) {
          // Skip this path as it contains no line style
          continue;
        } 
        
        agg::conv_curve< agg::path_storage > curve(this_path_agg); // to render curves
        agg::conv_stroke< agg::conv_curve < agg::path_storage > > 
          stroke(curve);  // to get an outline
        
        const line_style& lstyle = line_styles[this_path_gnash.m_line-1];
          
        int thickness = lstyle.getThickness();
        if (!thickness) stroke.width(1); // hairline
        else if ( (!lstyle.scaleThicknessVertically()) && (!lstyle.scaleThicknessHorizontally()) )
        {
          stroke.width(TWIPS_TO_PIXELS(thickness));
        }
        else
        {
          if ( (!lstyle.scaleThicknessVertically()) || (!lstyle.scaleThicknessHorizontally()) )
          {
             LOG_ONCE( log_unimpl(_("Unidirectionally scaled strokes in AGG renderer (we'll scale by the scalable one)")) );
          }
          stroke.width(std::max(1.0f, thickness*stroke_scale));
        }
        
        // TODO: support endCapStyle
        
        // TODO: When lstyle.noClose==0 and the start and end point matches,
        // then render a real join instead of the caps.

        switch (lstyle.startCapStyle()) {
          case CAP_NONE   : stroke.line_cap(agg::butt_cap); break; 
          case CAP_SQUARE : stroke.line_cap(agg::square_cap); break;          
          default : case CAP_ROUND : stroke.line_cap(agg::round_cap); 
        }
        
        switch (lstyle.joinStyle()) {
          case JOIN_BEVEL : stroke.line_join(agg::bevel_join); break;
          case JOIN_MITER : stroke.line_join(agg::miter_join); break;
          default : case JOIN_ROUND : stroke.line_join(agg::round_join);
        }
        
        stroke.miter_limit(lstyle.miterLimitFactor());
                
        ras.reset();
        ras.add_path(stroke);
        
        rgba color = cx.transform(lstyle.get_color());
        ren_sl.color(agg::rgba8_pre(color.m_r, color.m_g, color.m_b, color.m_a));       
                
        agg::render_scanlines(ras, sl, ren_sl);
        
      }
    
    
    }
      
  } // draw_outlines_impl


  
  /// Draws the given polygon.
  template <class scanline_type>
  void draw_poly_impl(const point* corners, size_t corner_count, const rgba& fill, 
    const rgba& outline, scanline_type& sl, const matrix& poly_mat) {
    
    assert(m_pixf.get());

    if (corner_count<1) return;
    
    if ( _clipbounds.empty() ) return;
    
    matrix mat = stage_matrix;
    mat.concatenate(poly_mat);
    
    typedef agg::rasterizer_scanline_aa<> ras_type;
    renderer_base rbase(*m_pixf);
    ras_type ras;
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase);
      
    // -- create path --
    agg::path_storage path;
    point pnt, origin;
    
    
    // Note: The coordinates are rounded and 0.5 is added to snap them to the 
    // center of the pixel. This avoids blurring caused by anti-aliasing.
    
    // The default conversion of the boost converter is truncation.
    boost::numeric::converter<int,float> truncator;

    mat.transform(&origin, 
      point(truncator(corners[0].x), truncator(corners[0].y)));
    path.move_to(truncator(origin.x)+0.5, truncator(origin.y)+0.5);
    
    for (unsigned int i=1; i<corner_count; ++i) {
    
      mat.transform(&pnt, point(corners[i].x, corners[i].y));
        
      path.line_to(truncator(pnt.x)+0.5, truncator(pnt.y)+0.5);
    }
    
    // close polygon
    path.line_to(truncator(origin.x)+0.5, truncator(origin.y)+0.5);
    
    
    
    // -- render --
      
    // iterate through clipping bounds
    for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {
    
      const geometry::Range2d<int>& bounds = _clipbounds[cno];         
      apply_clip_box<ras_type> (ras, bounds);     
            
      
      // fill polygon
      if (fill.m_a>0) {
        ras.add_path(path);
        ren_sl.color(agg::rgba8_pre(fill.m_r, fill.m_g, fill.m_b, fill.m_a));
        
        agg::render_scanlines(ras, sl, ren_sl);
      }
      
      // draw outline
      if (outline.m_a>0) {
        agg::conv_stroke<agg::path_storage> stroke(path);
        
        stroke.width(1);
        
        ren_sl.color(agg::rgba8_pre(outline.m_r, outline.m_g, outline.m_b, outline.m_a));
        
        ras.add_path(stroke);
        
        agg::render_scanlines(ras, sl, ren_sl);
      }
    }
    
  } //draw_poly_impl
  
  
  void draw_poly(const point* corners, size_t corner_count, const rgba& fill, 
    const rgba& outline, const matrix& mat, bool masked) {
    
    if (masked && !m_alpha_mask.empty()) {
    
      // apply mask
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> sl_type; 
      
      sl_type sl(m_alpha_mask.back()->get_amask());
         
      draw_poly_impl<sl_type> (corners, corner_count, fill, outline, sl, mat);       
    
    } else {
    
      // no mask
      
      typedef agg::scanline_p8 sl_type; // packed scanline (faster for solid fills)
      
      sl_type sl;
         
      draw_poly_impl<sl_type> (corners, corner_count, fill, outline, sl, mat);
    
    }
    
  }


  inline float get_stroke_scale() {
    return (stage_matrix.get_x_scale() + stage_matrix.get_y_scale()) / 2.0f;
  }                      
  
  inline void world_to_pixel(int& x, int& y,
    float world_x, float world_y)
  {
    // negative pixels seems ok here... we don't 
    // clip to valid range, use world_to_pixel(rect&)
    // and Intersect() against valid range instead.
    point p(world_x, world_y);
    stage_matrix.transform(p);
    x = (int)p.x;
    y = (int)p.y;
  }

  geometry::Range2d<int> world_to_pixel(const rect& wb)
  {
      using namespace gnash::geometry;

    if ( wb.is_null() ) return Range2d<int>(nullRange);
    if ( wb.is_world() ) return Range2d<int>(worldRange);

    int xmin, ymin, xmax, ymax;

    world_to_pixel(xmin, ymin, wb.get_x_min(), wb.get_y_min());
    world_to_pixel(xmax, ymax, wb.get_x_max(), wb.get_y_max());

    return Range2d<int>(xmin, ymin, xmax, ymax);
  }
  
  geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& wb)
  {
    if (wb.isNull() || wb.isWorld()) return wb;
    
    int xmin, ymin, xmax, ymax;

    world_to_pixel(xmin, ymin, wb.getMinX(), wb.getMinY());
    world_to_pixel(xmax, ymax, wb.getMaxX(), wb.getMaxY());

    return geometry::Range2d<int>(xmin, ymin, xmax, ymax);
  }
  
  point 
  pixel_to_world(int x, int y)
  {
    point p(x, y);
    matrix mat = stage_matrix;
    mat.invert().transform(p);
    return p;    
  };
  
  void set_invalidated_region_world() {
    InvalidatedRanges ranges;
    ranges.setWorld();
    set_invalidated_regions(ranges);
  }
  
  virtual void set_invalidated_region(const rect& bounds) {
  
    // NOTE: Both single and multi ranges are supported by AGG renderer.
    
    InvalidatedRanges ranges;
    ranges.add(bounds.getRange());
    set_invalidated_regions(ranges);
  
  }
    
  virtual void set_invalidated_regions(const InvalidatedRanges& ranges) {
    using gnash::geometry::Range2d;
    
    int count=0;

    _clipbounds_selected.clear();
    _clipbounds.clear();    

    // TODO: cache 'visiblerect' and maintain in sync with
    //       xres/yres.
    Range2d<int> visiblerect;
    if ( xres && yres ) visiblerect = Range2d<int>(0, 0, xres-1, yres-1);
    
    for (size_t rno=0; rno<ranges.size(); ++rno) {
    
      const Range2d<float>& range = ranges.getRange(rno);

      Range2d<int> pixbounds = world_to_pixel(range);
      
      geometry::Range2d<int> bounds = Intersection(pixbounds, visiblerect);
      
      if (bounds.isNull()) continue; // out of screen
      
      assert(bounds.isFinite());
      
      _clipbounds.push_back(bounds);
      
      ++count;
    }
    //log_debug("%d inv. bounds in frame", count);
    
  }
  
  
  virtual bool bounds_in_clipping_area(const geometry::Range2d<float>& bounds) {    
    
    using gnash::geometry::Range2d;
  
    Range2d<int> pixbounds = world_to_pixel(bounds);
    
    for (unsigned int cno=0; cno<_clipbounds.size(); ++cno) {  
      if (Intersect(pixbounds, _clipbounds[cno]))
        return true;
    }
    return false;
  }

  bool getPixel(rgba& color_return, int x, int y) {
  
    if ((x<0) || (y<0) || (x>=xres) || (y>=yres))
      return false;

    agg::rgba8 color = m_pixf->pixel(x, y);    
    
    color_return.m_r = color.r;
    color_return.m_g = color.g;
    color_return.m_b = color.b;
    color_return.m_a = color.a;
    
    return true;
  }
  
  void set_scale(float new_xscale, float new_yscale) {
    
    scale_set=true;
    stage_matrix.set_identity();
    stage_matrix.set_scale(new_xscale/20.0f, new_yscale/20.0f);
  }

  void set_translation(float xoff, float yoff) {
    stage_matrix.set_translation(xoff, yoff);
  }

  virtual unsigned int getBytesPerPixel() const {
    return bpp/8;
  }  
  
private:  // private methods  

  /// Returns the cache manager instance of the given character definition.
  /// Allocates a new manager if necessary.
  agg_cache_manager* get_cache_of(character_def* def) {
  
    if (def->m_render_cache == NULL) {
      def->m_render_cache = new agg_cache_manager;
    }
    
    return def->m_render_cache;
  
  }
  
private:  // private variables

  agg::rendering_buffer m_rbuf;  

  std::auto_ptr<PixelFormat> m_pixf;
  
  /// clipping rectangle
  std::vector< geometry::Range2d<int> > _clipbounds;
  std::vector< geometry::Range2d<int>* > _clipbounds_selected;
  
  // this flag is set while a mask is drawn
  bool m_drawing_mask; 
  
  // Alpha mask stack
  std::vector< agg_alpha_mask* > m_alpha_mask;
};  // end class render_handler_agg



// detect the endianess of the host (would prefer to NOT have this function
// here)
bool is_little_endian_host() {

  union {
    boost::uint16_t word;
    struct {
      boost::uint8_t b1;
      boost::uint8_t b2;
    } s;
  } u;
    
  u.s.b1 = 1;
  u.s.b2 = 2;
  
  return u.word == 0x0201;

}


DSOEXPORT render_handler_agg_base*  create_render_handler_agg(const char *pixelformat)
{

  if (!pixelformat) return NULL;

  if (is_little_endian_host())
    log_debug(_("Framebuffer pixel format is %s (little-endian host)"), pixelformat);
  else
    log_debug(_("Framebuffer pixel format is %s (big-endian host)"), pixelformat);
  
#ifdef PIXELFORMAT_RGB555  
  if (!strcmp(pixelformat, "RGB555"))
    return new render_handler_agg<agg::pixfmt_rgb555_pre> (16); // yep, 16!
  
  else
#endif   
#ifdef PIXELFORMAT_RGB565  
  if (!strcmp(pixelformat, "RGB565") || !strcmp(pixelformat, "RGBA16"))
    return new render_handler_agg<agg::pixfmt_rgb565_pre> (16); 
  else 
#endif   
#ifdef PIXELFORMAT_RGB24  
  if (!strcmp(pixelformat, "RGB24"))
    return new render_handler_agg<agg::pixfmt_rgb24_pre> (24);    
  else 
#endif   
#ifdef PIXELFORMAT_BGR24  
  if (!strcmp(pixelformat, "BGR24"))
    return new render_handler_agg<agg::pixfmt_bgr24_pre> (24);
  else 
#endif   
#ifdef PIXELFORMAT_RGBA32 
  if (!strcmp(pixelformat, "RGBA32"))
    return new render_handler_agg<agg::pixfmt_rgba32_pre> (32);
  else 
#endif   
#ifdef PIXELFORMAT_BGRA32  
  if (!strcmp(pixelformat, "BGRA32"))
    return new render_handler_agg<agg::pixfmt_bgra32_pre> (32);
#endif   
#ifdef PIXELFORMAT_RGBA32 
  if (!strcmp(pixelformat, "ARGB32"))
    return new render_handler_agg<agg::pixfmt_argb32_pre> (32);
  else 
#endif   
#ifdef PIXELFORMAT_BGRA32  
  if (!strcmp(pixelformat, "ABGR32"))
    return new render_handler_agg<agg::pixfmt_abgr32_pre> (32);
        
  else 
#endif
  {
    log_error("Unknown pixelformat: %s\n", pixelformat);
    return NULL;
    //abort();
  }
  
  return NULL; // avoid compiler warning
}


DSOEXPORT const char *agg_detect_pixel_format(unsigned int rofs, unsigned int rsize,
  unsigned int gofs, unsigned int gsize,
  unsigned int bofs, unsigned int bsize,
  unsigned int bpp) {
  
  if (!is_little_endian_host() && (bpp>=24)) {
  
    // Swap bits for big endian hosts, because the following tests assume
    // little endians. The pixel format string matches the bytes in memory.
    
    // This applies for 24 bpp and 32 bpp modes only because AGG uses arrays
    // in the premultiply() implementation for these modes. 16 bpp modes 
    // instead use bit shifting, which is transparent to host endianess.
    // See bug #22799.
    
    rofs = bpp - rofs - rsize;
    gofs = bpp - gofs - gsize;
    bofs = bpp - bofs - bsize; 
  
  }
  
  // 15 bits RGB (hicolor)
  if ((rofs==10) && (rsize==5)
   && (gofs==5) && (gsize==5)
   && (bofs==0) && (bsize==5) ) {
   
    return "RGB555";
      
  } else   
  // 16 bits RGB (hicolor)
  if ((rofs==11) && (rsize==5)
   && (gofs==5) && (gsize==6)
   && (bofs==0) && (bsize==5) ) {
   
    return "RGB565";
      
  } else   
  
  // 24 bits RGB (truecolor)
  if ((rofs==16) && (rsize==8)
   && (gofs==8) && (gsize==8)
   && (bofs==0) && (bsize==8) ) {
   
    if (bpp==24)
      return "BGR24";
    else
      return "BGRA32";
      
  } else   
  // 24 bits BGR (truecolor)
  if ((rofs==0) && (rsize==8)
   && (gofs==8) && (gsize==8)
   && (bofs==16) && (bsize==8)) {
   
    if (bpp==24)
      return "RGB24";
    else
      return "RGBA32";
      
  } else
  // special 32 bits (mostly on big endian hosts)
  if ((rofs==8) && (rsize==8)
   && (gofs==16) && (gsize==8)
   && (bofs==24) && (bsize==8)) {
   
   return "ARGB32";
   
  } else
  // special 32 bits (mostly on big endian hosts)
  if ((rofs==24) && (rsize==8)
   && (gofs==16) && (gsize==8)
   && (bofs==8) && (bsize==8)) {
   
   return "ABGR32";
   
  }
  
  return NULL; // unknown format
  
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
/* vim: set cindent tabstop=8 softtabstop=4 shiftwidth=4: */
