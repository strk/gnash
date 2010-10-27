// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>

// FIXME: this should be a command line option
#undef GTK_TEST_RENDER

#ifdef GTK_TEST_RENDER
# include <gtk/gtk.h>
# include <gdk/gdk.h>
#endif

#include "log.h"
#include "dejagnu.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"

#ifdef RENDERER_AGG
#include "Renderer_agg.h"
#endif
#ifdef RENDERER_OPENGL
#include "Renderer_ogl.h"
#endif
#ifdef RENDERER_OPENVG
#include "Renderer_ovg.h"
#endif
#ifdef RENDERER_GLES1
#include "Renderer_gles1.h"
#endif
#ifdef RENDERER_GLES2
#include "Renderer_gles2.h"
#endif
#ifdef RENDERER_CAIRO
#include "Renderer_cairo.h"
#endif

TestState runtest;

using namespace gnash; 
using namespace std; 

void test_renderer(Renderer *renderer, const std::string &type);
void test_geometry(Renderer *renderer, const std::string &type);
void test_iterators(Renderer *renderer, const std::string &type);

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    const char *pixelformat = "RGB24";
    
#ifdef GTK_TEST_RENDER
    // FIXME: GTK specific!
    GtkWidget *drawing_area = 0;
    if (!drawing_area) {
        log_error("No GDK drawing area!");
        exit(-1);
    }
    
    GdkVisual *wvisual = gdk_drawable_get_visual(drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);
    const GdkVisual* visual = tmpimage->visual;

    // FIXME: we use bpp instead of depth, because depth doesn't appear to
    // include the padding byte(s) the GdkImage actually has.
    pixelformat = agg_detect_pixel_format(
        visual->red_shift, visual->red_prec,
        visual->green_shift, visual->green_prec,
        visual->blue_shift, visual->blue_prec,
        tmpimage->bpp * 8);
#endif  // end of GTK_TEST_RENDER

    Renderer *renderer = 0;
    
#ifdef RENDERER_AGG
    renderer = create_Renderer_agg(pixelformat);
    test_renderer(renderer, "AGG");
    test_geometry(renderer, "AGG");
    test_iterators(renderer, "AGG");
#endif

#ifdef RENDERER_OPENVG
    renderer = renderer::openvg::create_handler(pixelformat);
    if (renderer) {
        test_renderer(renderer, "OpenVG");
        test_geometry(renderer, "OpenVG");
        test_iterators(renderer, "OpenVG");
    } else {
        cerr << "ERROR: No OpenVG renderer to test!" << endl;
    }
#endif
    
#ifdef RENDERER_OPENGL
    renderer = renderer::opengl::create_handler(true);
    if (renderer) {
        test_renderer(renderer, "OpenGL");
        test_geometry(renderer, "OpenGL");
        test_iterators(renderer, "OpenGL");
    } else {
        cerr << "ERROR: No OpenGL renderer to test!" << endl;
    }
#endif
    
#ifdef RENDERER_GLES1
    renderer = renderer::gles1::create_handler(pixelformat);
    if (renderer) {
        test_renderer(renderer, "OpenGLES1");
        test_geometry(renderer, "OpenGLES1");
        test_iterators(renderer, "OpenGLES1");
    } else {
        cerr << "ERROR: No OpenGLES1 renderer to test!" << endl;
    }
#endif

#ifdef RENDERER_GLES2
    renderer =  renderer::gles2::create_handler(pixelformat);
    if (renderer) {
        test_renderer(renderer, "OpenGLES2");
        test_geometry(renderer, "OpenGLES2");
        test_iterators(renderer, "OpenGLES2");
    } else {
        cerr << "ERROR: No OpenGLES2 renderer to test!" << endl;
    }
#endif

#ifdef RENDERER_CAIRO
    renderer = renderer::cairo::create_handler();
    if (renderer) {
        test_renderer(renderer, "Cairo");
        test_geometry(renderer, "Cairo");
        test_iterators(renderer, "Cairo");
    } else {
        cerr << "ERROR: No Cairo renderer to test!" << endl;
    }
#endif

}

void
test_renderer(Renderer *renderer, const std::string &type)
{
    cout << "Testing " << type << " Renderer" << endl;
    
    if (!renderer) {
        runtest.unresolved("No renderer to test!");
        return;
    }
    
    if (renderer > 0) {
        runtest.pass("Got Renderer");
    } else {
        runtest.fail("Couldn't get Renderer");
    }

    if (!renderer->description().empty()) {
        if (renderer->description() == type) {
            runtest.pass("description is correct");
        } else {
            runtest.fail("description is wrong");
        }
    } else {
        runtest.fail("Couldn't get description!");
    }
    
    if (renderer->getBitsPerPixel()) {
        runtest.pass("getBitsPerPixel()");
    } else {
        runtest.fail("getBitsPerPixel()");
    }

    // Initializes the renderer for off-screen rendering used by the testsuite.
    if (renderer->initTestBuffer(10, 10)) {
        runtest.pass("initTestBuffer()");
    } else {
        runtest.fail("initTestBuffer()");
    }
    
    /// @coords an array of 16-bit signed integer coordinates. Even indices
    ///         (and 0) are x coordinates, while uneven ones are y coordinates.
    /// @vertex_count the number of x-y coordinates (vertices).
    /// @color the color to be used to draw the line strip.
    /// @mat the SWFMatrix to be used to transform the vertices.
    boost::uint16_t x = 10;
    boost::uint16_t y = 10;
    boost::uint16_t h = 10;
    std::vector<point> box = boost::assign::list_of
        (point(x, y))
        (point(x, y + h));

    rgba color(0, 0, 0, 255);
    SWFMatrix mat;
    mat.set_scale_rotation(1, 3, 0);

    renderer->drawLine(box, color, mat);
    // if (1) {
    //     runtest.pass("drawLine()");
    // } else {
    //     runtest.fail("drawLine()");
    // }
    runtest.unresolved("drawLine()");

//    drawVideoFrame(image::GnashImage* frame, const Transform& xform, const SWFRect* bounds, bool smooth);
    image::GnashImage *frame;
    const Transform xform;
    const SWFRect bounds;
    bool smooth;
//    renderer->drawVideoFrame(frame, xform, bounds, smooth);
    runtest.unresolved("drawVideoFrame()");

    point *corners = 0;
    size_t corner_count = 0;
    rgba fill(0, 0, 0, 255);;
    rgba outline(0, 0, 0, 255);;
    bool masked = true;
    renderer->drawPoly(corners, corner_count, fill, outline, mat, masked);
    runtest.unresolved("draw_poly()");
    
//    SWF::ShapeRecord shape;
    // Transform xform;
//    renderer->drawShape(shape, xform);
    runtest.unresolved("drawShape()");

//    SWF::ShapeRecord rec;
    // rgba color;
    // SWFMatrix mat;
//    renderer->drawGlyph(rec, color, mat);
    runtest.unresolved("drawGlyph()");
 
    boost::shared_ptr<IOChannel> io;
    FileType ftype;
//    renderer->renderToImage(io, ftype);
    runtest.unresolved("renderToImage()");

    CachedBitmap *bitmap = 0;
    image::GnashImage *frame2 = new image::ImageRGBA(10, 10);
    std::auto_ptr<image::GnashImage> im(frame2);
    bitmap = renderer->createCachedBitmap(im);
    if (bitmap) {
        runtest.pass("createCachedBitmap()");
    } else {
        runtest.fail("createCachedBitmap()");
    }
    runtest.unresolved("createCachedBitmap()");
    
}
void
test_geometry(Renderer *renderer, const std::string &type)
{
    cout << "\t" << type << " geometry tests" << endl;
    
    if (!renderer) {
        runtest.unresolved("No renderer to test!");
        return;
    }
    
    int x = 10;
    int y = 10;
    geometry::Point2d nz(200, 0);
    geometry::Point2d z(x, y);
    geometry::Range2d<int> pixelbounds;
    geometry::Range2d<int> worldbounds;
    z = renderer->pixel_to_world(x, y);
    if ((z.x >= 199) || (z.y >= 199)) {
        runtest.pass("pixel_to_world(int, int)");
    } else {
        runtest.fail("pixel_to_world(int, int)");
    }
//    worldbounds = renderer->pixel_to_world(pixelbounds);
    if (worldbounds.isNull()) {
        runtest.pass("pixel_to_world(geometry::Range2d<int>)");
    } else {
        runtest.fail("pixel_to_world(geometry::Range2d<int>)");
    }
    
    pixelbounds = renderer->world_to_pixel(worldbounds);
    if (pixelbounds.isNull()) {
        runtest.pass("world_to_pixel(geometry::Range2d<int>)");
    } else {
        runtest.fail("world_to_pixel(geometry::Range2d<int>)");
    }
    
    SWFRect bounds;
    if (!renderer->bounds_in_clipping_area(bounds)) {
        runtest.pass("bounds_in_clipping_area(SWFRect)");
    } else {
        runtest.fail("bounds_in_clipping_area(SWFRect)");
    }
    
    InvalidatedRanges ranges;
    if (!renderer->bounds_in_clipping_area(ranges)) {
        runtest.pass("bounds_in_clipping_area(InvalidatedRanges)");
    } else {
        runtest.fail("bounds_in_clipping_area(InvalidatedRanges)");
    }

    if (!renderer->bounds_in_clipping_area(pixelbounds)) {
        runtest.pass("bounds_in_clipping_area(geometry::Range2d<int>)");
    } else {
        runtest.fail("bounds_in_clipping_area(geometry::Range2d<int>)");
    }
}

//
// Machinery for delayed images rendering (e.g. Xv with YV12 or VAAPI)
//
void
test_iterators(Renderer *renderer, const std::string &type)
{
    cout << "\t"<< type << " iterator tests" << endl;
    
    if (!renderer) {
        runtest.unresolved("No renderer to test!");
        return;
    }

    Renderer::RenderImages::iterator fit = renderer->getFirstRenderImage();
    Renderer::RenderImages::iterator lit = renderer->getLastRenderImage();
    // When there are no images, the first and last are the same obviously
    if (fit == lit) {
         runtest.pass("getFirstRenderImage()");
    } else {
        runtest.fail("getFirstRenderImage()");
    }

    
    geometry::Point2d a(1, 2);
    geometry::Point2d c(3, 4);
    Renderer::RenderImage image;
    image::GnashImage *frame = new image::ImageRGBA(10, 10);
    gnash::GnashVaapiImage *foo = static_cast<gnash::GnashVaapiImage *>(frame);
    // gnash::GnashVaapiImageProxy *bar = new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y);
    std::auto_ptr<image::GnashImage> rgba(frame);
//    image.reset(new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y));

    renderer->addRenderImage(image);
//    image.reset(new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y));
    renderer->addRenderImage(image);
    
    fit = renderer->getFirstRenderImage();
    lit = renderer->getLastRenderImage();
    // When there are no images, the first and last are the same obviously
    if (fit != lit) {
         runtest.pass("addRenderImage()");
    } else {
        runtest.fail("addRenderImage()");
    }
    
#if 0
    typedef boost::shared_ptr<GnashVaapiImageProxy> RenderImage;
    typedef std::vector<RenderImage> RenderImages;

    // Get first render image
    virtual RenderImages::iterator getFirstRenderImage()
            { return _render_images.begin(); }
    virtual RenderImages::const_iterator getFirstRenderImage() const
            { return _render_images.begin(); }

    // Get last render image
    virtual RenderImages::iterator getLastRenderImage()
            { return _render_images.end(); }
    virtual RenderImages::const_iterator getLastRenderImage() const
            { return _render_images.end(); }
#endif
}

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
