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
#include <sstream>
#include <map>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>
#include <boost/date_time/date.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

// FIXME: this should be a command line option
#undef GTK_TEST_RENDER

#ifdef HAVE_GTK2
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
#include "boost/date_time/posix_time/posix_time.hpp"

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
#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
# include <eglDevice.h>
#else
# error "This file needs EGL"
#endif

TestState runtest;

using namespace gnash; 
using namespace renderer; 
using namespace std;
using namespace boost::posix_time;

void test_renderer(Renderer *renderer, const std::string &type);
void test_geometry(Renderer *renderer, const std::string &type);
void test_iterators(Renderer *renderer, const std::string &type);

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

#ifdef HAVE_GTK2
GtkWidget *create_GTK_window();
#endif

// Simple class to do nanosecond based timing for performance analysis
class Timer {
public:
    Timer(const std::string &name, bool flag)
        {
            _print = flag;
            _name = name;
            start();
        }
    Timer(const std::string &name) : _print(false) { _name = name; start(); }
    Timer() : _print(false) { start(); }
    ~Timer()
        {
            stop();
            if (_print) {
                cerr << "Total time for " << _name << " was: " << elapsed() << endl;
            }
        };
    
    void start() {
        _starttime = boost::posix_time::microsec_clock::local_time(); 
    }
    
    void stop() {
        _stoptime = boost::posix_time::microsec_clock::local_time(); 
    }

    std::string elapsed() {
        stringstream ss;
        time_duration td = _stoptime - _starttime;
        ss << td.total_nanoseconds() << "ns elapsed";
        return ss.str();
    }
    void setName(const std::string &name) { _name = name; };
    std::string &getName() { return _name; };
    
private:
    bool _print;
    std::string _name;
    boost::posix_time::ptime _starttime;
    boost::posix_time::ptime _stoptime;
};

int
main(int argc, char *argv[])
{
    // FIXME: for now, always run verbose till this supports command line args
    dbglogfile.setVerbosity();

    const char *pixelformat = "RGB24";

#ifdef GTK_TEST_RENDER
    GdkVisual *wvisual = gdk_drawable_get_visual(canvas);

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

#ifdef RENDERER_AGG
    Timer tagg("AGG");
    Renderer *renderer1 = create_Renderer_agg(pixelformat);
#ifdef HAVE_GTK2
    GtkWidget *canvas = create_GTK_window();
    GdkVisual* visual = gdk_drawable_get_visual(canvas->window);
    GdkImage *offscreenbuf = gdk_image_new (GDK_IMAGE_FASTEST, visual, 200,200);
    static_cast<Renderer_agg_base *>(renderer1)->init_buffer(
        (unsigned char*) offscreenbuf->mem,
        offscreenbuf->bpl * offscreenbuf->height,
        offscreenbuf->width,
        offscreenbuf->height,
        offscreenbuf->bpl);
#endif
    
    if (renderer1) {
        test_renderer(renderer1, "AGG");
        test_geometry(renderer1, "AGG");
        test_iterators(renderer1, "AGG");
        tagg.stop();
    }
    cerr << "AGG tests took " << tagg.elapsed() << endl << endl;
#endif

#ifdef RENDERER_OPENVG
    Timer tovg("OpenVG");
    Renderer *renderer2 = renderer::openvg::create_handler(pixelformat);
    EGLDevice *ovg = dynamic_cast<EGLDevice *>(renderer2);
    ovg->initDevice(EGLDevice::OPENVG);
    ovg->initEGL(*(reinterpret_cast<EGLNativeWindowType *>(canvas)));
    if (renderer2) {
        test_renderer(renderer2, "OpenVG");
        test_geometry(renderer2, "OpenVG");
        test_iterators(renderer2, "OpenVG");
    } else {
        cerr << "ERROR: No OpenVG renderer to test!" << endl;
    }
    tovg.stop();
    cerr << "OpenVG tests took " << tovg.elapsed() << endl;
#endif
    
#if 1
#ifdef RENDERER_GLES1
    Timer tgles1("OpenGLES1");
    Renderer *renderer3 = renderer::gles1::create_handler(pixelformat);
    if (renderer3) {
        test_renderer(renderer3, "OpenGLES1");
        test_geometry(renderer3, "OpenGLES1");
        test_iterators(renderer3, "OpenGLES1");
    } else {
        cerr << "ERROR: No OpenGLES1 renderer to test!" << endl;
    }
    tgles1.stop();
    cerr << "OpenGLES1 tests took " << tgles1.elapsed() << endl;
#endif

#ifdef RENDERER_GLES2
    Timer tgles2("OpenGLES2");
    Renderer *renderer4 = renderer::gles2::create_handler(pixelformat);
    if (renderer4) {
        test_renderer(renderer4, "OpenGLES2");
        test_geometry(renderer4, "OpenGLES2");
        test_iterators(renderer4, "OpenGLES2");
    } else {
        cerr << "ERROR: No OpenGLES2 renderer to test!" << endl;
    }
    tgles2.stop();
    cerr << "OpenGLES2 tests took " << tgles2.elapsed() << endl;
#endif

#ifdef RENDERER_CAIRO 
    Timer tcairo("Cairo");
    Renderer *renderer5 = renderer::cairo::create_handler();
    if (renderer5) {
        test_renderer(renderer5, "Cairo");
        test_geometry(renderer5, "Cairo");
        test_iterators(renderer5, "Cairo");
    } else {
        cerr << "ERROR: No Cairo renderer to test!" << endl;
    }
    tcairo.stop();
    cerr << "Cairo tests took " << tcairo.elapsed() << endl;
#endif
    
#ifdef RENDERER_OPENGL
    Timer tgl("OpenGL");
    Renderer *renderer6 = renderer::opengl::create_handler(true);
    if (renderer6) {
        test_renderer(renderer6, "OpenGL");
        test_geometry(renderer6, "OpenGL");
        test_iterators(renderer6, "OpenGL");
    } else {
        cerr << "ERROR: No OpenGL renderer to test!" << endl;
    }
    tgl.stop();
    cerr << "OpenGL tests took " << tgl.elapsed() << endl;
#endif
#endif
    
#ifdef HAVE_GTK2
    gtk_main();
    gtk_main_quit();
    gtk_exit(0);
#endif
}

void
test_renderer(Renderer *renderer, const std::string &type)
{
    cout << "Testing " << type << " Renderer" << endl;
//    Timer trend("Renderer Tests", true);
    
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

#ifdef HAVE_GTK2
    GtkWidget *canvas = create_GTK_window();
    if (type == "AGG") {
        GdkVisual* visual = gdk_drawable_get_visual(canvas->window);
        GdkImage *offscreenbuf = gdk_image_new (GDK_IMAGE_FASTEST, visual, 200,
                                                200);;
        static_cast<Renderer_agg_base *>(renderer)->init_buffer(
            (unsigned char*) offscreenbuf->mem,
            offscreenbuf->bpl * offscreenbuf->height,
            offscreenbuf->width,
            offscreenbuf->height,
            offscreenbuf->bpl);    
    }
            
    if ((type == "OpenVG") && (type == "OpenGLES1") && (type == "OpenGLES2")) {
        EGLDevice *ovg = dynamic_cast<EGLDevice *>(renderer);
        ovg->initDevice(EGLDevice::OPENVG);
        ovg->initEGL(*(reinterpret_cast<EGLNativeWindowType *>(canvas)));
    }
#else
    // Initializes the renderer for off-screen rendering used by the testsuite.
    if (renderer->initTestBuffer(10, 10)) {
        runtest.pass("initTestBuffer()");
    } else {
        runtest.fail("initTestBuffer()");
    }
#endif
    
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

    Timer tdrawline("drawline");
    renderer->drawLine(box, color, mat);
    tdrawline.stop();
    
    // if (1) {
    //     runtest.pass("drawLine()");
    // } else {
    //     runtest.fail("drawLine()");
    // }
    runtest.unresolved(std::string("drawLine() ") + tdrawline.elapsed());

    //drawVideoFrame(image::GnashImage* frame, const Transform& xform, const SWFRect* bounds, bool smooth);
#if 0
    image::GnashImage *frame;
    const Transform xform;
    const SWFRect bounds;
    bool smooth;
    Timer tdrawvideo("drawVideoFrame");
    renderer->drawVideoFrame(frame, xform, bounds, smooth);
    tdrawvideo.stop();
#endif
    runtest.untested("drawVideoFrame()");

    point *corners = 0;
    size_t corner_count = 0;
    rgba fill(0, 0, 0, 255);;
    rgba outline(0, 0, 0, 255);;
    bool masked = true;
    Timer tdrawpoly("drawPoly");
    renderer->drawPoly(corners, corner_count, fill, outline, mat, masked);
    tdrawpoly.stop();
    runtest.unresolved(std::string("drawPoly() ") + tdrawpoly.elapsed());
    
//    SWF::ShapeRecord shape;
    // Transform xform;
    
//    Timer drawshape("drawShape");
//    renderer->drawShape(shape, xform);
    runtest.untested("drawShape()");
//    drawshape.stop();
    
//    SWF::ShapeRecord rec;
    // rgba color;
    // SWFMatrix mat;
//    Timer drawGlyph("drawGlyph");
//    renderer->drawGlyph(rec, color, mat);
    runtest.untested("drawGlyph()");
//   drawglyph.stop();

#if 0
    boost::shared_ptr<IOChannel> io;
    FileType ftype;
    Timer renderi("renderToImage");
    renderer->renderToImage(io, ftype);
    renderi.stop();
#endif
    runtest.untested("renderToImage()");

    CachedBitmap *bitmap = 0;
    image::GnashImage *frame2 = new image::ImageRGBA(10, 10);
    std::auto_ptr<image::GnashImage> im(frame2);
    Timer cbit("createCachedBitmap");
    bitmap = renderer->createCachedBitmap(im);
    cbit.stop();
    if (bitmap) {
        runtest.pass(std::string("createCachedBitmap() ") + cbit.elapsed());
    } else {
        runtest.fail(std::string("createCachedBitmap() ") + cbit.elapsed());
    }
    
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
    Timer tpixtow("pixel_to_world(int, int)");
    z = renderer->pixel_to_world(x, y);
    tpixtow.stop();
    
    if ((z.x >= 199) || (z.y >= 199)) {
        runtest.pass(std::string("pixel_to_world(int, int) ") + tpixtow.elapsed());
    } else {
        runtest.fail(std::string("pixel_to_world(int, int) ") + tpixtow.elapsed());
    }
    
#if 0
    Timer tpixtow2("pixel_to_world(pixelbounds)");
    worldbounds = renderer->pixel_to_world(pixelbounds);
    tpixtow2.stop();
    if (worldbounds.isNull()) {
        runtest.pass(std::string("pixel_to_world(geometry::Range2d<int>) ") + tpixtow2.elapsed());
    } else {
        runtest.fail(std::string("pixel_to_world(geometry::Range2d<int>) ") + tpixtow2.elapsed());
    }
#else
    runtest.untested("pixel_to_world(geometry::Range2d<int>)");
#endif
    
    Timer twtop("world_to_pixel(geometry::Range2d<int>)");
    pixelbounds = renderer->world_to_pixel(worldbounds);
    twtop.stop();
    if (pixelbounds.isNull()) {
        runtest.pass(std::string("world_to_pixel(geometry::Range2d<int>) ") + twtop.elapsed());
    } else {
        runtest.fail(std::string("world_to_pixel(geometry::Range2d<int>) ") + twtop.elapsed());
    }
    
    SWFRect bounds;
    Timer tbounds1("bounds_in_clipping_area(SWFRect)");
    bool ret = renderer->bounds_in_clipping_area(bounds);
    tbounds1.stop();
    if (ret) {
        runtest.pass(std::string("bounds_in_clipping_area(SWFRect) ") + tbounds1.elapsed());
    } else {
        runtest.fail(std::string("bounds_in_clipping_area(SWFRect) ") + tbounds1.elapsed());
    }
    
    InvalidatedRanges ranges;
    Timer tbounds2("bounds_in_clipping_area(InvalidatedRanges)");
    ret = renderer->bounds_in_clipping_area(ranges);
    tbounds2.stop();
    if (!ret) {
        runtest.pass(std::string("bounds_in_clipping_area(InvalidatedRanges) ") + tbounds2.elapsed());
    } else {
        runtest.fail(std::string("bounds_in_clipping_area(InvalidatedRanges) ") + tbounds2.elapsed());
    }

    Timer tbounds3("bounds_in_clipping_area(geometry::Range2d<int>)");
    ret = renderer->bounds_in_clipping_area(pixelbounds);
    tbounds3.stop();
    if (!ret) {
        runtest.pass(std::string("bounds_in_clipping_area(geometry::Range2d<int>) ") + tbounds3.elapsed());
    } else {
        runtest.fail(std::string("bounds_in_clipping_area(geometry::Range2d<int>) ") + tbounds3.elapsed());
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
    // gnash::GnashVaapiImage *foo = static_cast<gnash::GnashVaapiImage *>(frame);
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
