// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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


#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#ifdef HAVE_GLES2_GL2_H
# include <GLES2/gl2.h>
# include <GLES2/gl2ext.h>
#endif

#include <cstring>
#include <cmath>

#include <smart_ptr.h>
#include "swf/ShapeRecord.h"
#include "RGBA.h"
#include "GnashImage.h"
#include "GnashTexture.h"
#include "GnashNumeric.h"
#include "log.h"
#include "utility.h"
#include "Range2d.h"
#include "SWFMatrix.h"
#include "swf/ShapeRecord.h"

#include "Renderer_gles2.h"

#include <boost/utility.hpp>
#include <boost/bind.hpp>

// Defined to 1 to disable (slow) anti-aliasing with the accumulation buffer
#define NO_ANTIALIASING 1

/// \file Renderer_gles.cpp
/// \brief The OpenGL-ES renderer and related code.
///

// TODO:
// - Profiling!
// - Optimize code:
// * Use display lists
// * Use better suited standard containers
// * convert to double at a later stage (oglVertex)
// * keep data for less time
// * implement hardware accelerated gradients. Most likely this will require
//   the use of fragment shader language.

// * The "Programming Tips" in the OpenGL "red book" discusses a coordinate system
// that would give "exact two-dimensional rasterization". AGG uses a similar
// system; consider the benefits and drawbacks of switching.

namespace gnash {

namespace renderer {

namespace gles2 {

Renderer_gles2::Renderer_gles2()
{
//    if (!initDevice(EGLDevice::OPENVG))
    if (!initDevice(0, 0)) {
        log_error("Couldn't initialize EGL Device!");
    }
}

Renderer_gles2::~Renderer_gles2()
{

}

void
Renderer_gles2::init(float x, float y)
{
    GNASH_REPORT_FUNCTION;
}

CachedBitmap *
Renderer_gles2::createCachedBitmap(std::auto_ptr<gnash::image::GnashImage>)
{
    return 0;
}

void
Renderer_gles2::drawVideoFrame(image::GnashImage* /* frame */,
                               const gnash::Transform& t/* m */,
                               const SWFRect* /* bounds */, bool /*smooth*/)
{
    log_unimpl("drawVideoFrame");  
}



void
Renderer_gles2::world_to_pixel(int& x, int& y, float world_x, float world_y)
{
#if 0
    // negative pixels seems ok here... we don't
    // clip to valid range, use world_to_pixel(rect&)
    // and Intersect() against valid range instead.
    point p(world_x, world_y);
    stage_matrix.transform(p);
    x = (int)p.x;
    y = (int)p.y;
#endif
}

geometry::Range2d<int>
Renderer_gles2::world_to_pixel(const SWFRect& wb)
{
    using namespace gnash::geometry;
    
    if ( wb.is_null() ) return Range2d<int>(nullRange);
    if ( wb.is_world() ) return Range2d<int>(worldRange);
    
    int xmin, ymin, xmax, ymax;
    
    world_to_pixel(xmin, ymin, wb.get_x_min(), wb.get_y_min());
    world_to_pixel(xmax, ymax, wb.get_x_max(), wb.get_y_max());
    
    return Range2d<int>(xmin, ymin, xmax, ymax);
}

geometry::Range2d<int>
Renderer_gles2::world_to_pixel(const geometry::Range2d<float>& wb)
{
    if (wb.isNull() || wb.isWorld()) return wb;
    
    int xmin, ymin, xmax, ymax;
    
    world_to_pixel(xmin, ymin, wb.getMinX(), wb.getMinY());
    world_to_pixel(xmax, ymax, wb.getMaxX(), wb.getMaxY());
    
    return geometry::Range2d<int>(xmin, ymin, xmax, ymax);
}

point
Renderer_gles2::pixel_to_world(int x, int y)
{
#if 0
    point p(x, y);
    SWFMatrix mat = stage_matrix;
    mat.invert().transform(p);
    return p;
#endif
};

void
Renderer_gles2::begin_display(const gnash::rgba&, int, int, float,
                                        float, float, float)
{
    GNASH_REPORT_FUNCTION;
}

void
Renderer_gles2::end_display()
{
    GNASH_REPORT_FUNCTION;
}

void
Renderer_gles2::drawLine(const std::vector<point>& coords, const rgba& fill,
                       const SWFMatrix& mat)
{
    GNASH_REPORT_FUNCTION;
}
void
Renderer_gles2::drawPoly(const point* corners, size_t corner_count, 
                       const rgba& fill, const rgba& /* outline */,
                       const SWFMatrix& mat, bool /* masked */)
{
    GNASH_REPORT_FUNCTION;
}
void
Renderer_gles2::drawShape(const gnash::SWF::ShapeRecord&, const gnash::Transform&)
{
    GNASH_REPORT_FUNCTION;
}
// void
// Renderer_gles2::drawShape(const SWF::ShapeRecord& shape, const SWFCxForm& cx,
//                         const SWFMatrix& mat)
// {
//     GNASH_REPORT_FUNCTION;
// }
void
Renderer_gles2::drawGlyph(const SWF::ShapeRecord& rec, const rgba& c,
                        const SWFMatrix& mat)
{
    GNASH_REPORT_FUNCTION;
}

void
Renderer_gles2::set_antialiased(bool /* enable */)
{
    log_unimpl("set_antialiased");
}

void
Renderer_gles2::begin_submit_mask()
{
#if 0
    PathVec mask;
    _masks.push_back(mask);
    
    _drawing_mask = true;
#endif
}

void
Renderer_gles2::end_submit_mask()
{
#if 0
    _drawing_mask = false;
    apply_mask();
#endif
}
void
Renderer_gles2::apply_mask()
{
    GNASH_REPORT_FUNCTION;
}
void
Renderer_gles2::disable_mask()
{
    GNASH_REPORT_FUNCTION;
}
void
Renderer_gles2::set_scale(float xscale, float yscale)
{
#if 0
    _xscale = xscale;
    _yscale = yscale;
    stage_matrix.set_identity();
    stage_matrix.set_scale(xscale/20.0f, yscale/20.0f);
#endif
}

void
Renderer_gles2::set_invalidated_regions(const InvalidatedRanges& /* ranges */)
{
}

bool
Renderer_gles2::initTestBuffer(unsigned int width, unsigned int height)
{
#if 0
    int size = width * height; // * getBytesPerPixel();
    
    _testBuffer = static_cast<unsigned char *>(realloc(_testBuffer, size));
    memset(_testBuffer, 0, size);
    printf("\tRenderer Test memory at: %p\n", _testBuffer);
#endif
//    init_buffer(_testBuffer, size, width, height, width * getBytesPerPixel());
//    init(width, height);
    
    return true;
}

Renderer *
Renderer_gles2::startInternalRender(gnash::image::GnashImage&)
{
}

void
Renderer_gles2::endInternalRender()
{
}

Renderer *
create_handler(const char *pixelformat)
{
  Renderer_gles2 *renderer = new Renderer_gles2;
  return renderer;

  return 0;
}  

} // namespace gnash::renderer::gles2
} // namespace gnash::renderer
} // end of gnash namespace

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
