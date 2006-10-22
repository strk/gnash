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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
 

/* $Id: render_handler_agg.cpp,v 1.26 2006/10/22 22:53:28 bjacques Exp $ */

// Original version by Udo Giacomozzi and Hannes Mayr, 
// INDUNET GmbH (www.indunet.it)


/// A render_handler that uses the Anti-Grain Geometry Toolkit (antigrain.com)
/// and renders directly to a buffer (for example to the framebuffer). This 
/// backend is *completely* independent of any hardware. It can be used for
/// rendering to the Linux FrameBuffer device, or be blitted inside a 
/// window (regardless of what operating system). It should also be no problem
/// to render into a file...

/*

Status:

  outlines:
    solid          COMPLETE
    patterns       NOT IMPLEMENTED (seems like Gnash does not support them yet)
    widths         COMPLETE
    colors, alpha  COMPLETE
    
  fills:
    solid fills    COMPLETE
    gradients      NOT IMPLEMENTED
    bitmaps        NOT IMPLEMENTED
    
  fonts            COMPLETE
    
  masks            NOT IMPLEMENTED (masks are drawn as shapes)
  
  caching          currently working on it...
  
  video            don't know how that works    

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
#include <agg_scanline_p.h>
#include <agg_renderer_scanline.h>
// must only include if render_scanlines_compound_layered is not defined
#if ! HAVE_AGG_SCANLINES_COMPOUND_LAYERED
#warning including compound
#include "render_handler_agg_compat.h"
#endif
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



using namespace gnash;




class bitmap_info_agg : public gnash::bitmap_info
{
public:

  bitmap_info_agg() {
    //log_msg("bitmap_info_agg instance");
    // dummy
  }

};


namespace gnash {

// --- CACHE -------------------------------------------------------------------

// Possible caching mechanisms (ideas):
//  - cache paths after applying matrix
//  - cache characters as bitmap
//  - try to update only changed parts of the stage!! This may require 
//    additional code in the character instances, something like a 
//    "invalidated" flag.
//  - smart cache: start caching after 3 or 5 hyptothetical cache hits
//    (hits to cache objects that contain no data, yet)
 

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


// --- AGG HELPER CLASSES ------------------------------------------------------

// Style handler for AGG's compound rasterizer.
class agg_style_handler
{
public:
    agg_style_handler() : 
        m_transparent(0, 0, 0, 0)
    {}

    /// Called by AGG to ask if a certain style is a solid color
    bool is_solid(unsigned style) const { 
      return true;  // The backend currently only supports solid fills 
    }
    
    /// Adds a new solid fill color style
    void add(const agg::rgba8 color) {
      m_colors.push_back(color);
    }

    /// Returns the color of a certain fill style (solid)
    const agg::rgba8& color(unsigned style) const 
    {
        if (style < m_colors.size())
            return m_colors[style];

        return m_transparent;
    }

    /// Called by AGG to generate a scanline span for non-solid fills 
    void generate_span(agg::rgba8* span, int x, int y, unsigned len, unsigned style)
    { 
      // non-solid fills currently not supported
    }


private:
    std::vector<agg::rgba8> m_colors;
    agg::rgba8          m_transparent;
};  // class agg_style_handler




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
	double scale;


public:
  int              m_view_width;      // TODO: remove these??
  int              m_view_height;

  // Enable/disable antialiasing.
  bool	m_enable_antialias;

  // Output size.
  float	m_display_width;
  float	m_display_height;

  gnash::matrix	m_current_matrix;
  gnash::cxform	m_current_cxform;

  void set_antialiased(bool /*enable*/) {
		// dummy
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
	  UNUSED(im);
	  // bitmaps currently not supported! - return dummy for fontlib
	  return new bitmap_info_agg(); 
    return NULL;
	}


  gnash::bitmap_info*	create_bitmap_info_rgba(image::rgba* im)
	// Given an image, returns a pointer to a bitmap_info class
	// that can later be passed to fill_style_bitmap(), to set a
	// bitmap fill style.
	//
	// This version takes an image with an alpha channel.
	{
	  UNUSED(im);
	  // bitmaps currently not supported! - return dummy for fontlib
	  return new bitmap_info_agg();
    return NULL;
	}


  gnash::bitmap_info*	create_bitmap_info_empty()
	// Create a placeholder bitmap_info.  Used when
	// DO_NOT_LOAD_BITMAPS is set; then later on the host program
	// can use movie_definition::get_bitmap_info_count() and
	// movie_definition::get_bitmap_info() to stuff precomputed
	// textures into these bitmap infos.
	{
	  // bitmaps currently not supported! - return dummy for fontlib
	  return new bitmap_info_agg();
    return NULL;
	}

  gnash::bitmap_info*	create_bitmap_info_alpha(int w, int h, uint8_t* data)
	// Create a bitmap_info so that it contains an alpha texture
	// with the given data (1 byte per texel).
	{
	  UNUSED(w);
    UNUSED(h);
    UNUSED(data); 
	  // bitmaps currently not supported! - return dummy for fontlib
	  return new bitmap_info_agg();
    return NULL;
	}


  void	delete_bitmap_info(gnash::bitmap_info* bi)
	// Delete the given bitmap info class.
	{
    free(bi);
	}


  // Constructor
  render_handler_agg(int bits_per_pixel)
  {
    memaddr = NULL;
    memsize = 0;
  	bpp			= bits_per_pixel;
  	m_pixf  = NULL;

  }  	

  // Destructor
  ~render_handler_agg()
  {
    if (m_pixf != NULL)
  	  delete m_pixf;    // TODO: is this correct??
  }

  /// Initializes the rendering buffer. The memory pointed by "mem" is not
  /// owned by the renderer and init_buffer() may be called multiple times
  /// when the buffer size changes, for example. However, bits_per_pixel must
  /// remain the same. 
  /// This method *must* be called prior to any other method of the class! 
  void init_buffer(unsigned char *mem, int size, int x, int y)
  {
  	memaddr = mem;
  	memsize	= size;
  	xres 		= x;
  	yres 		= y;
  	scale		= 1/20.0;
  	
  	if (m_pixf != NULL)
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
	int viewport_width, int viewport_height,
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
	  renderer_base rbase(*m_pixf);
	  rbase.clip_box(m_clip_xmin, m_clip_ymin, m_clip_xmax, m_clip_ymax);
    rbase.clear(agg::rgba8(background_color.m_r, background_color.m_g,
    	background_color.m_b, background_color.m_a));

    // calculate final pixel scale
    double scaleX, scaleY;
    scaleX = (double)xres / (double)viewport_width / 20.0;  // 20=TWIPS
    scaleY = (double)yres / (double)viewport_height / 20.0;
    scale = scaleX<scaleY ? scaleX : scaleY;
	}

  bool allow_glyph_textures() {
    // We want to render all glyphs in place 
    return false; 
  }

  void	end_display()
	// Clean up after rendering a frame.  Client program is still
	// responsible for calling glSwapBuffers() or whatever.
	{
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
  	path.move_to(pnt.m_x * scale, pnt.m_y * scale);

    for (vertex += 2;  vertex_count > 1;  vertex_count--, vertex += 2) {
      m_current_matrix.transform(&pnt, point(vertex[0], vertex[1]));
    	path.line_to(pnt.m_x * scale, pnt.m_y * scale);
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
	}

  void begin_submit_mask()
	{
    // not implemented
	}

  void end_submit_mask()
	{
    // not implemented
	}

  void disable_mask()
	{
    // not implemented
	}
	


  /*
   Takes a list of paths and combines them according to their fill style so 
   that each path is a closed polygon. Each path will have exactly one fill 
   style.
   This is necessary because Flash uses up to two fill styles for each path,
   one for each side. It can happen that a path contains only one edge. In that
   case it just divides a shape in two (two colors). Since this is impossible
   to draw directly we need to restore the original polygons so that they can
   be drawn independently.
   
   IMPORTANT NOTE: This is currently *not* used for AGG anymore, because AGG
   has a direct rasterizer for double styles...   
   */   
  void combine_paths(std::vector<path> &paths_in, std::vector<path> &paths_out) {
    
    int ino;      // index of input path
    int incount;  // cache value for paths_in.size()
    int ono;      // index of output path
    int eno;      // edge index 
    
    #define EQUAL(a,b) (fabs(a-b)<0.000000001)
    
    incount = paths_in.size();
    
    /*
    Strategy: For fill style 0, each path is compared with other paths sharing
    the same fill style and it is tried to attach the new path to a already 
    known path. The start point of the new path must match the end point of the
    previous path. Only distant shapes will not find a matching path and thus
    will create a new one.
    For fill style 1 the same is done except that the path is reversed first.
    The resulting paths will use exactly one fill style (fill style 0).
    Each source path may be used for two different resulting shapes (one time
    normally and the other reversed).    
    */
    
    // browse through all paths...
    for (ino=0; ino<incount; ino++) {
    
      path &new_path = paths_in[ino];
      int found;

      // === FILL STYLE 0 ===
      
      if (new_path.m_fill0) {
        found=0;
        
        // Search paths sharing the same fill style and whose end point matches
        // the START point of <new_path>
        for (ono=0; ono < paths_out.size(); ono++) {
        
          path &cp = paths_out[ono];
          edge &last_edge = cp.m_edges.back();
          
          log_msg("  = [FS0] compare style %d with %d: ", new_path.m_fill0,
            paths_out[ono].m_fill0);
          if (new_path.m_fill0 != paths_out[ono].m_fill0) {
            log_msg("no match\n"); 
            continue;  // fill style mismatch
          }
          log_msg("match!\n");
            
          log_msg("  = [FS0] compare edge %f/%f with %f/%f: ", 
            new_path.m_ax, new_path.m_ay,
            last_edge.m_ax, last_edge.m_ay);  
          if (!EQUAL(new_path.m_ax, last_edge.m_ax) ||
              !EQUAL(new_path.m_ay, last_edge.m_ay)) {
            log_msg("no match\n");
            continue;  // cannot attach
          }
          log_msg("match!\n");
            
            
          // ==> ok, attach "this_path" to "cp"
          
          log_msg(" == [FS0] attach path #%d\n", ino);
           
          // TODO: Resize vector and set elements directly, avoiding multiple
          // resizing...
          //cp->resize( cp->size() + new_path->size() );
          
          for (eno=0; eno<new_path.m_edges.size(); eno++) {
            cp.m_edges.push_back(new_path.m_edges[eno]);
          }
          
          found=1;
          
          break;
        
        }  // for ono
        
        if (!found) {
          // We found no matching path, so start a new one...
          log_msg(" == [FS0] start new path #%d\n", ino);
          path temp = new_path;
          temp.m_fill1=0;   // use only fill style 0
          paths_out.push_back(temp);
        }

      } // if m_fill0
      
    
      // === FILL STYLE 1 ===
      
      if (new_path.m_fill1) {
        float last_cx;
        float last_cy;
        float next_cx;
        float next_cy;
        found=0;
        
        // create a new, reversed path
        path rev_path;
        rev_path.m_fill0 = new_path.m_fill1;
        rev_path.m_ax = new_path.m_edges.back().m_ax;
        rev_path.m_ay = new_path.m_edges.back().m_ay;
        last_cx = new_path.m_edges.back().m_cx;
        last_cy = new_path.m_edges.back().m_cy;
        for (eno=new_path.m_edges.size()-2; eno>=0; eno--) {  
          edge temp = new_path.m_edges[eno];
          next_cx = temp.m_cx;
          next_cy = temp.m_cy;
          temp.m_cx = last_cx;
          temp.m_cy = last_cy;
          rev_path.m_edges.push_back(temp);
          last_cx=next_cx;
          last_cy=next_cy;
        }
        
        {
          edge temp;
          temp.m_ax = new_path.m_ax;
          temp.m_ay = new_path.m_ay;
          temp.m_cx = last_cx;
          temp.m_cy = last_cy;
          rev_path.m_edges.push_back(temp);   // add anchor of new_path as last edge
        }
        
        
        
        // ==> now proceed just as like with fill style 0...
        
        
        
        // Search paths sharing the same fill style and whose end point matches
        // the START point of <new_path>
        for (ono=0; ono < paths_out.size(); ono++) {
        
          path &cp = paths_out[ono];
          edge &last_edge = cp.m_edges.back();
          
          log_msg("  = [FS1] compare style %d with %d: ", rev_path.m_fill0,
            paths_out[ono].m_fill0);
          if (rev_path.m_fill0 != paths_out[ono].m_fill0) {
            log_msg("no match\n"); 
            continue;  // fill style mismatch
          }
          log_msg("match!\n");
            
          log_msg("  = [FS1] compare edge %f/%f with %f/%f: ", 
            rev_path.m_ax, rev_path.m_ay,
            last_edge.m_ax, last_edge.m_ay);  
          if (!EQUAL(rev_path.m_ax, last_edge.m_ax) ||
              !EQUAL(rev_path.m_ay, last_edge.m_ay)) {
            log_msg("no match\n");
            continue;  // cannot attach
          }
          log_msg("match!\n");
            
            
          // ==> ok, attach "this_path" to "cp"
          
          log_msg(" == [FS1] attach path #%d\n", ino);
           
          // TODO: Resize vector and set elements directly, avoiding multiple
          // resizing...
          //cp->resize( cp->size() + new_path->size() );
          
          for (eno=0; eno<rev_path.m_edges.size(); eno++) {
            cp.m_edges.push_back(rev_path.m_edges[eno]);
          }
          
          found=1;
          
          break;
        
        }  // for ono
        
        if (!found) {
          // We found no matching path, so start a new one...
          log_msg(" == [FS1] start new path #%d\n", ino);
          path temp = rev_path;
          temp.m_fill1=0;   // use only fill style 0
          paths_out.push_back(temp);
        }

      }

    
    }
    
    #undef EQUAL   
    
  }



  void draw_glyph(shape_character_def *def,
      const matrix& mat, rgba color, float /*pixel_scale*/) {
      
    // create a new path with the matrix applied   
    std::vector<path> paths;    
    apply_matrix_to_path(def->get_paths(), paths, mat);
      
    // make sure m_single_fill_styles contains the required color 
    need_single_fill_style(color);

    // draw the shape
    draw_shape(paths, m_single_fill_styles, m_neutral_cxform, false);
    
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
    
    draw_shape(paths, fill_styles, cx, true);
    
    draw_outlines(paths, line_styles, cx);
  }


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


  /// Draws the given path using the given fill style and color transform.
  /// Normally, Flash shapes are drawn using even-odd filling rule. However,
  /// for glyphs non-zero filling rule should be used (even_odd=0).
  void draw_shape(const std::vector<path> &paths,
    const std::vector<fill_style> &fill_styles, const cxform& cx, int even_odd) {
    
    /*
    Fortunately, AGG provides a rasterizer that fits perfectly to the flash
    data model. So we just have to feed AGG with all data and we're done. :-)
    This is also far better than recomposing the polygons as the rasterizer
    can do everything in one pass and it is also better for adjacent edges
    (anti aliasing).
    Thank to Maxim Shemanarev for providing us such a great tool with AGG...
    */
    
	  assert(m_pixf != NULL);

    // Gnash stuff 
    int pno, eno, fno;
    int pcount, ecount, fcount;
    
    // AGG stuff
    renderer_base rbase(*m_pixf);
    agg::scanline_u8 sl;                // scanline renderer
    agg::rasterizer_scanline_aa<> ras;  // anti alias
    agg::rasterizer_compound_aa<agg::rasterizer_sl_clip_dbl> rasc;  // flash-like renderer
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::span_allocator<agg::rgba8> alloc;  // span allocator (?)
    agg_style_handler sh;               // holds fill style definitions
    
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
    //log_msg("%d fill styles\n", fcount);
    for (fno=0; fno<fcount; fno++) {
      rgba color = cx.transform(fill_styles[fno].get_color());
      
      // add the color to our self-made style handler (basically just a list)
      sh.add(agg::rgba8(color.m_r, color.m_g, color.m_b, color.m_a)); 
    }
    
      
    // push paths to AGG
    pcount = paths.size();
    //log_msg("%d paths\n", pcount);
    for (pno=0; pno<pcount; pno++) {
    
      const path &this_path = paths[pno];
      agg::path_storage path;
      agg::conv_curve< agg::path_storage > curve(path);
    
      // Tell the rasterizer which styles the following path will use.
      // The good thing is, that it already supports two fill styles out of
      // the box. 
      // Flash uses value "0" for "no fill", whereas AGG uses "-1" for that. 
      rasc.styles(this_path.m_fill0-1, this_path.m_fill1-1);
      
      // starting point of path
      path.move_to(this_path.m_ax*scale, this_path.m_ay*scale);      
      
      ecount = this_path.m_edges.size();
      edge_count += ecount;
      for (eno=0; eno<ecount; eno++) {
      
        const edge &this_edge = this_path.m_edges[eno];

        if (this_edge.is_straight())
          path.line_to(this_edge.m_ax*scale, this_edge.m_ay*scale);
        else
          path.curve3(this_edge.m_cx*scale, this_edge.m_cy*scale,
                      this_edge.m_ax*scale, this_edge.m_ay*scale);
        
      }
      
      // add path to the compound rasterizer
      rasc.add_path(curve); 
    
    }
    //log_msg("%d edges\n", edge_count);
    
    // render!
    agg::render_scanlines_compound_layered(rasc, sl, rbase, alloc, sh);
    
  } // draw_shape



  /// Just like draw_shapes() except that it draws an outline.
  void draw_outlines(const std::vector<path> &paths,
    const std::vector<line_style> &line_styles, const cxform& cx) {
    
	  assert(m_pixf != NULL);

    // TODO: While walking the paths for filling them, remember when a path
    // has a line style associated, so that we avoid walking the paths again
    // when there really are no outlines to draw...
    
    // Gnash stuff    
    int pno, eno;
    int pcount, ecount;
    
    // AGG stuff
    renderer_base rbase(*m_pixf);
    agg::scanline_p8 sl;                // scanline renderer
    agg::rasterizer_scanline_aa<> ras;  // anti alias
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase); // solid fills
    agg::path_storage agg_path;             // a path in the AGG world

    ras.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);

    agg::conv_curve< agg::path_storage > curve(agg_path);    // to render curves
    agg::conv_stroke< agg::conv_curve < agg::path_storage > > 
      stroke(curve);  // to get an outline
    
    
    pcount = paths.size();   
    for (pno=0; pno<pcount; pno++) {
      
      const path &this_path = paths[pno];
      
      if (!this_path.m_line)  
        continue;     // invisible line
        
      const line_style &lstyle = line_styles[this_path.m_line-1];
      rgba color = cx.transform(lstyle.get_color());
      int width = lstyle.get_width();

      if (width==1)
        stroke.width(1);
      else
        stroke.width(width*scale);
      stroke.line_cap(agg::round_cap);
      stroke.line_join(agg::round_join);

        
      agg_path.remove_all();  // clear path
      
      agg_path.move_to(this_path.m_ax*scale, this_path.m_ay*scale);
        
      ecount = this_path.m_edges.size();
      for (eno=0; eno<ecount; eno++) {
      
        const edge &this_edge = this_path.m_edges[eno];
        
        if (this_edge.is_straight())
          agg_path.line_to(this_edge.m_ax*scale, this_edge.m_ay*scale);
        else
          agg_path.curve3(this_edge.m_cx*scale, this_edge.m_cy*scale,
                      this_edge.m_ax*scale, this_edge.m_ay*scale);
        
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

    ras.clip_box((double)m_clip_xmin, (double)m_clip_ymin, 
      (double)m_clip_xmax, (double)m_clip_ymax);
      
    agg::path_storage path;
    point pnt, origin;
    
    // Note: The coordinates are rounded and 0.5 is added to snap them to the 
    // center of the pixel. This avoids blurring caused by anti-aliasing.
    
    m_current_matrix.transform(&origin, 
      point(trunc(corners[0].m_x), trunc(corners[0].m_y)));
    path.move_to(trunc(origin.m_x*scale)+0.5, trunc(origin.m_y*scale)+0.5);
    
    for (unsigned int i=1; i<corner_count; i++) {
    
      m_current_matrix.transform(&pnt, point(corners[i].m_x, corners[i].m_y));
        
      path.line_to(trunc(pnt.m_x*scale)+0.5, trunc(pnt.m_y*scale)+0.5);
    }
    
    // close polygon
    path.line_to(trunc(origin.m_x*scale)+0.5, trunc(origin.m_y*scale)+0.5);
    
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
                      
                      
  /*
  This method is *not* being used anymore, because we use a special AGG 
  rasterizer that can deal with Flash edges directly. 
  It is kept here since it was hard work and who knows it it becomes useful
  again some day. 
  It works for 95% of the shapes. There is just a special case where it does
  not draw a curve when it should do so. I guess there is some bug in the
  combine_paths() method when reversing a path. Probably control point of the
  first or last edge of a path is wrong (gets lost) somehow.  
  */
  void draw_shape_character_old(shape_character_def *def, 
    character *inst) {
    
    // replaces: shape_character_def::display()   (both versions)
    // replaces: shape_character_def::tesselate() 

    // Gnash stuff
    std::vector<path> paths, orig_paths;
    std::vector<fill_style> fill_styles;
    path current_path;
    int paths_count;
    int pathno, edgeno, edge_count;    
    rgba color;
    int fillno;
    int fillidx;
    
    // AGG stuff
    PixelFormat pixf(m_rbuf);
    renderer_base rbase(pixf);
    agg::scanline_p8 sl;
    agg::rasterizer_scanline_aa<> ras;
    agg::renderer_scanline_aa_solid<
      agg::renderer_base<PixelFormat> > ren_sl(rbase);
     
    // We need one AGG path (polygon) for each fill style
    std::vector< agg::path_storage > agg_paths; 

    log_msg("draw_shape_character() called.\n");
    
    /*
    
    Flash will tend to save outlines in clockwise rotating order.
    We use the "non zero" filling rule of AGG, that is, when you raw a rectangle 
    in clockwise order and another rectangle inside the other with 
    counter-clockwise order, it will keep the inner rectangle transparent.
    
    AGG could also do the "even odd" filling rule where the order does not 
    matter (so we don't need to reverse fill style #1), however this leads to
    noticeable borders between polygons. Don't know exactly why, however the
    typical fill style text (Flash SDK) shows two green triangles instead of
    a green rectangle. 
    
    So, we simple draw all paths for fill style 1 in reverse order, as opposed
    to fill style 0, which is drawn normally. 
    
    */
    ras.filling_rule(agg::fill_non_zero);
    //ras.filling_rule(agg::fill_even_odd);
    
    
    //--
    
    // Combine paths, so that they are easier to draw
    orig_paths = def->get_paths();
    combine_paths(orig_paths, paths);
      
  
    
    fill_styles = def->get_fill_styles();
    
    paths_count = paths.size();     // fasten access
    
    log_msg("Fill styles vector contains %d items.\n", fill_styles.size());
    
    log_msg("Preparing paths vector...\n");
    agg_paths.resize(fill_styles.size());
    
    log_msg("Paths vector contains %d items.\n", paths_count);
    
    for (pathno=0; pathno<paths_count; pathno++) {
    
      log_msg("Processing path #%d\n", pathno);
      
      current_path = paths[pathno];
      
      log_msg("  path fill0 has index %d\n", current_path.m_fill0);
      log_msg("  path fill1 has index %d\n", current_path.m_fill1);
      
      log_msg("  path anchor at %f / %f\n", 
        current_path.m_ax/20, current_path.m_ay/20);

      edge_count = current_path.m_edges.size();      
      log_msg("  path has %d edges.\n", edge_count);
      

      //=== DRAW FILL STYLE 0 IN NORMAL ORDER ===

      fillidx = current_path.m_fill0-1;
      
      if (fillidx>=0) {
      
        agg::path_storage *the_path = &agg_paths[fillidx];
        
        log_msg("  adding path for fill style 0 (normal)\n");

        the_path->move_to(current_path.m_ax*scale, current_path.m_ay*scale);      
        
        for (edgeno=0; edgeno<edge_count; edgeno++) {
          log_msg("    edge #%d anchor %f/%f control %f/%f\n", edgeno,
            current_path.m_edges[edgeno].m_ax/20,
            current_path.m_edges[edgeno].m_ay/20,
            current_path.m_edges[edgeno].m_cx/20,
            current_path.m_edges[edgeno].m_cy/20);
            
          if (current_path.m_edges[edgeno].is_straight()) {
            the_path->line_to(current_path.m_edges[edgeno].m_ax*scale, 
              current_path.m_edges[edgeno].m_ay*scale);
          }
          else 
          {
            log_msg("    drawing curve\n");
            the_path->curve3(
              current_path.m_edges[edgeno].m_cx*scale, 
              current_path.m_edges[edgeno].m_cy*scale,
              current_path.m_edges[edgeno].m_ax*scale, 
              current_path.m_edges[edgeno].m_ay*scale);
          }
        } // for edge
        
        
      } // if fillidx



      //=== DRAW FILL STYLE 1 IN REVERSED ORDER ===

      fillidx = current_path.m_fill1-1;
      
      if (fillidx>=0) {
      
        float next_ax, next_ay, next_cx, next_cy;
        float last_ax, last_ay, last_cx, last_cy;
      
        agg::path_storage *the_path = &agg_paths[fillidx];
        
        log_msg("  adding path for fill style 1 (reversed)\n");

        /*the_path->move_to(current_path.m_edges[edge_count-1].m_ax*scale, 
                         current_path.m_edges[edge_count-1].m_ay*scale);*/
                         
        last_ax = current_path.m_ax*scale;       
        last_ay = current_path.m_ay*scale;
        last_cx = last_ax;        
        last_cy = last_ay;
        the_path->move_to(last_ax, last_ay);      
        
        for (edgeno=edge_count-1; edgeno>=0; edgeno--) {
          log_msg("    edge #%d anchor %f/%f control %f/%f\n", edgeno,
            current_path.m_edges[edgeno].m_ax/20,
            current_path.m_edges[edgeno].m_ay/20,
            current_path.m_edges[edgeno].m_cx/20,
            current_path.m_edges[edgeno].m_cy/20);
            
          next_ax = current_path.m_edges[edgeno].m_ax*scale;
          next_ay = current_path.m_edges[edgeno].m_ay*scale;
          next_cx = current_path.m_edges[edgeno].m_cx*scale;
          next_cy = current_path.m_edges[edgeno].m_cy*scale;
          
          log_msg("      the_path->curve3(%f, %f, %f, %f);\n", last_cx, last_cy, next_ax, next_ay);
          the_path->curve3(last_cx, last_cy, next_ax, next_ay);
          
          // TODO: Do not add a curve when it is a straight line
          
          last_cx = next_cx;
          last_cy = next_cy;
          
        } // for edge
        
        the_path->curve3(last_cx, last_cy, current_path.m_ax*scale, current_path.m_ay*scale);      
        // TODO: Do not add a curve when it is a straight line

      }
                              
    } // for path
    
    
    /*
    Ok, now we have prepared all the paths for all fill styles. Note that AGG
    won't render curves unless we use conv_curve. The next step is to feed the
    single paths to AGG with the right color.
    Some paths may be empty (unused fill styles).    
    */
    
    for (fillidx=0; fillidx<fill_styles.size(); fillidx++) {
    
      agg::conv_curve< agg::path_storage > curve(agg_paths[fillidx]);
    
      log_msg("  drawing fill style #%d ...\n", fillidx);     
    
      color = fill_styles[fillidx].get_color();
      
      log_msg("    color is R/G/B/A %d/%d/%d/%d\n", 
        color.m_r, color.m_g, color.m_b, color.m_a);
      
      ren_sl.color(agg::rgba8(color.m_r, color.m_g, color.m_b, color.m_a));    
          
      ras.add_path(curve);
    
      agg::render_scanlines(ras, sl, ren_sl);
      usleep(1000000);
     }
     
      
  }	// draw_shape_character_old

  
  void world_to_pixel(int *x, int *y, const float world_x, const float world_y) 
  {
    *x = (int) (world_x * scale);
    *y = (int) (world_y * scale);
  }
  
  
  virtual void set_invalidated_region(const rect bounds) {
  
    if (bounds.width() > 1e10f) {
    
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
      if (m_clip_xmax < xres-1) m_clip_xmax = xres-1;    
      if (m_clip_ymax < yres-1) m_clip_ymax = yres-1;
      
     }    
  
  }

  virtual void get_invalidated_region(int& xmin, int& ymin, int& xmax, int& ymax) {
    xmin = m_clip_xmin;
    ymin = m_clip_ymin;
    xmax = m_clip_xmax;
    ymax = m_clip_ymax;
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
