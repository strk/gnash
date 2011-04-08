// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011
//   Free Software Foundation, Inc
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
// - Rotation problem in gradient-tests.swf.
//
// TODOs:
// - Would be nice to have a header/implementation separation.
// - Document workings of Cairo and this renderer.
// - Test bitmap implementation correctness.
// - Figure out what extend types should be used and when.
// - Cleanups.
// - Optimizations.
//
// Already implemented:
// - outlines
// - fills: solid, linear, radial, focal and bitmap
// - bitmaps
// - fonts
// - masks
// - video (from old Cairo renderer)

#include "Renderer_cairo.h"

#include <cmath>
#include <cairo/cairo.h>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>

#include "Renderer.h"
#include "GnashImage.h"
#include "PathParser.h"
#include "swf/ShapeRecord.h"
#include "utility.h"
#include "FillStyle.h"
#include "Transform.h"
#include "ImageIterators.h"
#include "CachedBitmap.h"

namespace gnash {

namespace {
    void pattern_add_color_stops(const GradientFill& f,
            cairo_pattern_t* pattern, const SWFCxForm& cx);
    void init_cairo_matrix(cairo_matrix_t* cairo_matrix,
            const SWFMatrix& gnash_matrix);
}

namespace {

// Converts from RGB image to 32-bit pixels in CAIRO_FORMAT_RGB24 format
static void
rgb_to_cairo_rgb24(boost::uint8_t* dst, const image::GnashImage* im)
{
    boost::uint32_t* dst32 = reinterpret_cast<boost::uint32_t*>(dst);
    for (size_t y = 0;  y < im->height();  y++)
    {
        const boost::uint8_t* src = scanline(*im, y);
        for (size_t x = 0;  x < im->width();  x++, src += 3) {
            *dst32++ = (src[0] << 16) | (src[1] << 8) | src[2];
        }
    }
}

// Converts from RGBA image to 32-bit pixels in CAIRO_FORMAT_ARGB32 format
static void
rgba_to_cairo_argb(boost::uint8_t* dst, const image::GnashImage* im)
{
    boost::uint32_t* dst32 = reinterpret_cast<boost::uint32_t*>(dst);
    for (size_t y = 0;  y < im->height();  y++)
    {
        const boost::uint8_t* src = scanline(*im, y);
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

class bitmap_info_cairo : public CachedBitmap, boost::noncopyable
{
  public:
    bitmap_info_cairo(boost::uint8_t* data, int width, int height,
                           size_t bpp, cairo_format_t format)
        :
        _data(data),
        _width(width),
        _height(height),
        _bytes_per_pixel(bpp),
        _format(format),
        _surface(cairo_image_surface_create_for_data(_data.get(),
                 format, width, height, width * bpp)),
        _pattern(cairo_pattern_create_for_surface (_surface))
    {
      
      assert(cairo_surface_status(_surface) == CAIRO_STATUS_SUCCESS);
      assert(cairo_pattern_status(_pattern) == CAIRO_STATUS_SUCCESS);
    }

    virtual void dispose() {
        _image.reset();
        _data.reset();
    }

    virtual bool disposed() const {
        return !_data.get();
    }
   
    
    image::GnashImage& image() {
        if (_image.get()) return *_image;

        switch (_format) {
            case CAIRO_FORMAT_RGB24:
                _image.reset(new image::ImageRGB(_width, _height));
                break;

            case CAIRO_FORMAT_ARGB32:
                _image.reset(new image::ImageRGBA(_width, _height));
                break;

            default:
                std::abort();
        }

        // We assume that cairo uses machine-endian order, as that's what
        // the existing conversion functions do.
        boost::uint32_t* start =
            reinterpret_cast<boost::uint32_t*>(_data.get());
        const size_t sz = _width * _height;
        std::copy(start, start + sz, image::begin<image::ARGB>(*_image));
        return *_image;
    }

    void update() const {
        if (!_image.get()) return;
        switch (_format) {
            case CAIRO_FORMAT_RGB24:
                rgb_to_cairo_rgb24(_data.get(), _image.get());
                break;
            case CAIRO_FORMAT_ARGB32:
                rgba_to_cairo_argb(_data.get(), _image.get());
                break;
            default:
                break;
        }
        _image.reset();
    }

    ~bitmap_info_cairo()
    {
      cairo_surface_destroy(_surface);
      cairo_pattern_destroy(_pattern);
    }
    
    cairo_pattern_t* apply(const cairo_matrix_t* mat, int /*fill_type*/) const
    {
      assert(mat);
      assert(_pattern);
      update();
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
    mutable boost::scoped_ptr<image::GnashImage> _image;
    boost::scoped_array<boost::uint8_t> _data;
    int _width;
    int _height;
    size_t _bytes_per_pixel;
    cairo_format_t _format;
    cairo_surface_t* _surface;
    cairo_pattern_t* _pattern;
};


/// Style handler
//
/// Transfer FillStyles to agg styles.
struct StyleHandler : boost::static_visitor<cairo_pattern_t*>
{
    StyleHandler(const SWFCxForm& c)
        :
        _cx(c)
    {}

    cairo_pattern_t* operator()(const GradientFill& f) const {
        const SWFMatrix m = f.matrix();
        switch (f.type()) {
            case GradientFill::LINEAR:
            {
                cairo_matrix_t mat;
                init_cairo_matrix(&mat, m);
      
                cairo_pattern_t* pattern =
                    cairo_pattern_create_linear(0, 0, 256.0, 0);
                cairo_pattern_set_matrix (pattern, &mat);

                pattern_add_color_stops(f, pattern, _cx);
                return pattern;
            }
            case GradientFill::RADIAL:
            {
              
                // Undo the translation our parser applied.
                gnash::SWFMatrix transl;
                transl.concatenate(m);

                cairo_matrix_t mat;
                init_cairo_matrix(&mat, transl);

                /// This is 0 for radial gradients.
                const double focal_pos = 32.0f * f.focalPoint();

                cairo_pattern_t* pattern =
                    cairo_pattern_create_radial(focal_pos, 0.0, 0.0,
                            0.0, 0.0, 32.0f);

                cairo_pattern_set_matrix (pattern, &mat);          
              
                pattern_add_color_stops(f, pattern, _cx);
                return pattern;
            }
        }
        // We should never get here.
        return 0;
    }

    cairo_pattern_t* operator()(const SolidFill& f) const {
        rgba c = _cx.transform(f.color());
        cairo_pattern_t* pattern =
            cairo_pattern_create_rgba(c.m_r / 255.0, c.m_g / 255.0,
                                    c.m_b / 255.0, c.m_a / 255.0);
        return pattern;
    }

    cairo_pattern_t* operator()(const BitmapFill& f) const {
        SWFMatrix m = f.matrix();
      
        const CachedBitmap* bm = f.bitmap();

        if (!bm) {
            // See misc-swfmill.all/missing_bitmap.swf
            cairo_pattern_t* pattern =
                cairo_pattern_create_rgba(255, 0, 0, 255);
            return pattern;
        }

        if (bm->disposed()) {
            // See misc-ming.all/BeginBitmapFill.swf
            cairo_pattern_t* pattern =
                cairo_pattern_create_rgba(0, 0, 0, 0);
            return pattern;
        }

        const bitmap_info_cairo* binfo =
            dynamic_cast<const bitmap_info_cairo*>(bm);
      
        cairo_matrix_t mat;
        init_cairo_matrix(&mat, m);       

        // TODO: the second argument should be fill_type but is unused.
        cairo_pattern_t* pattern = binfo->apply(&mat, 0);
        return pattern;
    }

private:
    const SWFCxForm& _cx;
};  

}


static void
snap_to_half_pixel(cairo_t* cr, double& x, double& y)
{
    cairo_user_to_device(cr, &x, &y);

    x = std::floor(x + 0.5) + 0.5;
    y = std::floor(y + 0.5) + 0.5;

    cairo_device_to_user(cr, &x, &y);
}

static cairo_pattern_t*
get_cairo_pattern(const FillStyle& style, const SWFCxForm& cx)
{
    StyleHandler st(cx);
    cairo_pattern_t* pattern = boost::apply_visitor(st, style.fill);
  
    return pattern;

}

class CairoPathRunner : public PathParser
{
public:
  CairoPathRunner(Renderer_cairo& renderer,
                  const std::vector<Path>& paths,
                  const std::vector<FillStyle>& FillStyles, cairo_t* context)
  : PathParser(paths, FillStyles.size()),
    _renderer(renderer),
    _cr(context),
    _pattern(0),
    _FillStyles(FillStyles)
  {
  }
  
  virtual void prepareFill(int fill_index, const SWFCxForm& cx)
  {
    if (!_pattern) {
      _pattern = get_cairo_pattern(_FillStyles[fill_index-1], cx);
    }
  }
  virtual void terminateFill(int FillStyle)
  {
    UNUSED(FillStyle);

    if (!_pattern) {
      cairo_new_path(_cr);
      return;
    }
    
    cairo_set_source(_cr, _pattern);

    cairo_fill(_cr);

    // Surfaces are owned by const bitmap_info_cairo
    if (cairo_pattern_get_type(_pattern) != CAIRO_PATTERN_TYPE_SURFACE) {
      cairo_pattern_destroy(_pattern);
      _pattern = 0;
    }
  }

  void moveTo(const point& ap)
  {
      cairo_move_to(_cr, ap.x, ap.y);
  }

  virtual void curveTo(const Edge& cur_edge)
  {
    const float two_thirds = 2.0/3.0;
    const float one_third = 1 - two_thirds;

    double x, y;
    cairo_get_current_point(_cr, &x, &y);

    double x1 = x + two_thirds * (cur_edge.cp.x - x);
    double y1 = y + two_thirds * (cur_edge.cp.y - y);

    double x2 = cur_edge.cp.x + one_third * (cur_edge.ap.x - cur_edge.cp.x);
    double y2 = cur_edge.cp.y + one_third * (cur_edge.ap.y - cur_edge.cp.y);

    x = cur_edge.ap.x;
    y = cur_edge.ap.y;

    cairo_curve_to(_cr, x1, y1, x2, y2, x, y);    
  }

  void lineTo(const point& ap)
  {
    double x = ap.x;
    double y = ap.y;
      
    cairo_line_to(_cr, x, y);
  }

private:
  Renderer_cairo& _renderer;
  cairo_t* _cr;
  cairo_pattern_t* _pattern;
  const std::vector<FillStyle>& _FillStyles;
};




/// Transforms the current Cairo SWFMatrix using the given SWFMatrix.
/// When it goes out of scope, the SWFMatrix will be reset to what it
/// was before the new SWFMatrix was applied.
class CairoScopeMatrix : public boost::noncopyable
{
public:
  CairoScopeMatrix(cairo_t* cr, const SWFMatrix& new_mat)
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


/// Implementation of Renderer_cairo class
Renderer_cairo::Renderer_cairo()
    : _video_bufsize(0),
      _drawing_mask(false)
{    
    _cr = cairo_create(NULL);
    cairo_matrix_init_scale(&_stage_mat, 1/20.0f, 1/20.0f);
}
  
Renderer_cairo::~Renderer_cairo()
{
    cairo_destroy(_cr);
}

CachedBitmap*
Renderer_cairo::createCachedBitmap(std::auto_ptr<image::GnashImage> im) 
{
    int buf_size = im->width() * im->height() * 4;
    boost::uint8_t* buffer = new boost::uint8_t[buf_size];

    switch (im->type())
    {
        case image::TYPE_RGB:
        {
            rgb_to_cairo_rgb24(buffer, im.get());
    
            return new bitmap_info_cairo(buffer, im->width(), im->height(), 4,
                                 CAIRO_FORMAT_RGB24);
        }
        
        case image::TYPE_RGBA:
        {
            rgba_to_cairo_argb(buffer, im.get());
    
            return new bitmap_info_cairo(buffer, im->width(), im->height(), 4,
                                 CAIRO_FORMAT_ARGB32);
        }

        default:
            std::abort();
    }
}

void
Renderer_cairo::drawVideoFrame(image::GnashImage* baseframe, const Transform& xform,
                               const SWFRect* bounds, bool smooth)
{
    if (baseframe->type() == image::TYPE_RGBA)
    {
        LOG_ONCE(log_error(_("Can't render videos with alpha")));
        return;
    }

    image::ImageRGB* frame = dynamic_cast<image::ImageRGB*>(baseframe);

    assert(frame);

    // Extract frame attributes
    int w = frame->width();
    int h = frame->height();

    // Compute bounding rectangle size relative to video object
    double w_scale = bounds->width() / w;
    double h_scale = bounds->height() / h;

    // Fit video to bounding rectangle
    cairo_matrix_t mat;
    cairo_matrix_init_scale(&mat, w_scale, h_scale);
    cairo_matrix_translate(&mat, bounds->get_x_min(), bounds->get_y_min());

    // Now apply transformation to video
    cairo_matrix_t frame_mat;
    init_cairo_matrix(&frame_mat, xform.matrix);

    cairo_matrix_multiply(&mat, &mat, &frame_mat);

    // Inverse the SWFMatrix for pattern space
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

    cairo_filter_t filter;
    switch (_quality) {
        case QUALITY_BEST:
        case QUALITY_HIGH:
            if (smooth) {
                filter = CAIRO_FILTER_GOOD;
                break;
            }
        case QUALITY_MEDIUM:
        case QUALITY_LOW:
            filter = CAIRO_FILTER_FAST;
    }
    cairo_pattern_set_filter(pattern, filter);

    // Draw the frame now
    cairo_save(_cr);
    cairo_set_source(_cr, pattern);
    
    geometry::Range2d<boost::int32_t> range = bounds->getRange();
    xform.matrix.transform(range);
  
    cairo_rectangle(_cr, range.getMinX(), range.getMinY(), range.width(),
                    range.height());
    
    cairo_clip(_cr);
    cairo_paint(_cr);
    cairo_restore(_cr);

    // Clean up
    cairo_pattern_destroy(pattern);
    cairo_surface_destroy(surface);
}

  
geometry::Range2d<int>
Renderer_cairo::world_to_pixel(const SWFRect& worldbounds) const
{
    double xmin = worldbounds.get_x_min(),
           ymin = worldbounds.get_y_min(),
           xmax = worldbounds.get_x_max(),
           ymax = worldbounds.get_y_max();

    cairo_matrix_transform_point(&_stage_mat, &xmin, &ymin);
    cairo_matrix_transform_point(&_stage_mat, &xmax, &ymax);
  
    geometry::Range2d<int> ret(xmin, ymin, xmax, ymax);
    return ret;
}

// FIXME
point
Renderer_cairo::pixel_to_world(int x, int y) const
{
    cairo_matrix_t inv_stage = _stage_mat;
    cairo_matrix_invert(&inv_stage);

    double xconv = x;
    double yconv = y;
    
    cairo_matrix_transform_point(&inv_stage, &xconv, &yconv);
    return point(xconv, yconv);
}
  
void
Renderer_cairo::set_color(const rgba& c)
{
    cairo_set_source_rgba (_cr, c.m_r / 255.0, c.m_g / 255.0,
                                c.m_b / 255.0, c.m_a / 255.0);
}

void
Renderer_cairo::set_invalidated_regions(const InvalidatedRanges& ranges)
{
    _invalidated_ranges = ranges;
}

void
Renderer_cairo::begin_display(const rgba& bg_color,
            int /*viewport_width*/, int /*viewport_height*/,
            float /*x0*/, float /*x1*/, float /*y0*/, float /*y1*/)
{
    cairo_identity_matrix(_cr);

    cairo_save(_cr);

    if (bg_color.m_a) {
      set_color(bg_color);
    }


    for (size_t rno=0; rno < _invalidated_ranges.size(); rno++) {
        const geometry::Range2d<boost::int32_t>& range =
            _invalidated_ranges.getRange(rno);
        if (range.isNull()) {
            continue;
        }
        if (range.isWorld()) {
            cairo_paint(_cr);
            // reset any rectangles that might have been added to the path...
            cairo_new_path(_cr);
            cairo_set_matrix(_cr, &_stage_mat);
            return;
        }

        double x = range.getMinX(),
               y = range.getMinY(),
               maxx = range.getMaxX(),
               maxy = range.getMaxY();

        // Transform to pixels.
        cairo_matrix_transform_point(&_stage_mat, &x, &y);
        cairo_matrix_transform_point(&_stage_mat, &maxx, &maxy);

        cairo_rectangle(_cr, rint(x), rint(y), rint(maxx - x), rint(maxy - y));
    }

    cairo_clip(_cr);

    // Paint the background color over the clipped region(s).
    cairo_paint(_cr);

    cairo_set_matrix(_cr, &_stage_mat);
}

void
Renderer_cairo::end_display()
{
    cairo_restore(_cr);
}

void
Renderer_cairo::set_scale(float xscale, float yscale)
{
    _stage_mat.xx = xscale / 20;
    _stage_mat.yy = yscale / 20;
}

void
Renderer_cairo::set_translation(float xoff, float yoff)
{
    _stage_mat.x0 = xoff;
    _stage_mat.y0 = yoff;
}
    
void
Renderer_cairo::drawLine(const std::vector<point>& coords,
                              const rgba& color, const SWFMatrix& mat)
{
    if (coords.empty()) return;

    CairoScopeMatrix mat_transformer(_cr, mat);

    std::vector<point>::const_iterator i = coords.begin();
    
    double x = i->x, y = i->y;
    snap_to_half_pixel(_cr, x, y);

    cairo_move_to(_cr, x, y);

    for (std::vector<point>::const_iterator e = coords.end(); i != e; ++i) {
        double x = i->x, y = i->y;
        snap_to_half_pixel(_cr, x, y);
        cairo_line_to(_cr, x, y);
    }

    set_color(color);
    cairo_set_line_cap(_cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_join(_cr, CAIRO_LINE_JOIN_ROUND);

    double hwidth = 1.0;

    cairo_device_to_user_distance(_cr, &hwidth, &hwidth);
    cairo_set_line_width(_cr, hwidth);

    cairo_stroke(_cr);
}
  
void
Renderer_cairo::draw_poly(const std::vector<point>& corners, 
                               const rgba& fill, const rgba& outline,
                               const SWFMatrix& mat, bool /*masked*/)
{
    CairoScopeMatrix mat_transformer(_cr, mat);
    cairo_transform(_cr, &_stage_mat);

    if (corners.empty()) return;
        
    cairo_move_to(_cr, corners[0].x, corners[0].y);
    
    for (size_t i = 0; i < corners.size(); ++i) {
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
        
        cairo_set_line_width(_cr, 1.0);
        cairo_stroke_preserve(_cr);    
    }
    
    // Clear the current path which was _preserve()d.
    cairo_new_path(_cr);    
}

void
Renderer_cairo::set_antialiased(bool /*enable*/)
{
    log_unimpl("set_antialiased");
}
    
void
Renderer_cairo::begin_submit_mask()
{
    PathVec mask;
    _masks.push_back(mask);
    
    _drawing_mask = true;
}
  
void
Renderer_cairo::end_submit_mask()
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
  
void
Renderer_cairo::disable_mask()
{
    // Restore the previous clip.
    cairo_restore(_cr);

    _masks.pop_back();
}

void
Renderer_cairo::add_path(cairo_t* cr, const Path& cur_path)
{
    double x = cur_path.ap.x;
    double y = cur_path.ap.y;
    
    snap_to_half_pixel(cr, x, y);
    cairo_move_to(cr, x, y);
    
    for (std::vector<Edge>::const_iterator it = cur_path.m_edges.begin(),
         end = cur_path.m_edges.end(); it != end; ++it) {
        const Edge& cur_edge = *it;
      
        if (cur_edge.straight()) {
            x = cur_edge.ap.x;
            y = cur_edge.ap.y;
            snap_to_half_pixel(cr, x, y);
            cairo_line_to(cr, x, y);
        } else {
            // Cairo expects a cubic Bezier curve, while Flash gives us a
            // quadratic one. We must apply a conversion:
            
            const float two_thirds = 2.0/3.0;
            const float one_third = 1 - two_thirds;
            
            double x1 = x + two_thirds * (cur_edge.cp.x - x);
            double y1 = y + two_thirds * (cur_edge.cp.y - y);
            
            double x2 = cur_edge.cp.x
                           + one_third * (cur_edge.ap.x - cur_edge.cp.x);
            double y2 = cur_edge.cp.y
                           + one_third * (cur_edge.ap.y - cur_edge.cp.y);
            
            x = cur_edge.ap.x;
            y = cur_edge.ap.y;
 
            snap_to_half_pixel(cr, x1, y1);
            snap_to_half_pixel(cr, x2, y2);
            snap_to_half_pixel(cr, x, y);    

            cairo_curve_to(cr, x1, y1, x2, y2, x, y);
        }
    }
}

void
Renderer_cairo::apply_line_style(const LineStyle& style, const SWFCxForm& cx,
                                 const SWFMatrix& /*mat*/)
{
    cairo_line_join_t join_style = CAIRO_LINE_JOIN_MITER;
    switch (style.joinStyle()) {
        case JOIN_ROUND:
            join_style = CAIRO_LINE_JOIN_ROUND;
            break;
        case JOIN_BEVEL:
            join_style = CAIRO_LINE_JOIN_BEVEL;
            break;
        case JOIN_MITER:
            break;
        default:
          log_unimpl("join style");
    }
    cairo_set_line_join(_cr, join_style);

    if (style.startCapStyle() != style.endCapStyle()) {
        log_unimpl("differing start and end cap styles");
    }

    cairo_line_cap_t cap_style = CAIRO_LINE_CAP_ROUND;
    switch(style.startCapStyle()) {
        case CAP_ROUND:
            break;
        case CAP_NONE:
            cap_style = CAIRO_LINE_CAP_BUTT;
            break;
        case CAP_SQUARE:
            cap_style = CAIRO_LINE_CAP_SQUARE;
            break;
        default:
            log_unimpl("cap style");
    }

    cairo_set_line_cap(_cr, cap_style);

    // TODO: test that this is correct.
    cairo_set_miter_limit(_cr, style.miterLimitFactor());

    float width = style.getThickness();

    if ( width == 0.0 ) {

        cairo_matrix_t inv_stage = _stage_mat;
        cairo_matrix_invert(&inv_stage);

        double xconv = 1.0;
        double yconv = 1.0;

        cairo_matrix_transform_distance(&inv_stage, &xconv, &yconv);

        cairo_set_line_width(_cr, xconv);
    } else {
        // TODO: this is correct for !style.scaleThicknessVertically() 
        //       and !style.scaleThicknessHorizontally().
        //       If that's not the case, we should scale the thickness
        //       togheter with the shapes.
        if (style.scaleThicknessVertically() ||
            style.scaleThicknessHorizontally()) {
            LOG_ONCE( log_unimpl(_("Scaled strokes in Cairo renderer")) );
        }

        cairo_set_line_width(_cr, width);
    }

    rgba color = cx.transform(style.get_color());
    set_color(color);
}
  
void
Renderer_cairo::draw_outlines(const PathVec& path_vec,
                              const std::vector<LineStyle>& line_styles,
                              const SWFCxForm& cx,
                              const SWFMatrix& mat)
{
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
        const Path& cur_path = *it;

        if (!cur_path.m_line) {
            continue;
        }
      
        apply_line_style(line_styles[cur_path.m_line-1], cx, mat);
        add_path(_cr, cur_path);
        cairo_stroke(_cr);
    }  
}

void
Renderer_cairo::draw_subshape(const PathVec& path_vec, const SWFMatrix& mat,
                              const SWFCxForm& cx,
                              const std::vector<FillStyle>& FillStyles,
                              const std::vector<LineStyle>& line_styles)
{ 
    CairoPathRunner runner(*this, path_vec, FillStyles, _cr);
    runner.run(cx, mat);

    draw_outlines(path_vec, line_styles, cx, mat);
}


std::vector<PathVec::const_iterator>
Renderer_cairo::find_subshapes(const PathVec& path_vec)
{
    std::vector<PathVec::const_iterator> subshapes;
    
    PathVec::const_iterator it = path_vec.begin();
    PathVec::const_iterator end = path_vec.end();
    
    subshapes.push_back(it);
    ++it;

    for (;it != end; ++it) {
        const Path& cur_path = *it;
  
        if (cur_path.m_new_shape) {
            subshapes.push_back(it); 
        }  
    } 
  
    subshapes.push_back(end);
    
    return subshapes;
}

void
Renderer_cairo::draw_mask(const PathVec& path_vec)
{    
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
        const Path& cur_path = *it;
      
        if (cur_path.m_fill0 || cur_path.m_fill1) {
            _masks.back().push_back(cur_path);     
        }
    }  
}
  
void
Renderer_cairo::add_paths(const PathVec& path_vec)
{
    for (PathVec::const_iterator it = path_vec.begin(), end = path_vec.end();
         it != end; ++it) {
        const Path& cur_path = *it;
        
        add_path(_cr, cur_path);
    }  
}

  /// Takes a path and translates it using the given SWFMatrix.
void
Renderer_cairo::apply_matrix_to_paths(std::vector<Path>& paths,
                                      const SWFMatrix& mat)
{  
    std::for_each(paths.begin(), paths.end(),
                  boost::bind(&Path::transform, _1, boost::ref(mat)));
}
  
void
Renderer_cairo::drawShape(const SWF::ShapeRecord& shape, const Transform& xform)
{
    const PathVec& path_vec = shape.paths();
    
    if (!path_vec.size()) {
        return;    
    }
    
    cairo_set_fill_rule(_cr, CAIRO_FILL_RULE_EVEN_ODD); // TODO: Move to init
        
    if (_drawing_mask) {      
        PathVec scaled_path_vec = path_vec;
        
        apply_matrix_to_paths(scaled_path_vec, xform.matrix);
        draw_mask(scaled_path_vec); 
        return;
    }
    
    CairoScopeMatrix mat_transformer(_cr, xform.matrix);

    std::vector<PathVec::const_iterator> subshapes = find_subshapes(path_vec);
    
    const std::vector<FillStyle>& FillStyles = shape.fillStyles();
    const std::vector<LineStyle>& line_styles = shape.lineStyles();

    for (size_t i = 0; i < subshapes.size()-1; ++i) {
        PathVec subshape_paths;
        
        if (subshapes[i] != subshapes[i+1]) {
            subshape_paths = PathVec(subshapes[i], subshapes[i+1]);
        } else {
            subshape_paths.push_back(*subshapes[i]);
        }
        
        draw_subshape(subshape_paths, xform.matrix, xform.colorTransform,
                FillStyles, line_styles);
    }
}
  
void
Renderer_cairo::drawGlyph(const SWF::ShapeRecord& rec, const rgba& color,
                          const SWFMatrix& mat)
{
    SWFCxForm dummy_cx;
    std::vector<FillStyle> glyph_fs;
    
    FillStyle coloring = FillStyle(SolidFill(color));
    
    glyph_fs.push_back(coloring);
    
    const PathVec& path_vec = rec.paths();
    
    std::vector<LineStyle> dummy_ls;
    
    CairoScopeMatrix mat_transformer(_cr, mat);
    
    draw_subshape(path_vec, mat, dummy_cx, glyph_fs, dummy_ls);
}

void
Renderer_cairo::set_context(cairo_t* context)
{
    if (context == _cr) {
        return;    
    }

    cairo_destroy(_cr);
    _cr = context;
}
  
bool
Renderer_cairo::initTestBuffer(unsigned width, unsigned height)
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
  
unsigned int
Renderer_cairo::getBitsPerPixel() const
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
  
bool
Renderer_cairo::getPixel(rgba& color_return, int x, int y) const
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

namespace {

void
pattern_add_color_stops(const GradientFill& f, cairo_pattern_t* pattern,
                        const SWFCxForm& cx)
{      
    for (size_t index = 0; index < f.recordCount(); ++index) {
        const GradientRecord& grad = f.record(index);
        
        rgba c = cx.transform(grad.color);

        cairo_pattern_add_color_stop_rgba (pattern,
            grad.ratio / 255.0, c.m_r / 255.0, c.m_g / 255.0,
            c.m_b / 255.0, c.m_a / 255.0);
    }
}

void
init_cairo_matrix(cairo_matrix_t* cairo_matrix, const SWFMatrix& gnash_matrix)
{
    cairo_matrix_init(cairo_matrix,
              gnash_matrix.a()/65536.0, gnash_matrix.b()/65536.0,
              gnash_matrix.c()/65536.0, gnash_matrix.d()/65536.0,
              gnash_matrix.tx(), gnash_matrix.ty());
}


}


namespace renderer {
namespace cairo {

DSOEXPORT Renderer*
create_handler()
// Factory.
{
	return new Renderer_cairo();
}

DSOEXPORT void
set_context(Renderer* handler, cairo_t* context)
{
  Renderer_cairo* cairo_handler = static_cast<Renderer_cairo*>(handler);
  cairo_handler->set_context(context);
}


} // namespace cairo
} // namespace renderer
} // namespace gnash

