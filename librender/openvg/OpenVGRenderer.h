// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

///
/// Original Author: Visor <cutevisor@gmail.com>.
/// Heavily hacked by Rob <rob@welcomehome.org> to work with Gnash
/// git master.
///

#ifndef GNASH_RENDER_HANDLER_OVG_H
#define GNASH_RENDER_HANDLER_OVG_H

#include <EGL/egl.h>
#include <vector>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "Geometry.h"
#include "Renderer.h"
//#include "directfb/DirectFBDevice.h"
#include "GnashDevice.h"
#include "CachedBitmap.h"
#include "FillStyle.h"

#include <VG/vgu.h>
#ifdef OPENVG_VERSION_1_1
# include <VG/ext.h>
#else
# include <VG/vgext.h>
#endif
#include <VG/openvg.h>
#include "openvg/OpenVGBitmap.h"
#include "egl/eglDevice.h"

namespace gnash {

class SWFCxForm;
class GnashImage;

namespace renderer {

namespace openvg {

typedef std::vector<const Path*> PathRefs;
typedef std::vector<Path> PathVec;
typedef std::vector<geometry::Range2d<int> > ClipBounds;
typedef std::vector<const Path*> PathPtrVec;

// typedef std::map<const Path*, VGPath > PathPointMap;

class  DSOEXPORT Renderer_ovg: public Renderer
{
public:
    std::string description() const { return "OpenVG"; }

    Renderer_ovg();
    Renderer_ovg(renderer::GnashDevice::dtype_t dtype);
    
    ~Renderer_ovg();
        
    void init(float x, float y);
    CachedBitmap *createCachedBitmap(std::auto_ptr<image::GnashImage> im);

    void drawVideoFrame(gnash::image::GnashImage*, const gnash::Transform&,
                        const gnash::SWFRect*, bool);

    void world_to_pixel(int& x, int& y, float world_x, float world_y);
    gnash::geometry::Range2d<int> world_to_pixel(const gnash::SWFRect& wb);
    geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& wb);
    gnash::point pixel_to_world(int, int);

    // this is in master
    void begin_display(const gnash::rgba&, int, int, float,
                                        float, float, float);
    // This is from the patch
    void begin_display(const rgba& bg_color, int viewport_x0,
                       int viewport_y0, int viewport_width,
                       int viewport_height, float x0, float x1,
                       float y0, float y1);
    void end_display();
    void drawLine(const std::vector<point>& coords, const rgba& fill,
                  const SWFMatrix& mat);
    void drawVideoFrame(image::GnashImage* frame, const SWFMatrix *m,
                   const SWFRect* bounds, bool smooth);
    void drawPoly(const point* corners, size_t corner_count, 
                  const rgba& fill, const rgba& outline,
                  const SWFMatrix& mat, bool masked);
    // this is in master
    void drawShape(const gnash::SWF::ShapeRecord&,
                                    const gnash::Transform&);
    // This is from the patch
    void drawGlyph(const SWF::ShapeRecord& rec, const rgba& c,
                   const SWFMatrix& mat);

    void set_antialiased(bool enable);
    void begin_submit_mask();
    void end_submit_mask();
    void apply_mask();
    void disable_mask();
        
    void set_scale(float xscale, float yscale);
    void set_invalidated_regions(const InvalidatedRanges &ranges);

    // These weren't in the patch
    Renderer *startInternalRender(gnash::image::GnashImage&);
    void endInternalRender();

    unsigned int getBitsPerPixel();
    bool initTestBuffer(unsigned width, unsigned height);
    void init_buffer(unsigned char *mem, int size, int x, int y, int rowstride);

    // These methods are only for debugging and development
    void printVGParams();
    void printVGHardware();
    void printVGPath();
    
#if 0
    // These are all required by the Render class
    void draw_poly(const point* corners, size_t corner_count,
                           const rgba& fill, const rgba& outline,
                           const SWFMatrix& mat, bool masked);
    void drawVideoFrame(gnash::image::GnashImage*, const gnash::Transform&,
                        const gnash::SWFRect*, bool);

    void renderToImage(boost::shared_ptr<IOChannel> io,FileType type);
    void set_invalidated_regions(const InvalidatedRanges& ranges);
    
    void begin_submit_mask();
#endif
  private:
    void draw_mask(const PathVec& path_vec);
    void add_paths(const PathVec& path_vec);
    Path reverse_path(const Path& cur_path);
    const Path* find_connecting_path(const Path& to_connect,
                                     std::list<const Path*> path_refs);
    PathVec normalize_paths(const PathVec &paths);
    /// Analyzes a set of paths to detect real presence of fills and/or outlines
    /// TODO: This should be something the character tells us and should be 
    /// cached. 
    void analyze_paths(const PathVec &paths, bool& have_shape,
                       bool& have_outline);
    void apply_fill_style(const FillStyle& style, const SWFMatrix& /* mat */,
                          const SWFCxForm& cx);
    bool apply_line_style(const LineStyle& style, const SWFCxForm& cx,
                          const SWFMatrix& mat);
    void draw_outlines(const PathVec& path_vec, const SWFMatrix& mat,
                       const SWFCxForm& cx, const std::vector<
                       LineStyle>& line_styles);
    std::list<PathPtrVec> get_contours(const PathPtrVec &paths);
    PathPtrVec paths_by_style(const PathVec& path_vec, unsigned int style);
    std::vector<PathVec::const_iterator> find_subshapes(const PathVec& path_vec);
    void apply_matrix_to_paths(std::vector<Path>& paths, const SWFMatrix& mat);
    void draw_subshape(const PathVec& path_vec, const SWFMatrix& mat,
                       const SWFCxForm& cx,
                       const std::vector<FillStyle>& fill_styles,
                       const std::vector<LineStyle>& line_styles);
    void draw_submask(const PathVec& path_vec, const SWFMatrix& mat,
                      const SWFCxForm& cx, const FillStyle& f_style);
    
    float _xscale;
    float _yscale;
    float _width; // Width of the movie, in world coordinates.
    float _height;
  
    // Output size.
    float _display_width;
    float _display_height;
  
    std::vector<PathVec> _masks;
    bool _drawing_mask;
  
    gnash::SWFMatrix stage_matrix;  // conversion from TWIPS to pixels
    
    VGPaint     m_fillpaint;
    VGPaint     m_strokepaint;

#ifdef OPENVG_VERSION_1_1    
    VGMaskLayer m_mask;
#endif
    unsigned char *_testBuffer; // buffer used by initTestBuffer() only
    
    boost::scoped_ptr<renderer::GnashDevice> _device;
};

namespace {
    const CachedBitmap* createGradientBitmap(const GradientFill& gf,
                                  renderer::openvg::Renderer_ovg *renderer);
}

DSOEXPORT Renderer* create_handler(const char *pixelformat);

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_OVG_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
