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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA

#ifndef GNASH_RENDER_HANDLER_DIRECTFB_H
#define GNASH_RENDER_HANDLER_DIRECTFB_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "Renderer.h"
#include "Geometry.h"

#include <map>
#include <vector>

#ifdef HAVE_DIRECTFB_H
# include <directfb/directfb.h>
#endif

namespace gnash {

class GnashImage;
class SWFCxForm;

namespace renderer {

namespace DirectFB {

class  DSOEXPORT Renderer_DirectFB: public Renderer
{
public:
    std::string description() const { return "DirectFB"; }
    Renderer_DirectFB();
    ~Renderer_DirectFB();
        
    void init(float x, float y);
    CachedBitmap *createCachedBitmap(std::unique_ptr<image::GnashImage> im);

    void world_to_pixel(int& x, int& y, float world_x, float world_y);
    gnash::geometry::Range2d<int> world_to_pixel(const gnash::SWFRect& wb);
    geometry::Range2d<int> world_to_pixel(const geometry::Range2d<float>& wb);
    gnash::point pixel_to_world(int, int);

    void begin_display(const gnash::rgba&, int, int, float,
                                        float, float, float);
    // This is from the patch
    // void begin_display(const rgba& bg_color, int viewport_x0,
    //                    int viewport_y0, int viewport_width,
    //                    int viewport_height, float x0, float x1,
    //                    float y0, float y1);
    void end_display();
    void drawLine(const std::vector<point>& coords, const rgba& fill,
                  const SWFMatrix& mat);
    void drawVideoFrame(gnash::image::GnashImage *frame, const gnash::Transform& tx,
                        const gnash::SWFRect *bounds, bool smooth);
    void drawPoly(const point* corners, size_t corner_count, 
                  const rgba& fill, const rgba& outline,
                  const SWFMatrix& mat, bool masked);
    void drawShape(const gnash::SWF::ShapeRecord&, const gnash::Transform&);
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

    // These methods are only for debugging and development
    void printVGParams();
    void printVGHardware();
    void printVGPath();
  private:
    unsigned char *_testBuffer; // buffer used by initTestBuffer() only
};    

DSOEXPORT Renderer* create_handler(const char *pixelformat);

} // namespace gnash::renderer::DirectFB
} // namespace gnash::renderer
} // namespace gnash

#endif // __RENDER_HANDLER_DIRECTFB_H__

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
