// 
//   Copyright (C) 2010, 2011, 2012 Free Software Foundation, Inc
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "log.h"
#include "dejagnu.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "ShapeRecord.h"
#include "CachedBitmap.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"
#include "boost/date_time/posix_time/posix_time.hpp"

#ifdef RENDERER_AGG
#include "agg/Renderer_agg.h"
#endif
#ifdef RENDERER_OPENGL
#include "opengl/Renderer_ogl.h"
#endif
#ifdef RENDERER_OPENVG
#include "openvg/OpenVGRenderer.h"
#include <VG/vgu.h>
#ifdef HAVE_VG_VGEXT_H
# include <VG/vgext.h>
#else
# include <VG/ext.h>
#endif
#endif
#ifdef RENDERER_GLES1
#include "opengles1/Renderer_gles1.h"
#endif
#ifdef RENDERER_GLES2
#include "opengles2/Renderer_gles2.h"
#endif
#ifdef RENDERER_CAIRO
#include "cairo/Renderer_cairo.h"
#endif
#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL"
#endif

#ifdef BUILD_EGL_DEVICE
# include <egl/eglDevice.h>
#endif
#ifdef BUILD_DIRECTFB_DEVICE
# include <directfb/directfb.h>
#endif
#ifdef BUILD_X11_DEVICE
# include <x11/X11Device.h>
#endif

TestState runtest;

using namespace gnash; 
using namespace renderer; 
using namespace std;
using namespace boost::posix_time;

void test_device(Renderer *renderer, const std::string &type);
void test_renderer(Renderer *renderer, const std::string &type);
void test_geometry(Renderer *renderer, const std::string &type);
void test_iterators(Renderer *renderer, const std::string &type);

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

//------------------------------------------------------------

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
    int fd = 0;

#ifdef BUILD_RAWFB_DEVICE_XXX
    rawfb::RawFBDevice rawfb(argc, argv);
    fd = rawfb.getHandle();
#endif  // BUILD_RAWFB_DEVICE

#if defined(BUILD_EGL_DEVICE) && !defined(BUILD_X11_DEVICE)
    // Setup EGL, OpenVG needs it
    EGLDevice egl(argc, argv);
    egl.bindClient(GnashDevice::OPENVG);
    fd = open("/dev/fb0", O_RDWR);
    egl.attachWindow(fd);
#endif  // BUILD_EGL_DEVICE
    
#ifdef BUILD_X11_DEVICE
    // Setup EGL, OpenVG needs it
    EGLDevice egl;
    //egl.setAttrib(1);
    egl.initDevice(argc, argv);
    int vid = egl.getNativeVisual();
    egl.bindClient(GnashDevice::OPENVG);

    // Create an X11 device for the display. This is for libMesa
    // where we can also run OpenVG on the desktop.
    x11::X11Device x11(vid);
    //x11.initDevice(argc, argv);
    x11.createWindow("TestR", 0, 0, 640, 480);
    
    // This is the window that gets rendered in
    Window win = x11.getHandle();
    if (win) {
        egl.attachWindow(win);
    } else {
        log_error(_("Couldn't get Drawable window from X11"));
        exit(1);
    }    
    // Set initial projection/viewing transformation.
    // We can't be sure we'll get a ConfigureNotify event when the window
    // first appears.
#if 0
    if (reshape) {
        reshape(640, 480);
    }
#endif
    x11.eventLoop(10);
    std::cerr << "Hello World!" << std::endl;
    x11.eventLoop(10);
#endif

#ifdef RENDERER_AGG
    Timer tagg("AGG");
    Renderer *renderer1 = create_Renderer_agg(pixelformat);
    // The methods were moved to the glue API
    // if (renderer1) {
    //     test_device(renderer1, "EGL");
    //     test_device(renderer1, "DIRECTFB");
    //     test_device(renderer1, "X11");
    // }
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
//     EGLDevice *ovg = dynamic_cast<EGLDevice *>(renderer2);
//     if (ovg) {
//         ovg->initDevice(0, 0);
// //        ovg->attachWindow(*(reinterpret_cast<EGLNativeWindowType *>(canvas)));
//         ovg->attachWindow(0);
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
    
#if 0
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
}

// Each Renderer has an associated display device, currently EGL
// for OpenVG, OpenGLES1, and OpenGLES2. The DirectFB device is
// also available for these three renderers. The EGL device can
// also be run under X11 using the Mesa libraries. Both EGL and
// DirectFB are primarily for framebuffers. While all of the methods
// of each device class are available to the Renderer, most aren't
// exposed with more accesors beyond these for manipulating the
// device setting itself.
void
test_device(Renderer *renderer, const std::string &type)
{
    cout << endl << "Testing " << type << " Device" << endl;

#if 0
    std::unique_ptr<renderer::GnashDevice::dtype_t[]> devs = renderer->probeDevices();
    if (devs) {
        runtest.pass("Renderer::probeDevices()");
    } else {
        runtest.fail("Renderer::probeDevices()");
    }
    // Be default, there should be no device associated with this
    // renderer yet.
    if (renderer->getDevice() == GnashDevice::GNASH_NODEV) {
        runtest.pass("Renderer::getDevice()");
    } else {
        runtest.fail("Renderer::getDevice()");
    }

    // Set to a device and see if it's axctually set
    renderer->setDevice(GnashDevice::X11);
    if (renderer->getDevice() == GnashDevice::X11) {
        runtest.pass("Renderer::setDevice()");
    } else {
        runtest.fail("Renderer::setDevice()");
    }

    // reset to the original value so we don't screw up future tests
    renderer->resetDevice();
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

#if 0
    if (renderer->getBitsPerPixel()) {
        runtest.pass("getBitsPerPixel()");
    } else {
        runtest.fail("getBitsPerPixel()");
    }
#endif
    
    image::GnashImage *frame1 = new image::ImageRGBA(10, 12);
    std::unique_ptr<image::GnashImage> im1(frame1);
    CachedBitmap *cb = renderer->createCachedBitmap(im1);
    if (cb) {
        image::GnashImage &gi = cb->image();
        if ((gi.width() == 10) && (gi.height() == 12)) {
            runtest.pass("createCachedBitmap()");
        } else {
            runtest.fail("createCachedBitmap()");
        }
    } else {
        runtest.unresolved("createCachedBitmap()");
    }

#if 0
    // FIXME: initTestBuffer() is going away, replaced by the fake
    // framebuffer code.
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
    std::uint16_t x = 10;
    std::uint16_t y = 10;
    std::uint16_t h = 10;
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
    renderer->draw_poly(corners, corner_count, fill, outline, mat, masked);
    tdrawpoly.stop();
    runtest.unresolved(std::string("drawPoly() ") + tdrawpoly.elapsed());
    
//    SWF::ShapeRecord shape;
    SWFMatrix mat2(0x10000, 0x0, 0x0, 0x10000, 0x0, 0x0);
    SWFCxForm cxform;
    //(0x100, 0x100, 0x100, 0x100, 0x0, 0x0, 0x0, 0x0);
    Transform xform(mat2, cxform);
    SWF::ShapeRecord shape;
    // _bounds = {0x80000000, 0x7fffffff,
    //            0x80000000, 0x80000000,
    //            0x80000000, 0x80000000}}
    Timer drawshape("drawShape");
    renderer->drawShape(shape, xform);
    runtest.unresolved("drawShape()");
    drawshape.stop();
    
//    SWF::ShapeRecord rec;
    // rgba color;
    // SWFMatrix mat;
//    Timer drawGlyph("drawGlyph");
//    renderer->drawGlyph(rec, color, mat);
    runtest.untested("drawGlyph()");
//   drawglyph.stop();

#if 0
    std::unique_ptr<IOChannel> io;
    FileType ftype;
    Timer renderi("renderToImage");
    renderer->renderToImage(io, ftype);
    renderi.stop();
#endif
    runtest.untested("renderToImage()");

    CachedBitmap *bitmap = 0;
    image::GnashImage *frame2 = new image::ImageRGBA(10, 10);
    std::unique_ptr<image::GnashImage> im(frame2);
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

#if 0    
    geometry::Point2d a(1, 2);
    geometry::Point2d c(3, 4);
    Renderer::RenderImage image;
    image::GnashImage *frame = new image::ImageRGBA(10, 10);
    // gnash::GnashVaapiImage *foo = static_cast<gnash::GnashVaapiImage *>(frame);
    // gnash::GnashVaapiImageProxy *bar = new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y);
    std::unique_ptr<image::GnashImage> rgba(frame);
//    image.reset(new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y));

    renderer->addRenderImage(image);
//    image.reset(new gnash::GnashVaapiImageProxy(foo, a.x, a.y, c.x - a.x, c.y - a.y));
    renderer->addRenderImage(image);
#endif
    
    fit = renderer->getFirstRenderImage();
    lit = renderer->getLastRenderImage();
    // When there are no images, the first and last are the same obviously
    if (fit != lit) {
         runtest.pass("addRenderImage()");
    } else {
        runtest.fail("addRenderImage()");
    }
    
#if 0
    typedef std::shared_ptr<GnashVaapiImageProxy> RenderImage;
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

#if 0
FIXME:
 add tests for
Renderer_ovg::createBitmapInfo(std::unique_ptr<GnashImage> im)
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
