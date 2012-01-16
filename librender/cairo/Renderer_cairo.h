// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


#ifndef BACKEND_RENDER_HANDLER_CAIRO_H
#define BACKEND_RENDER_HANDLER_CAIRO_H

#include <vector>
#include <boost/scoped_array.hpp>
#include <cairo/cairo.h>
#include "Renderer.h"
#include "Geometry.h"

namespace gnash {
    class Transform;
    class SWFCxForm;
}

namespace gnash {

    typedef std::vector<Path> PathVec;
    typedef std::vector<const Path*> PathPtrVec;

class DSOEXPORT Renderer_cairo: public Renderer
{

public:
    Renderer_cairo();
    ~Renderer_cairo();

    std::string description() const { return "Cairo"; }

    CachedBitmap* createCachedBitmap(std::auto_ptr<image::GnashImage> im);

    void drawVideoFrame(image::GnashImage* baseframe, const Transform& xform,
                                const SWFRect* bounds, bool smooth);

    virtual geometry::Range2d<int> world_to_pixel(const SWFRect& worldbounds)
        const;

    virtual point pixel_to_world(int x, int y) const;

    void set_color(const rgba& c);

    void set_invalidated_regions(const InvalidatedRanges& ranges);
      
    virtual Renderer* startInternalRender(image::GnashImage& /*im*/) {
        return 0;
    }

    virtual void endInternalRender() {}

    virtual void begin_display(const rgba& bg_color,
                       int viewport_width, int viewport_height,
                       float x0, float x1, float y0, float y1);

    virtual void end_display();

    void set_scale(float xscale, float yscale);

    void set_translation(float xoff, float yoff);

    void drawLine(const std::vector<point>& coords, const rgba& color,
                          const SWFMatrix& mat);

    void draw_poly(const std::vector<point>& corners, 
                           const rgba& fill, const rgba& outline,
                           const SWFMatrix& mat, bool masked);

    void set_antialiased(bool enable);

    void begin_submit_mask();
    void end_submit_mask();
    void disable_mask();

    void add_path(cairo_t* cr, const Path& cur_path);

    void apply_line_style(const LineStyle& style, const SWFCxForm& cx,
                          const SWFMatrix& mat);

    void draw_outlines(const PathVec& path_vec,
                       const std::vector<LineStyle>& line_styles,
                       const SWFCxForm& cx,
                       const SWFMatrix& mat);

    std::vector<PathVec::const_iterator> find_subshapes(const PathVec& path_vec);

    void draw_subshape(const PathVec& path_vec,
                       const SWFMatrix& mat, const SWFCxForm& cx,
                       const std::vector<FillStyle>& FillStyles,
                       const std::vector<LineStyle>& line_styles);

    void draw_mask(const PathVec& path_vec);

    void add_paths(const PathVec& path_vec);

    void apply_matrix_to_paths(std::vector<Path>& paths, const SWFMatrix& mat);

    void drawShape(const SWF::ShapeRecord& shape, const Transform& xform);

    void drawGlyph(const SWF::ShapeRecord& rec, const rgba& color,
                   const SWFMatrix& mat);

    void set_context(cairo_t* context);

    unsigned int getBitsPerPixel() const;

    bool getPixel(rgba& color_return, int x, int y) const;

    bool initTestBuffer(unsigned width, unsigned height);

private:
    /// The cairo context.
    cairo_t* _cr;
    boost::scoped_array<boost::uint8_t> _video_buffer;
    std::vector<PathVec> _masks;
    size_t _video_bufsize;
    bool _drawing_mask;
    InvalidatedRanges _invalidated_ranges;
    cairo_matrix_t _stage_mat;
};



namespace renderer {

/// Cairo renderer namespace
namespace cairo {

/// Create a render handler
gnash::Renderer* create_handler();

/// Make sure to call this before starting display
void set_context(Renderer* handler, cairo_t* context);

} // namespace gnash::renderer::cairo
} // namespace gnash::renderer
} // namespace gnash

#endif // BACKEND_RENDER_HANDLER_CAIRO_H
