// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

 

/* $Id: render_handler_agg.cpp,v 1.47 2006/11/30 21:52:37 strk Exp $ */

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
    
  fills:
    solid fills       COMPLETE
    linear gradients  COMPLETE
    radial gradients  COMPLETE
    focal gradients   NOT IMPLEMENTED *
    bitmaps, tiled    COMPLETE
    bitmaps, clipped  COMPLETE
    bitmaps, smooth   COMPLETE
    bitmaps, hard     COMPLETE    
    color xform       COMPLETE
    
    * focal gradients (introduced in Flash 7, I think) are not yet supported 
    by Gnash itself AFAIK, but AGG supports them and it should be easy to add 
    them.    
    
  fonts               COMPLETE
    
  masks               COMPLETE
  
  caching             NONE IMPLEMENTED
  
  video               NOT IMPLEMENTED, only stubs
  
  Currently the renderer should be able to render everything correctly,
  except videos.
  
  
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
#include "config.h"
#endif


#include "gnash.h"
#include "types.h"
#include "image.h"
#include "utility.h"
#include "log.h"
#include "render_handler.h"
#include "render_handler_agg.h" 

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

#ifndef trunc
#define trunc(a) static_cast<long>(a)
#endif

using namespace gnash;




namespace gnash {

// --- CACHE -------------------------------------------------------------------
/// This class holds a completely transformed path (fixed position). Speeds
/// up characters that stay fixed on a certain position on the stage. 
// ***CURRENTLY***NOT***USED***

class agg_transformed_path 
{
  /// Original transformation matrix 
  matrix m_mat;  
  
  /// Number of cache hits 
  int hits;
  
  /// Number of cache misses
  int misses;
  
  /// Contents of this cache item. First dimension is fill style 
  std::vector <std::vector <agg::path_storage> > data;
};

class agg_cache_manager : private render_cache_manager
{
};



// --- YUV VIDEO ---------------------------------------------------------------
// Currently not implemented.

class agg_YUV_video : public gnash::YUV_video
{
public:

  agg_YUV_video(int width, int height): YUV_video(width, height)
  {
    log_msg("warning: YUV_video not supported by AGG renderer");
  }
  
  ~agg_YUV_video() {
  }
  
}; // class agg_YUV_video



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
  
    m_buffer = new uint8_t[width*height];
    
    m_rbuf.attach(m_buffer, width, height, width);
    
    // NOTE: The buffer is *not* cleared. The clear() function must be called
    // to clear the buffer (alpha=0). The reason is to avoid clearing the 
    // whole mask when only a small portion is really used.

  }
  
  ~agg_alpha_mask() 
  {
    delete [] m_buffer;
  }
  
  void clear(unsigned int left, unsigned int top, unsigned int width, 
    unsigned int height) {
    
	  if (!width) return;
	  
	  unsigned int y;
	  const unsigned int max_y = top+height; // to be exact, it's one off the max.
	  const agg::gray8 black(0);
	  	  
    for (y=top; y<max_y; y++) 
      m_pixf.copy_hline(left, y, width, black);
  }
  
  renderer_base& get_rbase() {
    return m_rbase;
  }
  
  amask_type& get_amask() {
    return m_amask;
  }  
  
  
private:
  // in-memory buffer
  uint8_t* m_buffer;
  
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
    
  // TODO: Change these!!
	unsigned char *memaddr;
	int	memsize;
	int xres;
	int yres;
	int bpp; 	// bits per pixel
	double xscale, yscale;


public:
  //int              m_view_width;      // TODO: remove these??
  //int              m_view_height;

  // Enable/disable antialiasing.
  bool	m_enable_antialias;

  // Output size.
  float	m_display_width;
  float	m_display_height;

  gnash::matrix	m_current_matrix;
  gnash::cxform	m_current_cxform;

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


  gnash::bitmap_info*	create_bitmap_info_rgb(image::rgb* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_styleX_bitmap(), to set a
	// bitmap fill style.
	{	   
	  return new agg_bitmap_info<agg::pixfmt_rgb24> (im->m_width, im->m_height,
      im->m_pitch, im->m_data, 24);
    assert(0); 
	}


  gnash::bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
	  return new agg_bitmap_info<agg::pixfmt_rgba32> (im->m_width, im->m_height,
      im->m_pitch, im->m_data, 32); 
	}


  gnash::bitmap_info*	create_bitmap_info_empty()
	// Create a placeholder bitmap_info.  Used when
	// DO_NOT_LOAD_BITMAPS is set; then later on the host program
	// can use movie_definition::get_bitmap_info_count() and
	// movie_definition::get_bitmap_info() to stuff precomputed
	// textures into these bitmap infos.
	{
	  // bitmaps currently not supported! - return dummy for fontlib
	  unsigned char dummy=0;
	  return new agg_bitmap_info<agg::pixfmt_rgb24> (0, 0, 0, &dummy, 24);
	}

  gnash::bitmap_info*	create_bitmap_info_alpha(int /*w*/, int /*h*/, uint8_t* /*data*/)
	// Create a bitmap_info so that it contains an alpha texture
	// with the given data (1 byte per texel).
	{
	  //return new agg_bitmap_info<agg::pixfmt_gray8> (w, h, w, data, 8);

	  // where is this used, anyway??
	  log_msg("create_bitmap_info_alpha() currently not supported");
	  
	  return new bitmap_info();
	}


  void	delete_bitmap_info(gnash::bitmap_info* bi)
	// Delete the given bitmap info class.
	{
    free(bi);
	}
	
	
	gnash::YUV_video*	create_YUV_video(int w, int h)
	{	  
	  return new agg_YUV_video(w, h);
  }
  
  void	delete_YUV_video(gnash::YUV_video* yuv)
	{
	  // don't need to pointer != null before deletion
	  //if (yuv)
	      delete yuv;
	}


  // Constructor
  render_handler_agg(int bits_per_pixel)
      :
      // Initialization list
      memaddr(NULL),
      memsize(0),
      xres(1),
      yres(1),
      bpp(bits_per_pixel),
      xscale(1.0),
      yscale(1.0),
      m_enable_antialias(true),
      m_pixf(NULL)
  {
  }  	

  // Destructor
  ~render_handler_agg()
  {
    // don't need to check m_pixf != NULL
    // as that check is already implemented
    // in the 'delete' statement
    // if (m_pixf != NULL)
  	  delete m_pixf;    // TODO: is this correct??
  }

  /// Initializes the rendering buffer. The memory pointed by "mem" is not
  /// owned by the renderer and init_buffer() may be called multiple times
  /// when the buffer size changes, for example. However, bits_per_pixel must
  /// remain the same. 
  /// This method *must* be called prior to any other method of the class! 
  void init_buffer(unsigned char *mem, int size, int x, int y)
  {
        assert(x > 0);
        assert(y > 0);

  	memaddr = mem;
  	memsize	= size;
  	xres 		= x;
  	yres 		= y;
  	
        // don't need to check m_pixf != NULL
        // as that check is already implemented
        // in the 'delete' statement
  	//if (m_pixf != NULL)
  	  delete m_pixf;    // TODO: is this correct??

    int row_size = xres*((bpp+7)/8);
    m_rbuf.attach(memaddr, xres, yres, row_size);

    // allocate pixel format accessor  	
    m_pixf = new PixelFormat(m_rbuf);
    //m_rbase = new renderer_base(*m_pixf);  --> does not work!!??
    
    m_clip_xmin = 0;
    m_clip_ymin = 0;
    m_clip_xmax = xres-1;
    m_clip_ymax = yres-1;
        
    log_msg("initialized AGG buffer <%p>, %d bytes, %dx%d, rowsize is %d bytes", 
      mem, size, x, y, row_size);
  }
  

  void begin_display(
	gnash::rgba background_color,
	int /*viewport_x0*/, int /*viewport_y0*/,
	int /*viewport_width*/, int /*viewport_height*/,
	float /*x0*/, float /*x1*/, float /*y0*/, float /*y1*/)
	// Set up to render a full frame from a movie and fills the
	// background.	Sets up necessary transforms, to scale the
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
	  assert(m_pixf != NULL);

	  // clear the stage using the background color    
    clear_framebuffer(m_clip_xmin, m_clip_ymin, 
      m_clip_xmax-m_clip_xmin+1, m_clip_ymax-m_clip_ymin+1, 
      agg::rgba8(background_color.m_r, background_color.m_g, 
      background_color.m_b, background_color.m_a));
    	  
    // calculate final pixel scale
    /*double scaleX, scaleY;
    scaleX = (double)xres / (double)viewport_width / 20.0;  // 20=TWIPS
    scaleY = (double)yres / (double)viewport_height / 20.0;
    scale = scaleX<scaleY ? scaleX : scaleY;*/
    
    // reset status variables
    m_drawing_mask = false;
	}
	
	/// renderer_base.clear() does no clipping which clears the whole framebuffer
	/// even if we update just a small portion of the screen. The result would be
	/// still correct, but slower. 
  /// This function clears only a certain portion of the screen, while /not/ 
  /// being notably slower for a fullscreen clear. 
	void clear_framebuffer(int left, int top, int width, int height, agg::rgba8 color) {
    
	  if (width<1) {
	    log_msg("warning: clear_framebuffer() called with width=%d", width);
      return;
    }
    
	  if (height<1) {
	    log_msg("warning: clear_framebuffer() called with height=%d", height);
      return;
    }
	  
	  unsigned int y;
	  const unsigned int max_y = top+height; // to be exact, it's one off the max.
	  	  
    for (y=top; y<max_y; y++) 
      m_pixf->copy_hline(left, y, width, color);
  }

  bool allow_glyph_textures() {
    // We want to render all glyphs in place 
    return false; 
  }

  void	end_display()
	// Clean up after rendering a frame.  Client program is still
	// responsible for calling glSwapBuffers() or whatever.
	{
	
	  if (m_drawing_mask) 
	    log_msg("warning: rendering ended while drawing a mask");
	    
	  while (! m_alpha_mask.empty()) {
	    log_msg("warning: rendering ended while masks were still active");
      disable_mask();	     
	  }
	
    // nothing to do
	}

  void	set_matrix(const gnash::matrix& m)
	// Set the current transform for mesh & line-strip rendering.
	{
    // used only for drawing line strips...	   
	  m_current_matrix = m;
	}

  void	set_cxform(const gnash::cxform& cx)
	// Set the current color transform for mesh & line-strip rendering.
	{
    // used only for drawing line strips...	   
    m_current_cxform = cx;
	}

  static void	apply_matrix(const gnash::matrix& /*m*/)
	// add user space transformation
	{
    // TODO: what's the use for this, anyway?? 
    log_msg("apply_matrix(); called - NOT IMPLEMENTED");
	}

  static void	apply_color(const gnash::rgba& /*c*/)
	// Set the given color.
	{
    // TODO: what's the use for this, anyway?? 
    log_msg("apply_color(); called - NOT IMPLEMENTED");
	}



  void	draw_line_strip(const void* coords, int vertex_count, const rgba color)
	// Draw the line strip formed by the sequence of points.
	{
	  assert(m_pixf != NULL);

    point pnt;
    
    renderer_base rbase(*m_pixf);

  	agg::scanline_p8 sl;
  	agg::rasterizer_scanline_aa<> ras;
  	agg::renderer_scanline_aa_solid<
    	agg::renderer_base<PixelFormat> > ren_sl(rbase);
    	
    ras.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);    	

    agg::path_storage path;
    agg::conv_stroke<agg::path_storage> stroke(path);
    stroke.width(1);
    stroke.line_cap(agg::round_cap);
    stroke.line_join(agg::round_join);
    path.remove_all(); // Not obligatory in this case

    const int16_t *vertex = static_cast<const int16_t*>(coords);
    
    m_current_matrix.transform(&pnt, point(vertex[0], vertex[1]));
  	path.move_to(pnt.m_x * xscale, pnt.m_y * yscale);

    for (vertex += 2;  vertex_count > 1;  vertex_count--, vertex += 2) {
      m_current_matrix.transform(&pnt, point(vertex[0], vertex[1]));
    	path.line_to(pnt.m_x * xscale, pnt.m_y * yscale);
    }
		// The vectorial pipeline
  	ras.add_path(stroke);

  	// Set the color and render the scanlines
  	ren_sl.color(agg::rgba8(color.m_r, color.m_g, color.m_b, color.m_a));
  	agg::render_scanlines(ras, sl, ren_sl);

	} // draw_line_strip


  void	draw_bitmap(
	const gnash::matrix& /*m*/,
	const gnash::bitmap_info* /*bi*/,
	const gnash::rect& /*coords*/,
	const gnash::rect& /*uv_coords*/,
	gnash::rgba /*color*/)
	// Draw a rectangle textured with the given bitmap, with the
	// given color.	 Apply given transform; ignore any currently
	// set transforms.
	//
	// Intended for textured glyph rendering.
	{
    log_msg("  draw_bitmap NOT IMPLEMENTED\n");
    // could be implemented, but is not used
	}

  void begin_submit_mask()
	{
	  // Set flag so that rendering of shapes is simplified (only solid fill) 
    m_drawing_mask = true;
    
    agg_alpha_mask* new_mask = new agg_alpha_mask(xres, yres);
    
    // TODO: implement a testInvariant() function for these
    assert(m_clip_xmin <= m_clip_xmax);
    assert(m_clip_ymin <= m_clip_ymax);

    new_mask->clear(m_clip_xmin, m_clip_ymin, 
      m_clip_xmax-m_clip_xmin+1, m_clip_ymax-m_clip_ymin+1); 
    
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
      const matrix& mat, rgba color, float /*pixel_scale*/) {
      
    // NOTE: def->get_bound() is NULL for glyphs so we can't check the 
    // clipping area (bounds_in_clipping_area):
      
    // create a new path with the matrix applied   
    std::vector<path> paths;    
    apply_matrix_to_path(def->get_paths(), paths, mat);
      
    // make sure m_single_fill_styles contains the required color 
    need_single_fill_style(color);

    // draw the shape
    if (m_drawing_mask)
	draw_mask_shape(paths, false);
    else
	draw_shape(-1, paths, m_single_fill_styles, m_neutral_cxform,  
	    mat, false);
    
    // NOTE: Do not use even-odd filling rule for glyphs!
  }


  void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    float /*pixel_scale*/,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles) {

    std::vector<path> paths;
    
    apply_matrix_to_path(def->get_paths(), paths, mat);

    if (m_drawing_mask) {
      
      // Shape is drawn inside a mask, skip sub-shapes handling and outlines
      draw_mask_shape(paths, true);      
    
    } else {
      
      // We need to separate sub-shapes during rendering. The current 
      // implementation is a bit sub-optimal because the fill styles get
      // re-initialized for each sub-shape. Maybe this will be no more a problem
      // once fill styles get cached, anyway.     
      const int subshape_count=count_sub_shapes(paths);
      
      for (int subshape=0; subshape<subshape_count; subshape++) {
        draw_shape(subshape, paths, fill_styles, cx, mat, true);    
        draw_outlines(subshape, paths, line_styles, cx, mat);
      }
    } // if not drawing mask
    
  } // draw_shape_character


  /// Takes a path and translates it using the given matrix. The new path
  /// is stored in paths_out.  
  void apply_matrix_to_path(const std::vector<path> &paths_in, 
    std::vector<path> &paths_out, const matrix &mat) {
    
    int pcount, ecount;
    int pno, eno;
    
    // copy path
    paths_out = paths_in;    
    pcount = paths_out.size();
        
    
    for (pno=0; pno<pcount; pno++) {
    
      path &the_path = paths_out[pno];     
      point oldpnt(the_path.m_ax, the_path.m_ay);
      point newpnt;
       
      mat.transform(&newpnt, oldpnt);
      the_path.m_ax = newpnt.m_x;    
      the_path.m_ay = newpnt.m_y;
      
      ecount = the_path.m_edges.size();
      for (eno=0; eno<ecount; eno++) {
      
        edge &the_edge = the_path.m_edges[eno];
        
        oldpnt.m_x = the_edge.m_ax;
        oldpnt.m_y = the_edge.m_ay;
        mat.transform(&newpnt, oldpnt);
        the_edge.m_ax = newpnt.m_x;
        the_edge.m_ay = newpnt.m_y;
        
        oldpnt.m_x = the_edge.m_cx;
        oldpnt.m_y = the_edge.m_cy;
        mat.transform(&newpnt, oldpnt);
        the_edge.m_cx = newpnt.m_x;
        the_edge.m_cy = newpnt.m_y;
      
      }          
      
    } 
    
  } // apply_matrix



  /// A shape can have sub-shapes. This can happen when there are multiple
  /// layers of the same frame count. Flash combines them to one single shape.
  /// The problem with sub-shapes is, that outlines can be hidden by other
  /// layers so they must be rendered separately. 
  unsigned int count_sub_shapes(const std::vector<path> &paths) {
  
    int sscount=1;
    
    int pcount = paths.size();
    
    for (int pno=0; pno<pcount; pno++) {    // skip first path!
      const path &this_path = paths[pno];
      
      // Udo said we could comment this out 
      // https://savannah.gnu.org/bugs/?18119#comment2
      //if (pno==0) 
      //  assert(!this_path.m_new_shape); // this would break draw_XXX
      
      if (this_path.m_new_shape)
        sscount++;
    }
    
    return sscount;
  }
  

  /// Draws the given path using the given fill style and color transform.
  /// Normally, Flash shapes are drawn using even-odd filling rule. However,
  /// for glyphs non-zero filling rule should be used (even_odd=0).
  /// Note the paths have already been transformed by the matrix and 
  /// 'fillstyle_matrix' is only provided for bitmap transformations.
  /// 'subshape_id' defines which sub-shape should be drawn (-1 means all 
  /// subshapes)  
  void draw_shape(int subshape_id, const std::vector<path> &paths,
    const std::vector<fill_style> &fill_styles, const cxform& cx,
    const matrix& fillstyle_matrix, int even_odd) {
    
    if (m_alpha_mask.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_shape_impl<scanline_type> (subshape_id, paths, fill_styles, cx, 
        fillstyle_matrix, even_odd, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(m_alpha_mask.back()->get_amask());
      
      draw_shape_impl<scanline_type> (subshape_id, paths, fill_styles, cx, 
        fillstyle_matrix, even_odd, sl);
        
    }
    
  }
   
  /// Template for draw_shape(). Two different scanline types are suppored, 
  /// one with and one without an alpha mask. This makes drawing without masks
  /// much faster.  
  template <class scanline_type>
  void draw_shape_impl(int subshape_id, const std::vector<path> &paths,
    const std::vector<fill_style> &fill_styles, const cxform& cx,
    const matrix& fillstyle_matrix, int even_odd, scanline_type& sl) {
    /*
    Fortunately, AGG provides a rasterizer that fits perfectly to the flash
    data model. So we just have to feed AGG with all data and we're done. :-)
    This is also far better than recomposing the polygons as the rasterizer
    can do everything in one pass and it is also better for adjacent edges
    (anti aliasing).
    Thank to Maxim Shemanarev for providing us such a great tool with AGG...
    */
    
	  assert(m_pixf != NULL);
	  
	  assert(!m_drawing_mask);

    // Gnash stuff 
    int pno, eno, fno;
    int pcount, ecount, fcount;
    
    // AGG stuff
    renderer_base rbase(*m_pixf);
    agg::rasterizer_scanline_aa<> ras;  // anti alias
    agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;  // flash-like renderer
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::span_allocator<agg::rgba8> alloc;  // span allocator (?)
    agg_style_handler sh;               // holds fill style definitions
    
    // TODO: implement a testInvariant() function for these
    assert(m_clip_xmin <= m_clip_xmax);
    assert(m_clip_ymin <= m_clip_ymax);

    rasc.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);
    
    // debug
    int edge_count=0;
    
    // activate even-odd filling rule
    if (even_odd)
      rasc.filling_rule(agg::fill_even_odd);
    else
      rasc.filling_rule(agg::fill_non_zero);
  
      
    // tell AGG what styles are used
    fcount = fill_styles.size();
    for (fno=0; fno<fcount; fno++) {
    
      bool smooth=false;
      int fill_type = fill_styles[fno].get_type();
      
      switch (fill_type) {

        case SWF::FILL_LINEAR_GRADIENT:
        {    
          matrix m = fill_styles[fno].get_gradient_matrix();
          matrix cm;
          cm.set_inverse(fillstyle_matrix);
          m.concatenate(cm);
          m.concatenate_scales(1.0f/xscale, 1.0f/yscale);
          
          sh.add_gradient_linear(fill_styles[fno], m, cx);
          break;
        } 

        case SWF::FILL_RADIAL_GRADIENT:
        {
          matrix m = fill_styles[fno].get_gradient_matrix();
          matrix cm;
          cm.set_inverse(fillstyle_matrix);
          m.concatenate(cm);
          m.concatenate_scales(1.0f/xscale, 1.0f/yscale);
          
          sh.add_gradient_radial(fill_styles[fno], m, cx);
          break;
        } 

        case SWF::FILL_TILED_BITMAP:
        case SWF::FILL_CLIPPED_BITMAP:
        smooth=true;  // continue with next case!
        
        case SWF::FILL_TILED_BITMAP_HARD:
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        {    
          matrix m = fill_styles[fno].get_bitmap_matrix();
          matrix cm;
          cm.set_inverse(fillstyle_matrix);
          m.concatenate(cm);
          m.concatenate_scales(1.0f/xscale, 1.0f/yscale);
          
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
          sh.add_color(agg::rgba8(color.m_r, color.m_g, color.m_b, color.m_a));
        } 
        
      } // switch
        
    } // for
    
      
    // push paths to AGG
    pcount = paths.size();
    int current_subshape = 0; 

    for (pno=0; pno<pcount; pno++) {
    
      const path &this_path = paths[pno];
      agg::path_storage path;
      agg::conv_curve< agg::path_storage > curve(path);
      
      if (this_path.m_new_shape) 
        current_subshape++;
        
      if ((subshape_id>=0) && (current_subshape!=subshape_id)) {
        // Skip this path as it is not part of the requested sub-shape.
        continue;
      }
        
      // Tell the rasterizer which styles the following path will use.
      // The good thing is, that it already supports two fill styles out of
      // the box. 
      // Flash uses value "0" for "no fill", whereas AGG uses "-1" for that. 
      rasc.styles(this_path.m_fill0-1, this_path.m_fill1-1);
      
      // starting point of path
      path.move_to(this_path.m_ax*xscale, this_path.m_ay*yscale);      
      
      ecount = this_path.m_edges.size();
      edge_count += ecount;
      for (eno=0; eno<ecount; eno++) {
      
        const edge &this_edge = this_path.m_edges[eno];

        if (this_edge.is_straight())
          path.line_to(this_edge.m_ax*xscale, this_edge.m_ay*yscale);
        else
          path.curve3(this_edge.m_cx*xscale, this_edge.m_cy*yscale,
                      this_edge.m_ax*xscale, this_edge.m_ay*yscale);
        
      }
      
      // add path to the compound rasterizer
      rasc.add_path(curve); 
    
    }
    //log_msg("%d edges\n", edge_count);
    
    // render!
    agg::render_scanlines_compound_layered(rasc, sl, rbase, alloc, sh);
    
  } // draw_shape




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
       
    // anti-aliased scanline rasterizer
    typedef agg::rasterizer_scanline_aa<> ras_type;
    ras_type ras;
    
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
    unsigned int pcount = paths.size();
    for (unsigned int pno=0; pno < pcount; pno++) {
    
      const path& this_path = paths[pno];
      agg::path_storage path;
      agg::conv_curve< agg::path_storage > curve(path);
      
      // reduce everything to just one fill style!
      rasc.styles(this_path.m_fill0==0 ? -1 : 0,
                  this_path.m_fill1==0 ? -1 : 0);
                  
      // starting point of path
      path.move_to(this_path.m_ax*xscale, this_path.m_ay*yscale);
    
      unsigned int ecount = this_path.m_edges.size();
      for (unsigned int eno=0; eno<ecount; eno++) {
      
        const edge &this_edge = this_path.m_edges[eno];

        if (this_edge.is_straight())
          path.line_to(this_edge.m_ax*xscale, this_edge.m_ay*yscale);
        else
          path.curve3(this_edge.m_cx*xscale, this_edge.m_cy*yscale,
                      this_edge.m_ax*xscale, this_edge.m_ay*yscale);
        
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
    const std::vector<line_style> &line_styles, const cxform& cx,
    const matrix& linestyle_matrix) {
    
    if (m_alpha_mask.empty()) {
    
      // No mask active, use normal scanline renderer
      
      typedef agg::scanline_u8 scanline_type;
      
      scanline_type sl;
      
      draw_outlines_impl<scanline_type> (subshape_id, paths, line_styles, 
        cx, linestyle_matrix, sl);
        
    } else {
    
      // Mask is active, use alpha mask scanline renderer
      
      typedef agg::scanline_u8_am<agg::alpha_mask_gray8> scanline_type;
      
      scanline_type sl(m_alpha_mask.back()->get_amask());
      
      draw_outlines_impl<scanline_type> (subshape_id, paths, line_styles, 
        cx, linestyle_matrix, sl);
        
    }
    
  }


  /// Template for draw_outlines(), see draw_shapes_impl().
  template <class scanline_type>
  void draw_outlines_impl(int subshape_id, const std::vector<path> &paths,
    const std::vector<line_style> &line_styles, const cxform& cx, 
    const matrix& linestyle_matrix, scanline_type& sl) {
    
	  assert(m_pixf != NULL);
	  
	  if (m_drawing_mask)    // Flash ignores lines in mask /definitions/
      return;    

    // TODO: While walking the paths for filling them, remember when a path
    // has a line style associated, so that we avoid walking the paths again
    // when there really are no outlines to draw...
    
    // Gnash stuff    
    int pno, eno;
    int pcount, ecount;
    
    // use avg between x and y scale
    const float stroke_scale = 
      (linestyle_matrix.get_x_scale() + linestyle_matrix.get_y_scale()) / 2.0f
      * (xscale+yscale)/2.0f;
    
    
    // AGG stuff
    renderer_base rbase(*m_pixf);
    agg::rasterizer_scanline_aa<> ras;  // anti alias
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::path_storage agg_path;             // a path in the AGG world

    // TODO: implement a testInvariant() function for these
    assert(m_clip_xmin <= m_clip_xmax);
    assert(m_clip_ymin <= m_clip_ymax);

    ras.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);

    agg::conv_curve< agg::path_storage > curve(agg_path);    // to render curves
    agg::conv_stroke< agg::conv_curve < agg::path_storage > > 
      stroke(curve);  // to get an outline
    
    
    int current_subshape = 0; 
    pcount = paths.size();   
    for (pno=0; pno<pcount; pno++) {
      
      const path &this_path = paths[pno];
      
      if (this_path.m_new_shape)
        current_subshape++;
        
      if ((subshape_id>=0) && (current_subshape!=subshape_id)) {
        // Skip this path as it is not part of the requested sub-shape.
        continue;
      }
      
      if (!this_path.m_line)  
        continue;     // invisible line
               
        
      const line_style &lstyle = line_styles[this_path.m_line-1];
      rgba color = cx.transform(lstyle.get_color());
      int width = lstyle.get_width();
      if (width==1)
        stroke.width(1);
      else
        stroke.width(width*stroke_scale);
      stroke.line_cap(agg::round_cap);
      stroke.line_join(agg::round_join);

        
      agg_path.remove_all();  // clear path
      
      agg_path.move_to(this_path.m_ax*xscale, this_path.m_ay*yscale);
        
      ecount = this_path.m_edges.size();
      for (eno=0; eno<ecount; eno++) {
      
        const edge &this_edge = this_path.m_edges[eno];
        
        if (this_edge.is_straight())
          agg_path.line_to(this_edge.m_ax*xscale, this_edge.m_ay*yscale);
        else
          agg_path.curve3(this_edge.m_cx*yscale, this_edge.m_cy*yscale,
                      this_edge.m_ax*yscale, this_edge.m_ay*yscale);
        
      } // for edges
      
      
      ras.add_path(stroke);
      ren_sl.color(agg::rgba8(color.m_r, color.m_g, color.m_b, color.m_a));
      
      agg::render_scanlines(ras, sl, ren_sl);
    
    
    }
      
  } // draw_outlines


  
  /// Draws the given polygon.
  void  draw_poly(const point* corners, size_t corner_count, const rgba fill, 
    const rgba outline) {
    
	  assert(m_pixf != NULL);

    if (corner_count<1) return;
    
    // TODO: Use aliased scanline renderer instead of anti-aliased one since
    // it is undesired anyway.
    renderer_base rbase(*m_pixf);
    agg::scanline_p8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase);

    // TODO: implement a testInvariant() function for these
    assert(m_clip_xmin <= m_clip_xmax);
    assert(m_clip_ymin <= m_clip_ymax);

    ras.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);
      
    agg::path_storage path;
    point pnt, origin;
    
    // Note: The coordinates are rounded and 0.5 is added to snap them to the 
    // center of the pixel. This avoids blurring caused by anti-aliasing.
    
    m_current_matrix.transform(&origin, 
      point(trunc(corners[0].m_x), trunc(corners[0].m_y)));
    path.move_to(trunc(origin.m_x*xscale)+0.5, trunc(origin.m_y*yscale)+0.5);
    
    for (unsigned int i=1; i<corner_count; i++) {
    
      m_current_matrix.transform(&pnt, point(corners[i].m_x, corners[i].m_y));
        
      path.line_to(trunc(pnt.m_x*xscale)+0.5, trunc(pnt.m_y*yscale)+0.5);
    }
    
    // close polygon
    path.line_to(trunc(origin.m_x*xscale)+0.5, trunc(origin.m_y*yscale)+0.5);
    
    // fill polygon
    if (fill.m_a>0) {
      ras.add_path(path);
      ren_sl.color(agg::rgba8(fill.m_r, fill.m_g, fill.m_b, fill.m_a));
      agg::render_scanlines(ras, sl, ren_sl);
    }
    
    // draw outline
    if (outline.m_a>0) {
      agg::conv_stroke<agg::path_storage> stroke(path);
      
      stroke.width(1);
      
      ren_sl.color(agg::rgba8(outline.m_r, outline.m_g, outline.m_b, outline.m_a));
      
      ras.add_path(stroke);
      agg::render_scanlines(ras, sl, ren_sl);
    }
    
  }
                      
  
  void world_to_pixel(int *x, int *y, const float world_x, const float world_y) 
  {
    *x = (int) (world_x * xscale);
    *y = (int) (world_y * yscale);
  }
  
  
  virtual void set_invalidated_region(const rect bounds) {
  
    // we really support such big numbers ?
    // better reduce the limit check, to make sure...
    if (bounds.width() > 1e9f) {
    
      // Region is entire rendering buffer. Don't convert to integer as 
      // this will overflow.
      m_clip_xmin = 0;
      m_clip_ymin = 0;
      m_clip_xmax = xres-1;
      m_clip_ymax = yres-1;
      
    } else {
    
      world_to_pixel(&m_clip_xmin, &m_clip_ymin, bounds.get_x_min(), bounds.get_y_min());
      world_to_pixel(&m_clip_xmax, &m_clip_ymax, bounds.get_x_max(), bounds.get_y_max());
      
      // add 2 pixels (GUI does that too)
      m_clip_xmin -= 2;
      m_clip_ymin -= 2;
      m_clip_xmax += 2;
      m_clip_ymax += 2;
  
      if (m_clip_xmin < 0) m_clip_xmin=0;    
      if (m_clip_ymin < 0) m_clip_ymin=0;    
      if (m_clip_xmax > xres-1) m_clip_xmax = xres-1;    
      if (m_clip_ymax > yres-1) m_clip_ymax = yres-1;
      
     }    
  
  }
  
  virtual bool bounds_in_clipping_area(const rect& bounds) {    
    int bxmin, bxmax, bymin, bymax;
    
    if (bounds.is_null()) return false;
    
    world_to_pixel(&bxmin, &bymin, bounds.get_x_min(), bounds.get_y_min()); 
    world_to_pixel(&bxmax, &bymax, bounds.get_x_max(), bounds.get_y_max());
    
    return
      (bxmin <= m_clip_xmax) &&
      (bxmin <= m_clip_xmax) &&
      (bymax >= m_clip_ymin) && 
      (bymax >= m_clip_ymin); 
  }

  void get_pixel(rgba& color_return, float world_x, float world_y) {
    int x, y;
    
    world_to_pixel(&x, &y, world_x, world_y);

    agg::rgba8 color = m_pixf->pixel(x, y);    
    
    color_return.m_r = color.r;
    color_return.m_g = color.g;
    color_return.m_b = color.b;
    color_return.m_a = color.a;
    
  }
  
  void set_scale(float new_xscale, float new_yscale) {
    xscale = new_xscale/20.0f;
    yscale = new_yscale/20.0f;
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
  PixelFormat *m_pixf;
  
  // clipping rectangle
  int m_clip_xmin;
  int m_clip_ymin;
  int m_clip_xmax;
  int m_clip_ymax;
  
  // this flag is set while a mask is drawn
  bool m_drawing_mask; 
  
  // Alpha mask stack
  std::vector< agg_alpha_mask* > m_alpha_mask;
};	// end class render_handler_agg




// TODO: Replace "pixelformat" with a enum!

DSOEXPORT render_handler_agg_base*	create_render_handler_agg(char *pixelformat)
{

  log_msg("framebuffer pixel format is %s", pixelformat);
  
  if (!strcmp(pixelformat, "RGB555"))
	  return new render_handler_agg<agg::pixfmt_rgb555> (16); // yep, 16!
	
	else if (!strcmp(pixelformat, "RGB565") || !strcmp(pixelformat, "RGBA16"))
	  return new render_handler_agg<agg::pixfmt_rgb565> (16);
	
	else if (!strcmp(pixelformat, "RGB24"))
	  return new render_handler_agg<agg::pixfmt_rgb24> (24);
		
	else if (!strcmp(pixelformat, "BGR24"))
	  return new render_handler_agg<agg::pixfmt_bgr24> (24);

	else if (!strcmp(pixelformat, "RGBA32"))
	  return new render_handler_agg<agg::pixfmt_rgba32> (32);
	  	  
	else {
		log_error("Unknown pixelformat: %s\n", pixelformat);
		assert(0);
	}
	
	return NULL; // avoid compiler warning
}

} // end of namespace gnash


// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
/* vim: set cindent tabstop=8 softtabstop=4 shiftwidth=4: */
