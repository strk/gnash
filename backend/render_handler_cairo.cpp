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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

// Known bugs:
// - Shape filling problems: renderer skips certain fills. See car_smash.swf.
// - Rotation problem in gradient-tests.swf.
// - The current coordinate system 
//
// TODOs:
// - Implement focal gradients.
// - Implement unimplemented methods.
// - Would be nice to have a header/implementation separation.
// - Document workings of Cairo and this renderer.
// - Test bitmap implementation correctness.
// - Figure out what extend types should be used and when.
// - Figure out what the deal with subpixel offsets is.
// - Cleanups.
// - Optimizations.
//
// Already implemented:
// - outlines
// - fills: solid, linear, radial and bitmap
// - bitmaps
// - fonts
// - masks
// - video (from old Cairo renderer)

#include <cairo/cairo.h>
#include <boost/scoped_array.hpp>
#include "render_handler.h"
#include "image.h"
#include <cmath>

namespace gnash {



// Converts from RGB image to 32-bit pixels in CAIRO_FORMAT_RGB24 format
static void
rgb_to_cairo_rgb24(boost::uint8_t* dst, const image::rgb* im)
{
  boost::uint32_t* dst32 = reinterpret_cast<boost::uint32_t*>(dst);
  for (size_t y = 0;  y < im->height();  y++)
  {
	  const boost::uint8_t* src = im->scanline(y);
	  for (size_t x = 0;  x < im->width();  x++, src += 3)
	  {
	      *dst32++ = (src[0] << 16) | (src[1] << 8) | src[2];
	  }
  }
}

// Converts from RGBA image to 32-bit pixels in CAIRO_FORMAT_ARGB32 format
static void
rgba_to_cairo_argb(boost::uint8_t* dst, const image::rgba* im)
{
  boost::uint32_t* dst32 = reinterpret_cast<boost::uint32_t*>(dst);
  for (size_t y = 0;  y < im->height();  y++)
  {
	  const boost::uint8_t* src = im->scanline(y);
	  for (size_t x = 0;  x < im->width();  x++, src += 4)
	  {
      const boost::uint8_t& r = src[0],
                     g = src[1],
                     b = src[2],
                     a = src[3];
      
      if (a) {  
        *dst32++ = (a << 24) | (r << 16) | (g << 8) | b;       
      } else {
        *dst32++ = 0;
      }
	  }
  }
}



class bitmap_info_cairo : public bitmap_info
{
  public:
    bitmap_info_cairo(boost::uint8_t* data, int width, int height,
                           size_t bpp, cairo_format_t format)
    : _data(data),
      _width(width),
      _height(height),
      _bytes_per_pixel(bpp),
      _format(format),
      _surface(cairo_image_surface_create_for_data(_data.get(),
               format, width, height, width * bpp)),
      _pattern(cairo_pattern_create_for_surface (_surface))
    {
    }
    
    ~bitmap_info_cairo()
    {
      cairo_surface_destroy(_surface);
    }
    
    cairo_pattern_t* apply(const cairo_matrix_t* mat, int fill_type)
    {	       
      cairo_pattern_set_matrix(_pattern, mat);
      
      cairo_extend_t extend = CAIRO_EXTEND_REPEAT;

#if 0
      // The extend type should probably depend on certain factors, but which?
      switch(fill_type) {
        case SWF::FILL_CLIPPED_BITMAP:
          break;
        case SWF::FILL_CLIPPED_BITMAP_HARD:
        case SWF::FILL_TILED_BITMAP_HARD:
          break;
        case SWF::FILL_TILED_BITMAP:
          break;
      }
#endif
    
      cairo_pattern_set_extend(_pattern, extend);
      
      return _pattern;
    }
   
  private:
    boost::scoped_array<boost::uint8_t> _data;
    int _width;
    int _height;
    size_t _bytes_per_pixel;
    cairo_format_t _format;
    cairo_surface_t* _surface;
    cairo_pattern_t* _pattern;
};

static void
init_cairo_matrix(cairo_matrix_t* cairo_matrix, const matrix& gnash_matrix)
{
  cairo_matrix_init(cairo_matrix,
    gnash_matrix.m_[0][0], gnash_matrix.m_[1][0],
    gnash_matrix.m_[0][1], gnash_matrix.m_[1][1],
    gnash_matrix.m_[0][2], gnash_matrix.m_[1][2]);
}


/// Transforms the current Cairo matrix using the given matrix. When it goes
/// out of scope, the matrix will be reset to what it was before the new matrix
/// was applied.
class CairoScopeMatrix : public boost::noncopyable
{
public:
  CairoScopeMatrix(cairo_t* cr, const matrix& new_mat)
   : _cr(cr)
  {
    cairo_get_matrix(_cr, &old_mat);

    cairo_matrix_t tmp;
    init_cairo_matrix(&tmp, new_mat);
    cairo_transform(_cr, &tmp);    
  }

  ~CairoScopeMatrix()
  {
    cairo_set_matrix(_cr, &old_mat);
  }

private:
  cairo_t* _cr;
  cairo_matrix_t old_mat;
};


class DSOEXPORT render_handler_cairo: public render_handler
{
  typedef std::vector<path> PathVec;
  typedef std::vector<const path*> PathPtrVec;
public:

  render_handler_cairo()
    : _video_bufsize(0),
      _drawing_mask(false)
  {    
  	cairo_surface_t* dummy_surface = cairo_image_surface_create(
	                                     CAIRO_FORMAT_A8, 1, 1);
	  _cr = cairo_create(dummy_surface);
	  cairo_surface_destroy(dummy_surface);
  }
  
  ~render_handler_cairo()
  {
  }

  virtual bitmap_info*  create_bitmap_info_rgb(image::rgb* im) 
  {
    int buf_size = im->width() * im->height() * 4;
    boost::uint8_t* buffer = new boost::uint8_t[buf_size];
    
    rgb_to_cairo_rgb24(buffer, im);
    
    return new bitmap_info_cairo(buffer, im->width(), im->height(), 4,
                                 CAIRO_FORMAT_RGB24);
  }

  virtual bitmap_info*  create_bitmap_info_rgba(image::rgba* im)
  {        
    int buf_size = im->width() * im->height() * 4;
    boost::uint8_t* buffer = new boost::uint8_t[buf_size];
    
    rgba_to_cairo_argb(buffer, im);
    
    return new bitmap_info_cairo(buffer, im->width(), im->height(), 4,
                                 CAIRO_FORMAT_ARGB32);
  }

  virtual void  delete_bitmap_info(bitmap_info* bi)
  {
    delete bi;
  }

  virtual int videoFrameFormat()
  {
    return render_handler::RGB;
  }
  
  virtual void drawVideoFrame(image::image_base* baseframe, const matrix* m, const rect* bounds)
  {
    // Extract frame attributes
    image::rgb* frame = static_cast<image::rgb*>(baseframe);
    int         w = frame->width();
    int         h = frame->height();

    // Compute bounding rectangle size relative to video object
    double w_scale = bounds->width() / w;
    double h_scale = bounds->height() / h;

    // Fit video to bounding rectangle
    cairo_matrix_t mat;
    cairo_matrix_init_scale(&mat, w_scale, h_scale);
    cairo_matrix_translate(&mat,
	    bounds->get_x_min(), bounds->get_y_min());

    // Now apply transformation to video
    cairo_matrix_t frame_mat;
    init_cairo_matrix(&frame_mat, *m);

    cairo_matrix_multiply(&mat, &mat, &frame_mat);

    // Inverse the matrix for pattern space
    cairo_matrix_invert(&mat);

    // Convert RGB frame to cairo format
    size_t buf_size = w * h * 4;
    
    if (_video_bufsize < buf_size) {
      _video_buffer.reset(new boost::uint8_t[buf_size]);
	    _video_bufsize = buf_size;
    }    
    
    rgb_to_cairo_rgb24(_video_buffer.get(), frame);

    // Create a pattern from the the RGB frame
    cairo_surface_t* surface = cairo_image_surface_create_for_data(
	    _video_buffer.get(), CAIRO_FORMAT_RGB24, w, h, w * 4);
    cairo_pattern_t* pattern = cairo_pattern_create_for_surface(surface);
    cairo_pattern_set_extend(pattern, CAIRO_EXTEND_NONE);
    cairo_pattern_set_matrix(pattern, &mat);

    // Draw the frame now
    cairo_save(_cr);
    cairo_set_source(_cr, pattern);
    
    geometry::Range2d<float> range = bounds->getRange();
	  m->transform(range);
	  
	  cairo_rectangle(_cr, range.getMinX(), range.getMinY(), range.width(),
	                       range.height());
    cairo_clip(_cr);
    cairo_paint(_cr);
    cairo_restore(_cr);

    // Clean up
    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(surface);
  }

  
  // FIXME
  geometry::Range2d<int>
  world_to_pixel(const rect& worldbounds)
{
  // TODO: verify this is correct
  geometry::Range2d<int> ret(worldbounds.getRange());
  ret.scale(1.0/20.0); // twips to pixels
  return ret;
}

  // FIXME
  point 
  pixel_to_world(int x, int y)
  {
    // TODO: verify this is correct
    return point(PIXELS_TO_TWIPS(x), PIXELS_TO_TWIPS(y));
  }
  
  void
  set_color(const rgba& c)
  {
    cairo_set_source_rgba (_cr, c.m_r / 255.0, c.m_g / 255.0,
                                c.m_b / 255.0, c.m_a / 255.0);
  }

  void set_invalidated_regions(const InvalidatedRanges& ranges)
  {
    _invalidated_ranges = ranges;
  }

  static void
  snap_to_pixel(cairo_t* cr, double& x, double& y, bool up)
  {
    cairo_user_to_device(cr, &x, &y);
   
    if (up) {
      x = std::ceil(x);
      y = std::ceil(y);
    } else {
      x = std::floor(x);
      y = std::floor(y);
    }

    cairo_device_to_user(cr, &x, &y);
  }

  virtual void  begin_display(
    const rgba& bg_color,
    int viewport_x0, int viewport_y0,
    int viewport_width, int viewport_height,
    float x0, float x1, float y0, float y1)
  {
    float display_width  = fabsf(x1 - x0);
    float display_height = fabsf(y1 - y0);

    cairo_identity_matrix(_cr);

    cairo_save(_cr);

    if (bg_color.m_a) {
      set_color(bg_color);
    }

    cairo_scale(_cr, viewport_width / display_width,
                     viewport_height / display_height);
    cairo_translate(_cr, x0, y0);

    for (size_t rno=0; rno < _invalidated_ranges.size(); rno++) {
    
      const Range2d<float>& range = _invalidated_ranges.getRange(rno);
      if (range.isNull()) {
        continue;
      }
      if (range.isWorld()) {
        cairo_rectangle(_cr, x0, y0, display_width, display_height);
        break;
      }

      double x = range.getMinX(),
             y = range.getMinY(),
             maxx = range.getMaxX(),
             maxy = range.getMaxY();

      snap_to_pixel(_cr, x, y, false);
      snap_to_pixel(_cr, maxx, maxy, true);

      cairo_rectangle(_cr, x, y, maxx - x, maxy - y);
    }

    cairo_clip(_cr);

    // Paint the background color over the clipped region(s).
    cairo_paint(_cr);
  }

  virtual void  end_display()
  {
    cairo_restore(_cr);
  }
    
  virtual void  draw_line_strip(const void* coords, int vertex_count,
      const rgba& color, const matrix& mat)
  {
    log_unimpl("draw_line_strip");
  }
  
  virtual void  draw_poly(const point* corners, size_t corner_count, 
    const rgba& fill, const rgba& outline, const matrix& mat, bool masked)
  {
    CairoScopeMatrix mat_transformer(_cr, mat);

    if (corner_count < 1) {
      return;
    }
        
    cairo_move_to(_cr, corners[0].x, corners[0].y);
    
    for (size_t i = 0; i < corner_count; ++i) {
      cairo_line_to(_cr, corners[i].x, corners[i].y);
    }
    
    cairo_close_path(_cr);
    
    if (fill.m_a) {
      set_color(fill);
      cairo_fill_preserve(_cr);    
    }
    
    if (outline.m_a) {
      set_color(outline);
      
      // FIXME: coordinate alignment (for single-pixel lines should be in
      //        between two pixels for sharp hair line.
      
      cairo_set_line_width(_cr, 20.0);
      cairo_stroke_preserve(_cr);    
    }
    
    // Clear the current path which was _preserve()d.
    cairo_new_path(_cr);    
  }

  virtual void  set_antialiased(bool enable)
  {
    log_unimpl("set_antialiased");
  }
    
  virtual void begin_submit_mask()
  {
    PathVec mask;
    _masks.push_back(mask);
    
    _drawing_mask = true;
  }
  
  virtual void end_submit_mask()
  {
    _drawing_mask = false;
    
    
    // Load the mask paths into the cairo context.
    add_paths(_masks.back());
      
    // Save the context so we can return to the former clip later.
    cairo_save(_cr);
    
    // Clip the fills defined by the current paths.
    cairo_clip(_cr);
    
    // Remove the current path since we have no further use for it (and may
    // confuse us later). 
    cairo_new_path(_cr);
    
  }
  
  virtual void disable_mask()
  {
    // Restore the previous clip.
    cairo_restore(_cr);

    _masks.pop_back();
  }

  void add_path(cairo_t* cr, const path& cur_path)
  {  
    cairo_move_to(cr, cur_path.ap.x, cur_path.ap.y);
    
    int prev_x = cur_path.ap.x,
        prev_y = cur_path.ap.y;
    
    for (std::vector<edge>::const_iterator it = cur_path.m_edges.begin(),
         end = cur_path.m_edges.end(); it != end; ++it) {
      const edge& cur_edge = *it;
      
      if (cur_edge.is_straight()) {
        cairo_line_to(cr, cur_edge.ap.x, cur_edge.ap.y);
      } else {
      
        // Cairo expects a cubic Bezier curve, while Flash gives us a
        // quadratic one. We must apply a conversion:
        
        const float two_thirds = 2.0/3.0;
        const float one_third = 1 - two_thirds;
        
        float x1 = prev_x + two_thirds * (cur_edge.cp.x - prev_x);
        float y1 = prev_y + two_thirds * (cur_edge.cp.y - prev_y);
        
        float x2 = cur_edge.cp.x + one_third * (cur_edge.ap.x - cur_edge.cp.x);
        float y2 = cur_edge.cp.y + one_third * (cur_edge.ap.y - cur_edge.cp.y);
        
        const int& x3 = cur_edge.ap.x;
        const int& y3 = cur_edge.ap.y;
    
    
        cairo_curve_to(cr, x1, y1, x2, y2, x3, y3);
      }
      
      prev_x = cur_edge.ap.x;
      prev_y = cur_edge.ap.y;
      
    }
  
  }
  
  
  PathPtrVec
  get_paths_by_style(const PathVec& path_vec, unsigned int style)
  {
    PathPtrVec paths;
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const path& cur_path = *it;
      
      if (cur_path.m_fill0 == style) {
        paths.push_back(&cur_path);
      }
      
      if (cur_path.m_fill1 == style) {
        paths.push_back(&cur_path);
      }
      
    }
    return paths;
  }
  
  void apply_line_style(const line_style& style, const cxform& cx)
  {
    float width = style.get_width();

    if ( width == 0.0 ) {
      // TODO: test this!
      cairo_set_line_width(_cr, 1.0); // expected: 1 pixel
    } else {
      cairo_set_line_width(_cr, width);
    }
    
    rgba color = cx.transform(style.get_color());

    set_color(color);
  }
  
  std::vector<cairo_pattern_t*>
  build_cairo_styles(const std::vector<fill_style>& fill_styles, const cxform& cx,
                     const matrix& mat)
  {
    std::vector<cairo_pattern_t*> styles_vec_cairo;

    for (std::vector<fill_style>::const_iterator it = fill_styles.begin(),
         end = fill_styles.end(); it != end; ++it) {
      const fill_style& cur_style = *it;
      
      cairo_pattern_t* pattern = get_cairo_pattern(cur_style, cx, mat);
      styles_vec_cairo.push_back(pattern);
    }
    
    return styles_vec_cairo;
  }
  
  void
  pattern_add_color_stops(const fill_style& style, cairo_pattern_t* pattern,
                          const cxform& cx)
  {      
    for (int index = 0; index < style.get_color_stop_count(); ++index) {
    
      const gradient_record& grad = style.get_color_stop(index);
      
      rgba c = cx.transform(grad.m_color);

      cairo_pattern_add_color_stop_rgba (pattern,
        grad.m_ratio / 255.0, c.m_r / 255.0, c.m_g / 255.0,
        c.m_b / 255.0, c.m_a / 255.0);
    }
  }
  
  cairo_pattern_t*
  get_cairo_pattern(const fill_style& style, const cxform& cx, const matrix& mat)
  {
    int fill_type = style.get_type();
    cairo_pattern_t* pattern = NULL;
        
    switch (fill_type) {

      case SWF::FILL_LINEAR_GRADIENT:
      {
        matrix m = style.get_gradient_matrix();
        matrix cm;
        cm.set_inverse(mat);
        m.concatenate(cm);         
      
        cairo_matrix_t mat;
        init_cairo_matrix(&mat, m);
        
        pattern = cairo_pattern_create_linear(0, 0, 256.0, 0);
        cairo_pattern_set_matrix (pattern, &mat);
        
        pattern_add_color_stops(style, pattern, cx);
                                                   
        break;
      }
      case SWF::FILL_RADIAL_GRADIENT:
      {        
        matrix m = style.get_gradient_matrix();
        matrix cm;
        cm.set_inverse(mat);
        m.concatenate(cm);
        
        // move the center of the radial fill to where it should be, according to Udo
        gnash::matrix transl;
        transl.concatenate_translation(-32.0f, -32.0f);
        transl.concatenate(m);  
      
        cairo_matrix_t mat;
        init_cairo_matrix(&mat, transl);
        
        pattern = cairo_pattern_create_radial(0.0, 0.0, 0.0, 0.0, 0.0, 32.0);

        cairo_pattern_set_matrix (pattern, &mat);          
        
        pattern_add_color_stops(style, pattern, cx);
        break;
      }
      case SWF::FILL_FOCAL_GRADIENT:
      {
        log_unimpl("focal gradient fill");
        
        break;
      }              
      case SWF::FILL_TILED_BITMAP_HARD:
      case SWF::FILL_TILED_BITMAP:
      case SWF::FILL_CLIPPED_BITMAP:
      case SWF::FILL_CLIPPED_BITMAP_HARD:
      {
        matrix m = style.get_bitmap_matrix();        
        
        bitmap_info_cairo* binfo
          = static_cast<bitmap_info_cairo*>(style.get_bitmap_info());

        if (!binfo) {
          return NULL;
        }
        
        cairo_matrix_t mat;
        init_cairo_matrix(&mat, m);       

        pattern = binfo->apply(&mat, fill_type);
        
        break;
      } 

      case SWF::FILL_SOLID:
      {            
        rgba c = cx.transform(style.get_color());
        pattern = cairo_pattern_create_rgba (c.m_r / 255.0, c.m_g / 255.0,
                                             c.m_b / 255.0, c.m_a / 255.0);
        break;
      }
      
    } // switch
    
    return pattern;

  }

  void draw_outlines(const PathVec& path_vec, const std::vector<line_style>& line_styles, const cxform& cx)
  {  
    cairo_set_line_cap(_cr, CAIRO_LINE_CAP_ROUND); // TODO: move to init
    cairo_set_line_join(_cr, CAIRO_LINE_JOIN_ROUND);

    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const path& cur_path = *it;
      if (!cur_path.m_line) {
        continue;
      }
      
      apply_line_style(line_styles[cur_path.m_line-1], cx);
      
      add_path(_cr, cur_path);
      
      cairo_stroke(_cr);
    }  
  }

void
draw_subshape(const PathVec& path_vec, const matrix& mat, const cxform& cx,
    float pixel_scale,
    const std::vector<cairo_pattern_t*>& fill_styles,
    const std::vector<line_style>& line_styles)
  {   
    for (size_t i = 0; i < fill_styles.size(); ++i) {
      PathPtrVec paths = get_paths_by_style(path_vec, i+1);
      
      
      for (PathPtrVec::const_iterator iter = paths.begin(), final = paths.end();
           iter != final; ++iter) {
        const path* cur_path = *iter;
        
        add_path(_cr, *cur_path);
        
      } // for (PathVec..)

      if (paths.size()) {
        cairo_set_source(_cr, fill_styles[i]);
        cairo_fill(_cr);
      }
      
    } // for(std::vector<fill_style> ..)
    
    
    
    draw_outlines(path_vec, line_styles, cx);
    
  }
	
	 
  std::vector<PathVec::const_iterator>
	find_subshapes(const PathVec& path_vec)
  {
    std::vector<PathVec::const_iterator> subshapes;
    
    PathVec::const_iterator it = path_vec.begin(),
                            end = path_vec.end();
    
    subshapes.push_back(it);
    ++it;

    for (;it != end; ++it) {
      const path& cur_path = *it;
	  
	    if (cur_path.m_new_shape) {
	      subshapes.push_back(it); 
	    }	  
	  } 
	  
	  subshapes.push_back(end);
	  
	  return subshapes;
	}

  void draw_mask(const PathVec& path_vec)
  {    
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const path& cur_path = *it;
      
      if (cur_path.m_fill0 || cur_path.m_fill1) {
        _masks.back().push_back(cur_path);     
      }
    }  
  }
  
  void
  add_paths(const PathVec& path_vec)
  {
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
      const path& cur_path = *it;
      
      add_path(_cr, cur_path);
    }  
  }
  
  /// Takes a path and translates it using the given matrix. The new path
  /// is stored in paths_out.
  /// Taken from render_handler_agg.cpp.  
  void apply_matrix_to_paths(const std::vector<path> &paths_in, 
    std::vector<path> &paths_out, const matrix& mat) {
    
    int pcount, ecount;
    int pno, eno;
    
    // copy path
    paths_out = paths_in;    
    pcount = paths_out.size();        
    
    for (pno=0; pno<pcount; pno++) {
    
      path &the_path = paths_out[pno];     
      point oldpnt(the_path.ap.x, the_path.ap.y);
      point newpnt;
      mat.transform(&newpnt, oldpnt);
      the_path.ap.x = newpnt.x;    
      the_path.ap.y = newpnt.y;
      
      ecount = the_path.m_edges.size();
      for (eno=0; eno<ecount; eno++) {
      
        edge &the_edge = the_path.m_edges[eno];
        
        oldpnt.x = the_edge.ap.x;
        oldpnt.y = the_edge.ap.y;
        mat.transform(&newpnt, oldpnt);
        the_edge.ap.x = newpnt.x;
        the_edge.ap.y = newpnt.y;
        
        oldpnt.x = the_edge.cp.x;
        oldpnt.y = the_edge.cp.y;
        mat.transform(&newpnt, oldpnt);
        the_edge.cp.x = newpnt.x;
        the_edge.cp.y = newpnt.y;
      
      }          
      
    } 
    
  }


  virtual void draw_shape_character(shape_character_def *def, 
    const matrix& mat,
    const cxform& cx,
    float pixel_scale,
    const std::vector<fill_style>& fill_styles,
    const std::vector<line_style>& line_styles)
  {
        
    const PathVec& path_vec = def->get_paths();
    
    if (!path_vec.size()) {
      return;    
    }
    
    cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_EVEN_ODD); // TODO: Move to init
        
    if (_drawing_mask) {      
      PathVec scaled_path_vec;
      
      apply_matrix_to_paths(path_vec, scaled_path_vec, mat);
      draw_mask(scaled_path_vec); 
      return;
    }
    
    CairoScopeMatrix mat_transformer(_cr, mat);

    std::vector<PathVec::const_iterator> subshapes = find_subshapes(path_vec);
    
    std::vector<cairo_pattern_t*> fill_styles_cairo
      = build_cairo_styles(fill_styles, cx, mat);
    
    for (size_t i = 0; i < subshapes.size()-1; ++i) {
      PathVec subshape_paths;
      
      if (subshapes[i] != subshapes[i+1]) {
        subshape_paths = PathVec(subshapes[i], subshapes[i+1]);
      } else {
        subshape_paths.push_back(*subshapes[i]);
      }
      
      draw_subshape(subshape_paths, mat, cx, pixel_scale, fill_styles_cairo,
                    line_styles);
    }
    
    destroy_cairo_patterns(fill_styles_cairo);
  }
  
  void
  destroy_cairo_patterns(std::vector<cairo_pattern_t*>& patterns)
  {
    for (std::vector<cairo_pattern_t*>::iterator it = patterns.begin(),
         end = patterns.end(); it != end; ++it) {
      cairo_pattern_t* pattern = *it;
      if (pattern && cairo_pattern_get_type(pattern) !=
                     CAIRO_PATTERN_TYPE_SURFACE) {
        cairo_pattern_destroy(pattern);
      }
    }
  }  

  virtual void draw_glyph(shape_character_def *def, const matrix& mat,
    const rgba& color, float pixel_scale)
  {
  
    cxform dummy_cx;
    std::vector<fill_style> glyph_fs;
    
    fill_style coloring;
    coloring.setSolid(color);
    
    glyph_fs.push_back(coloring);
    
    const PathVec& path_vec = def->get_paths();
    
    std::vector<line_style> dummy_ls;
    
    CairoScopeMatrix mat_transformer(_cr, mat);
    
    std::vector<cairo_pattern_t*> fill_styles_cairo
      = build_cairo_styles(glyph_fs, dummy_cx, mat);
      
    
    draw_subshape(path_vec, mat, dummy_cx, pixel_scale, fill_styles_cairo, dummy_ls);
    
    destroy_cairo_patterns(fill_styles_cairo);
  }

  void
  set_context(cairo_t* context)
  {
    if (context == _cr) {
      return;    
    }

    cairo_destroy(_cr);

    _cr = context;
  }
  
  void
  init_cairo_matrix(cairo_matrix_t* cairo_matrix, const matrix& gnash_matrix)
  {
    cairo_matrix_init(cairo_matrix,
	    gnash_matrix.m_[0][0], gnash_matrix.m_[1][0],
	    gnash_matrix.m_[0][1], gnash_matrix.m_[1][1],
	    gnash_matrix.m_[0][2], gnash_matrix.m_[1][2]);
  }
  
  bool initTestBuffer(unsigned width, unsigned height)
  {
    cairo_surface_t* test_surface =
      cairo_image_surface_create (CAIRO_FORMAT_ARGB32, width, height);
    
    if (cairo_surface_status(test_surface) != CAIRO_STATUS_SUCCESS) {
      return false;
    }
    
    cairo_t* context = cairo_create(test_surface);
    
    if (cairo_status(context) == CAIRO_STATUS_NO_MEMORY) {    
      return false;    
    }
    
    cairo_surface_destroy(test_surface);
    
    set_context(context);    
  
    return true;
  }
  
  unsigned int getBitsPerPixel() const
  {
    cairo_surface_t* surface = cairo_get_target (_cr);
    
    cairo_format_t format = cairo_image_surface_get_format (surface);
    
    switch(format) {
      case CAIRO_FORMAT_ARGB32:
        return 32;
      case CAIRO_FORMAT_RGB24:
        // In practice this is 32 with 8 bits unused...
        return 24;
      case CAIRO_FORMAT_A8:
        return 8;
      case CAIRO_FORMAT_A1:
        return 1;
      default:
        return 0;
    }   
  }
  
  bool getPixel(rgba& color_return, int x, int y)
  {
    if (x < 0 || y < 0) {
      return false;
    }

    cairo_surface_t* surface = cairo_get_target (_cr);
    
    assert(cairo_image_surface_get_format (surface) == CAIRO_FORMAT_ARGB32);
    
    unsigned char* data = cairo_image_surface_get_data (surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface); // in bytes
    
    if (x >= width || y >= height) {    
      return false;
    }
    
    unsigned char* ptr = data + y * stride + x * 4;
    
    color_return.m_a = ptr[3];
    color_return.m_r = ptr[2];
    color_return.m_g = ptr[1];
    color_return.m_b = ptr[0];

    return true;
  }
  
    
private:
  /// The cairo context.
  cairo_t* _cr;
  boost::scoped_array<boost::uint8_t> _video_buffer;
  std::vector<PathVec> _masks;
  size_t _video_bufsize;
  bool _drawing_mask;
  InvalidatedRanges _invalidated_ranges;
    
}; // class render_handler_cairo





namespace renderer {


namespace cairo
{

DSOEXPORT render_handler*
create_handler()
// Factory.
{
	return new render_handler_cairo();
}

DSOEXPORT void
set_context(render_handler* handler, cairo_t* context)
{
  render_handler_cairo* cairo_handler = static_cast<render_handler_cairo*>(handler);
  cairo_handler->set_context(context);
}


} // namespace cairo
} // namespace renderer
} // namespace gnash


