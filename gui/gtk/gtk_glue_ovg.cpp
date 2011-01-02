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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include <cerrno>
#include <exception>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "log.h"
#include "RunResources.h"
#include "Renderer.h"
#include "GnashException.h"

#ifdef HAVE_VG_OPENVG_H
# include "openvg/Renderer_ovg.h"
# include <VG/openvg.h>
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

#include "gtk_glue_ovg.h"

namespace gnash {

namespace gui {

GtkOvgGlue::GtkOvgGlue()
:   _offscreenbuf(0),
    _bpp(32),
    _width(0),
    _height(0)
{
    GNASH_REPORT_FUNCTION;
}

GtkOvgGlue::~GtkOvgGlue()
{
    GNASH_REPORT_FUNCTION;

#ifdef ENABLE_EGL_OFFSCREEN
    if (_offscreenbuf) {
        gdk_image_destroy(_offscreenbuf);
    }
#endif    

}

bool
GtkOvgGlue::init(int /*argc*/, char ** /*argv*/[])
{
    GNASH_REPORT_FUNCTION;
    
    bool egl = false;
    bool rawfb = false;
    bool dfb = false;
    bool x11 = false;
    
    // Probe to see what display devices we have that could be used.
    boost::shared_array<renderer::GnashDevice::dtype_t> devs = probeDevices();
    if (devs) {
        int i = 0;
        while (devs[i] != renderer::GnashDevice::NODEV) {
            switch (devs[i++]) {
              case renderer::GnashDevice::EGL:
                  log_debug("Probing found an EGL display device");
                  egl = true;
                  break;
              case renderer::GnashDevice::RAWFB:
                  log_debug("Probing found a raw Framebuffer display device");
                  rawfb = true;
                  break;
              case renderer::GnashDevice::X11:
                  log_debug("Probing found an X11 display device");
                  x11 = true;
                  break;
              case renderer::GnashDevice::DIRECTFB:
                  log_debug("Probing found a DirectFB display device");
                  dfb = true;
                  break;
              case renderer::GnashDevice::NODEV:
              default:
                  log_error("No display devices found by probing!");
                  break;
            }
        }

        // Now that we know what exists, we have to decide which one to
        // use, as OpenVG can work with anything. We can only have one
        // display device operating at a time.
        if (egl) {
            setDevice(renderer::GnashDevice::EGL);
        } else if (rawfb) {
            setDevice(renderer::GnashDevice::RAWFB);
        } else if (dfb) {
            setDevice(renderer::GnashDevice::DIRECTFB);
        } else if (x11) {
            setDevice(renderer::GnashDevice::X11);
        }        
    }

    // Initialize the display device
    _device->initDevice(0, 0);

    return true;
}

void
GtkOvgGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    GNASH_REPORT_FUNCTION;

    _drawing_area = drawing_area;
    
    // Disable double buffering, otherwise gtk tries to update widget
    // contents from its internal offscreen buffer at the end of expose event
    gtk_widget_set_double_buffered(_drawing_area, FALSE);

    // EGL needs to be bound to the type of client. The possible
    // clients are OpenVG, OpenGLES1, and OpenGLES2.
    if (_device->getType() == renderer::GnashDevice::EGL) {
        renderer::EGLDevice *egl = (renderer::EGLDevice*)_device.get();
        egl->bindClient(renderer::GnashDevice::OPENVG);
    }
    
#if 0
    renderer::EGLDevice *egl = (renderer::EGLDevice*)_device.get();
    egl->printEGLSurface(eglGetCurrentSurface(EGL_DRAW));
    egl->printEGLContext(eglGetCurrentContext());
#endif
}

Renderer*
GtkOvgGlue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;

    if (!_drawing_area) {
        log_error("No area to draw in!");
        return 0;
    }
    
    GdkVisual* wvisual = gdk_drawable_get_visual(_drawing_area->window);
    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);
    const GdkVisual* visual = tmpimage->visual;
    gdk_image_destroy(tmpimage);

    _renderer.reset(reinterpret_cast<renderer::openvg::Renderer_ovg *>
                    (renderer::openvg::create_handler(0)));
    if (!_renderer) {
        boost::format fmt = boost::format(
            _("Could not create OPENVG renderer"));
        throw GnashException(fmt.str());
    }

    return reinterpret_cast<Renderer *>(_renderer.get());
}

void
GtkOvgGlue::setRenderHandlerSize(int width, int height)
{
    GNASH_REPORT_FUNCTION;

    assert(width > 0);
    assert(height > 0);
    assert(_renderer != NULL);
    
#ifdef ENABLE_EGL_OFFSCREEN
    if (_offscreenbuf && _offscreenbuf->width == width &&
        _offscreenbuf->height == height) {
        return; 
    }

    if (_offscreenbuf) {
        gdk_image_destroy(_offscreenbuf);
    }

    GdkVisual* visual = gdk_drawable_get_visual(_drawing_area->window);
    _offscreenbuf = gdk_image_new (GDK_IMAGE_FASTEST, visual, width,
                                   height);
#endif

#if 1
    // if (_renderer) {
    //     vgScale(width, height);
    // }
#else
    // Attach the window to the low level device
    long xid = GDK_WINDOW_XID(gtk_widget_get_window(_drawing_area));
    _device->attachWindow(static_cast<renderer::GnashDevice::native_window_t>
                          (xid));

    vgLoadIdentity();

    // Allow drawing everywhere by default
    InvalidatedRanges ranges;
    ranges.setWorld();
    _renderer->set_invalidated_regions(ranges);    

    renderer::EGLDevice *egl = (renderer::EGLDevice*)_device.get();
    egl->swapBuffers();
#endif    
}

void 
GtkOvgGlue::beforeRendering()
{
    GNASH_REPORT_FUNCTION;

#ifdef ENABLE_EGL_OFFSCREEN
    if (_offscreenbuf && _offscreenbuf->type == GDK_IMAGE_SHARED) {
         gdk_flush();
    }
#endif
}

void
GtkOvgGlue::render()
{
    GNASH_REPORT_FUNCTION;

#if 0
    // clear the color buffer
    glClearColor(1.0, 1.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
#endif

#ifdef ENABLE_EGL_OFFSCREEN
    if ( _drawbounds.size() == 0 ) {
        return; // nothing to do
    }

    if (!_offscreenbuf) {
        log_error("No off screen buffer!");
        return;
    }
    
    render(0, 0, _offscreenbuf->width, _offscreenbuf->height);
#else
    render(0, 0, _width, _height);
#endif
}

void
GtkOvgGlue::render(int minx, int miny, int maxx, int maxy)
{
    GNASH_REPORT_FUNCTION;

#ifdef ENABLE_EGL_OFFSCREEN
    if (!_offscreenbuf) {
        log_error("No off screen buffer!");
        return;
    }
    
    const int& x = minx;
    const int& y = miny;
    size_t width = std::min(_offscreenbuf->width, maxx - minx);
    size_t height = std::min(_offscreenbuf->height, maxy - miny);
    
    GdkGC* gc = gdk_gc_new(_drawing_area->window);
    
    gdk_draw_image(_drawing_area->window, gc, _offscreenbuf, x, y, x, y, width,
                   height);
    gdk_gc_unref(gc);
#else
//    _device->swapBbuffers();
#endif
}

void
GtkOvgGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
    GNASH_REPORT_FUNCTION;

    setRenderHandlerSize(event->width, event->height);
}

#if 0
bool
GtkOvgGlue::checkEGLConfig(EGLConfig config)
{
    // GNASH_REPORT_FUNCTION;
    
    // Use this to explicitly check that the EGL config has the expected color depths
    EGLint value;
    if (_bpp == 32) {            
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (8 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
        if (0 != value) {
            return false;
        }
    } else if (_bpp == 16) {
        eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &value);
        if ( 5 != value ) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &value);
        if (6 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &value);
        if (5 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
        if (0 != value) {
            return false;
        }
        eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
#ifdef  RENDERER_GLES            
        if (4 != value) {
            return false;
        }
#endif
#ifdef  RENDERER_OPENVG
        if (0 != value) {
            return false;
            }
#endif
    } else {
        return false;
    }

    return true;
}

/// Query the system for all supported configs
int
GtkOvgGlue::queryEGLConfig(EGLDisplay display)
{
     GNASH_REPORT_FUNCTION;
     EGLConfig *configs = 0;
     EGLint max_num_config = 0;

     // Get the number of supported configurations
     if ( EGL_FALSE == eglGetConfigs(display, 0, 0, &max_num_config) ) {
         log_error("eglGetConfigs() failed to retrive the number of configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
     if(max_num_config <= 0) {
         printf( "No EGLconfigs found\n" );
         return 0;
     }
     log_debug("Max number of EGL Configs is %d", max_num_config);     
     
     configs = new EGLConfig[max_num_config];
     if (0 == configs) {
         log_error( "Out of memory\n" );
         return 0;
     }

     if ( EGL_FALSE == eglGetConfigs(display, configs, max_num_config, &max_num_config)) {
         log_error("eglGetConfigs() failed to retrive the configs (error %s)",
                   getErrorString(eglGetError()));
         return 0;
     }
     for (int i=0; i<max_num_config; i++ ) {
         log_debug("Config[%d] is:", i);
         printEGLConfig(configs[i]);
     }

     return max_num_config;
}
#endif

} // namespace gui
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
