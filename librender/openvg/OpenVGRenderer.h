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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifndef GNASH_RENDER_HANDLER_OVG_H
#define GNASH_RENDER_HANDLER_OVG_H

#include <EGL/egl.h>
#include <vector>
#include <list>
#include <boost/scoped_array.hpp>
#include <boost/scoped_ptr.hpp>

#include "Geometry.h"
#include "Renderer.h"
//#include "directfb/DirectFBDevice.h"
#include "GnashDevice.h"
#include "CachedBitmap.h"
#include "FillStyle.h"

#include <VG/vgu.h>
#ifdef HAVE_VG_EXT_H
# include <VG/ext.h>
#else
# ifdef HAVE_VG_VGEXT_H
#  include <VG/vgext.h>
# endif
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

    void world_to_pixel(int& x, int& y, float world_x, float world_y) const;
    gnash::geometry::Range2d<int> world_to_pixel(const gnash::SWFRect& wb) const;
    gnash::geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& wb) const;
    
    gnash::point pixel_to_world(int, int) const;

    // Called by movie_root::display()
    void begin_display(const gnash::rgba&, int, int, float,
                       float, float, float);
    void end_display();
    void drawLine(const std::vector<point>& coords, const rgba& fill,
                  const SWFMatrix& mat);
    void drawVideoFrame(image::GnashImage* frame, const SWFMatrix *m,
                   const SWFRect* bounds, bool smooth);
    void draw_poly(const std::vector<point>& corners,
                  const rgba& fill, const rgba& outline,
                  const SWFMatrix& mat, bool masked);
    // this is in master
    void drawShape(const gnash::SWF::ShapeRecord&, const gnash::Transform&);
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

    // These weren't in the patch, and do nothing anyway
    Renderer *startInternalRender(gnash::image::GnashImage&);
    void endInternalRender();

    unsigned int getBitsPerPixel();

    void setFillPaint(const VGPaint paint) { _fillpaint = paint; }

    // These methods are only for debugging and development
    void printVGParams();
    void printVGHardware();
    static void printVGPath(VGPath path);
    static void printVGMatrix(VGfloat *mat);
    static void printVGMatrix(const SWFMatrix &mat);

    static const char *getErrorString(VGErrorCode error);

    // VGImage (CachedBitmap *x) { return _image_cache[x]; };
  private:
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
    void draw_mask(const PathVec& path_vec);    
    void draw_submask(const PathVec& path_vec, const SWFMatrix& mat,
                      const SWFCxForm& cx, const FillStyle& f_style);

    float       _xscale;
    float       _yscale;
    float       _width; // Width of the movie, in world coordinates.
    float       _height;
  
    // Output size.
    float       _display_width;
    float       _display_height;
  
    std::vector<PathVec> _masks;
    bool        _drawing_mask;
#ifdef OPENVG_VERSION_1_1
    VGMaskLayer _mask_layer;
#endif
    gnash::SWFMatrix stage_matrix;  // conversion from TWIPS to pixels
    
    /// this paint object is used for solid, gradient, and pattern fills.
    VGPaint     _fillpaint;

    /// this pain object is used for paths
    VGPaint     _strokepaint;

    /// This stores the Aspect Ratio, which is required to properly
    /// set the X axis scale. This is usually represented as 4:3 or
    /// 16:9 for most framebuffers. To get the scale, divide the 2nd
    /// argument by the first, ie... 4:3 = 3/4 = 0.75.
    /// The official calculation for the aspect ratio for a monitor of
    /// 800x480 returns a 5:3 aspect ratio, which for a WVGA LCD
    /// touchscreen is wrong. The manufacturer specs say it's a 4:3 or
    /// 16:9, so we use 4:3, which in my testing is correct.
    double      _aspect_ratio;
    
    // FIXME: A cache for the VGImages might make some sense assuming
    // it takes more time to render the cached GnashImage to a
    // VGImage. Right now every til a fill style is applied, the
    // VGImage is rebuilt from the GnashImage. This appears to be
    // relatively fast, and possibly faster than the lookup in a map
    // as the size of the cache grows. The other issue would be when
    // to clear the cache, as we have no knowledge from the VM when
    // images are deleted, or unused.
    // std::map<CachedBitmap *, VGImage > _image_cache;
};

DSOEXPORT Renderer* create_handler(const char *pixelformat);

} // namespace gnash::renderer::openvg
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_OVG_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
