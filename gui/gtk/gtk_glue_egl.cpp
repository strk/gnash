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
#include "gtk_glue_egl.h"
#include "GnashException.h"
#include "openvg/Renderer_ovg.h"

#ifdef HAVE_VG_OPENVG_H
#include <VG/openvg.h>
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

#include <GL/gl.h>

namespace gnash
{

static const EGLint attrib32_list[] = {
    EGL_RED_SIZE,       8,
    EGL_GREEN_SIZE,     8,
    EGL_BLUE_SIZE,      8,
    EGL_ALPHA_SIZE,     0,
#ifdef RENDERER_GLES    
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
#endif
#ifdef RENDERER_OPENVG
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_DEPTH_SIZE,     24,
    EGL_STENCIL_SIZE,   8,
#endif
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_NONE
};

static EGLint const attrib16_list[] = {
    EGL_RED_SIZE,       5,
    EGL_GREEN_SIZE,     6,
    EGL_BLUE_SIZE,      5,
    EGL_ALPHA_SIZE,     0,
#ifdef RENDERER_GLES    
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
#endif
#ifdef RENDERER_OPENVG
    EGL_RENDERABLE_TYPE, EGL_OPENVG_BIT,
    EGL_DEPTH_SIZE,     0,
    EGL_STENCIL_SIZE,   0,
#endif
    EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
    EGL_SAMPLE_BUFFERS, 0,
    EGL_NONE
};

const EGLint window_attrib_list[] = {
    // Back buffering is used for window and pbuffer surfaces. Windows
    // require eglSwapBuffers() to become visible, and pbuffers don't.   
    // EGL_SINGLE_BUFFER is by pixmap surfaces. With OpenVG, windows
    // can also be single buffered. eglCopyBuffers() can be used to copy
    // both back and single buffered surfaces to a pixmap.
    EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
    EGL_COLORSPACE,    EGL_COLORSPACE_sRGB,
    EGL_NONE
};

// From the EGL 1.4 spec:
//
// EGL defines several types of drawing surfaces collectively referred
// to as EGLSurfaces. These include windows, used for onscreen
// rendering; pbuffers, used for offscreen rendering; and pixmaps,
// used for offscreen rendering into buffers that may be accessed
// through native APIs. EGL windows and pixmaps are tied to native
// window system windows and pixmaps.
//
// depth, multisample, and stencil buffers are currently used only by
// OpenGL-ES.

// EGL and OpenGL ES supports two rendering models: back buffered and
// single buffered. Back buffered rendering is used by window and
// pbuffer surfaces. Memory for the color buffer used during rendering
// is allocated and owned by EGL. When the client is finished drawing
// a frame, the back buffer may be copied to a visible window using
// eglSwapBuffers. Pbuffer surfaces have a back buffer but no
// associated window, so the back buffer need not be copied.
//
// Single buffered rendering is used by pixmap surfaces. Memory for
// the color buffer is specified at surface creation time in the form
// of a native pixmap, and client APIs are required to use that memory
// during rendering. When the client is finished drawing a frame, the
// native pixmap contains the final image. Pixmap surfaces typically
// do not support multisampling, since the native pixmap used as the
// color buffer is unlikely to provide space to store multisample
// information. Some client APIs , such as OpenGL and OpenVG , also
// support single buffered rendering to window surfaces. This behavior
// can be selected when creating the window surface, as defined in
// section 3.5.1. When mixing use of client APIs which do not support
// single buffered rendering into windows, like OpenGL ES , with
// client APIs which do support it, back color buffers and visible
// window contents must be kept consistent when binding window
// surfaces to contexts for each API type. Both back and single
// buffered surfaces may also be copied to a specified native pixmap
// using eglCopyBuffers.

// Native rendering will always be supported by pixmap surfaces (to
// the extent that native rendering APIs can draw to native
// pixmaps). Pixmap surfaces are typically used when mixing native and
// client API rendering is desirable, since there is no need to move
// data between the back buffer visible to the client APIs and the
// native pixmap visible to native rendering APIs. However, pixmap
// surfaces may, for the same reason, have restricted capabilities and
// performance relative to window and pbuffer surfaces.

GtkEGLGlue::GtkEGLGlue()
:   _offscreenbuf(0),
    _renderer(0),
    _eglConfig(0),
    _eglContext(EGL_NO_CONTEXT),
    _eglSurface(EGL_NO_SURFACE),
    _eglDisplay(EGL_NO_DISPLAY),
    _eglNumOfConfigs(0),
    _nativeWindow(0),
    _max_num_config(1),
    _bpp(32),
    _width(0),
    _height(0)
{
    GNASH_REPORT_FUNCTION;
}

GtkEGLGlue::~GtkEGLGlue()
{
    GNASH_REPORT_FUNCTION;

#ifdef ENABLE_EGL_OFFSCREEN
    if (_offscreenbuf) {
        gdk_image_destroy(_offscreenbuf);
    }
#endif    
    if (_eglDisplay != EGL_NO_DISPLAY) {  
        eglMakeCurrent(_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (_eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(_eglDisplay, _eglContext);
        
        if (_eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(_eglDisplay, _eglSurface);
        
        // if (_eglwin_native)
        //     free(_eglwin_native);
        
        eglTerminate(_eglDisplay);
    }
}

bool
GtkEGLGlue::init(int /*argc*/, char ** /*argv*/[])
{
    GNASH_REPORT_FUNCTION;
    
    EGLint major, minor;
    // see egl_config.c for a list of supported configs, this looks for
    // a 5650 (rgba) config, supporting OpenGL ES and windowed surfaces

    // step 1 - get an EGL display
    
    // This can be called multiple times, and always returns the same display
    _eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY); // FIXME: gdk_display ?
    if (EGL_NO_DISPLAY == _eglDisplay) {
        log_error( "eglGetDisplay() failed (error 0x%x)", eglGetError() );
        return false;
    }

    // This can be called multiple times safely
    if (EGL_FALSE == eglInitialize(_eglDisplay, &major, &minor)) {
        log_error( "eglInitialize() failed (error %s)",
                   getErrorString(eglGetError()));
        return false;
    }
    // log_debug("EGL_CLIENT_APIS = %s", eglQueryString(_eglDisplay, EGL_CLIENT_APIS));
    // log_debug("EGL_EXTENSIONS = %s",  eglQueryString(_eglDisplay, EGL_EXTENSIONS));
    log_debug("EGL_VERSION = %s, EGL_VENDOR = %s",
              eglQueryString(_eglDisplay, EGL_VERSION),
              eglQueryString(_eglDisplay, EGL_VENDOR));

    // step2 - bind to the wanted client API
#ifdef  RENDERER_GLES
    if(EGL_FALSE == eglBindAPI(EGL_OPENGL_ES_API)) {
        log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }
#endif
#ifdef RENDERER_OPENVG
    if(EGL_FALSE == eglBindAPI(EGL_OPENVG_API)) {
        log_error("eglBindAPI() failed to retrive the number of configs (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }
#endif

//    queryEGLConfig(_eglDisplay);
    
    // step3 - find a suitable config
    if (_bpp == 32) {
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib32_list, &_eglConfig,
                                          1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig() failed (error %s)", 
                       getErrorString(eglGetError()));
            return false;
        }
    } else if (_bpp == 16) {
        if (EGL_FALSE == eglChooseConfig(_eglDisplay, attrib16_list, &_eglConfig,
                                         1, &_eglNumOfConfigs)) {
            log_error("eglChooseConfig() failed (error %s)",
                       getErrorString(eglGetError()));
            return false;
        }
    } else {
        log_error("No supported bpp value!");
    }

    if (0 == _eglNumOfConfigs) {
        log_error("eglChooseConfig() was unable to find a suitable config");
        return false;
    }

    EGLint vid;
    if (!eglGetConfigAttrib(_eglDisplay, _eglConfig, EGL_NATIVE_VISUAL_ID, &vid)) {
        log_error("eglGetConfigAttrib() failed (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }
    
    XVisualInfo *visInfo, visTemplate;
    int num_visuals;
    // The X window visual must match the EGL config
   visTemplate.visualid = vid;
   visInfo = XGetVisualInfo(gdk_display, VisualIDMask, &visTemplate, &num_visuals);
   if (!visInfo) {
       log_error("couldn't get X visual");
       return false;
   }
   XFree(visInfo);
    
   if (!checkEGLConfig(_eglConfig)) {
       log_error("EGL configuration doesn't match!");
//       return false;
   } else {
       //printEGLConfig(_eglConfig);
   }

    // step4 - create a window surface
    _nativeWindow = gdk_x11_get_default_root_xwindow();
    

#ifdef  RENDERER_GLES
    _eglSurface = eglCreateWindowSurface(_eglDisplay, &_eglConfig, _nativeWindow, NULL);
#endif
#ifdef  RENDERER_OPENVG
    if (_nativeWindow) {
        _eglSurface = eglCreateWindowSurface(_eglDisplay, _eglConfig,
                                             _nativeWindow, 0); // was window_attrib_list
    } else {
        log_error("No native window!");
        return false;
    }
#endif

    if (EGL_NO_SURFACE == _eglSurface) {
        log_error("eglCreateWindowSurface failed (error %s)", 
                  getErrorString(eglGetError()));
        return false;
    } else {
        //printEGLSurface(_eglSurface);
    }

    // step5 - create a context
    _eglContext = eglCreateContext(_eglDisplay, _eglConfig, EGL_NO_CONTEXT, NULL);
    if (EGL_NO_CONTEXT == _eglContext) {
        log_error("eglCreateContext failed (error %s)",
                   getErrorString(eglGetError()));
        return false;
    } else {
        printEGLContext(_eglContext);
    }
    
    // step6 - make the context and surface current
    if (EGL_FALSE == eglMakeCurrent(_eglDisplay, _eglSurface, _eglSurface, _eglContext)) {
        log_error("eglMakeCurrent failed (error %s)",
                  getErrorString(eglGetError()));
        return false;
    }       // begin user code

#if 0
#if 1
    eglSwapInterval(_eglDisplay, 0);
#else
    eglSwapBuffers(_eglDisplay, _eglSurface);
#endif

    log_debug("Gnash EGL Frame width %d height %d bpp %d \n", _width, _height, _bpp);
#endif
    
    return true;
}

void
GtkEGLGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    GNASH_REPORT_FUNCTION;

    _drawing_area = drawing_area;

    // Disable double buffering, otherwise gtk tries to update widget
    // contents from its internal offscreen buffer at the end of expose event
    gtk_widget_set_double_buffered(_drawing_area, FALSE);

    DUMP_CURRENT_SURFACE;
    DUMP_CURRENT_CONTEXT;
}

Renderer*
GtkEGLGlue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;

    if (!_drawing_area) {
        log_error("No area to draw in!");
        return 0;
    }
    
    GdkVisual* wvisual = gdk_drawable_get_visual(_drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);

    const GdkVisual* visual = tmpimage->visual;

    // FIXME: we use bpp instead of depth, because depth doesn't appear to
    // include the padding byte(s) the GdkImage actually has.
    const char *pixelformat = 0;
    // agg_detect_pixel_format(visual->red_shift, visual->red_prec,
    // visual->green_shift, visual->green_prec, visual->blue_shift, visual->blue_prec,
    // tmpimage->bpp * 8);

    gdk_image_destroy(tmpimage);

//    _renderer = dynamic_cast<Renderer *>(renderer::openvg::create_handler(pixelformat));
    if (! _renderer) {
        boost::format fmt = boost::format(
            _("Could not create OPENVG renderer with pixelformat %s")
            ) % pixelformat;
        throw GnashException(fmt.str());
    }

    return _renderer;
}

void
GtkEGLGlue::setRenderHandlerSize(int width, int height)
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
    
    static_cast<Renderer_ovg_base *>(_renderer)->init_buffer(
        (unsigned char*) _offscreenbuf->mem,
        _offscreenbuf->bpl * _offscreenbuf->height,
        _offscreenbuf->width,
        _offscreenbuf->height,
        _offscreenbuf->bpl);
#else
        _renderer->init(width, height);
#endif
}

void 
GtkEGLGlue::beforeRendering()
{
    GNASH_REPORT_FUNCTION;

#ifdef ENABLE_EGL_OFFSCREEN
    if (_offscreenbuf && _offscreenbuf->type == GDK_IMAGE_SHARED) {
         gdk_flush();
    }
#endif
}

void
GtkEGLGlue::render()
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
GtkEGLGlue::render(int minx, int miny, int maxx, int maxy)
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
    eglSwapBuffers(_eglDisplay, _eglSurface);
#endif
}

void
GtkEGLGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
    GNASH_REPORT_FUNCTION;

    if (_renderer) {
        setRenderHandlerSize(event->width, event->height);
    }
}

const char *
GtkEGLGlue::getErrorString(int error)
{
    switch (error) {
    case EGL_SUCCESS:
        return "EGL_SUCCESS";
    case EGL_NOT_INITIALIZED:
        return "EGL_NOT_INITIALIZED";
    case EGL_BAD_ACCESS:
        return "EGL_BAD_ACCESS";
    case EGL_BAD_ALLOC:
        return "EGL_BAD_ALLOC";
    case EGL_BAD_ATTRIBUTE:
        return "EGL_BAD_ATTRIBUTE";
    case EGL_BAD_CONFIG:
        return "EGL_BAD_CONFIG";
    case EGL_BAD_CONTEXT:
        return "EGL_BAD_CONTEXT";
    case EGL_BAD_CURRENT_SURFACE:
        return "EGL_BAD_CURRENT_SURFACE";
    case EGL_BAD_DISPLAY:
        return "EGL_BAD_DISPLAY";
    case EGL_BAD_MATCH:
        return "EGL_BAD_MATCH";
    case EGL_BAD_NATIVE_PIXMAP:
        return "EGL_BAD_NATIVE_PIXMAP";
    case EGL_BAD_NATIVE_WINDOW:
        return "EGL_BAD_NATIVE_WINDOW";
    case EGL_BAD_PARAMETER:
        return "EGL_BAD_PARAMETER";
    case EGL_BAD_SURFACE:
        return "EGL_BAD_SURFACE";
    case EGL_CONTEXT_LOST:
        return "EGL_CONTEXT_LOST";
    default:
        return "unknown error code";
    }
}

bool
GtkEGLGlue::checkEGLConfig(EGLConfig config)
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
GtkEGLGlue::queryEGLConfig(EGLDisplay display)
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

void
GtkEGLGlue::printEGLConfig(EGLConfig config)
{
    EGLint red, blue, green;
    EGLint value;
    eglGetConfigAttrib(_eglDisplay, config, EGL_RED_SIZE, &red);
    eglGetConfigAttrib(_eglDisplay, config, EGL_GREEN_SIZE, &green);
    eglGetConfigAttrib(_eglDisplay, config, EGL_BLUE_SIZE, &blue);
    log_debug("\tConfig has RED = %d, GREEN = %d, BLUE = %d", red, green, blue);
    
    eglGetConfigAttrib(_eglDisplay, config, EGL_ALPHA_SIZE, &value);
    log_debug("\tEGL_ALPHA_SIZE is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_STENCIL_SIZE, &value);
    log_debug("\tEGL_STENCIL_SIZE is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLES, &value);
    log_debug("\tEGL_SAMPLES is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_DEPTH_SIZE, &value);
    log_debug("\tEGL_DEPTH_SIZE is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_MAX_SWAP_INTERVAL, &value);
    log_debug("\tEGL_MAX_SWAP_INTERVAL is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_MIN_SWAP_INTERVAL, &value);
    log_debug("\tEGL_MIN_SWAP_INTERVAL is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_NATIVE_RENDERABLE, &value);
    log_debug("\tEGL_NATIVE_RENDERABLE is %s", (value)? "true" : "false");
    eglGetConfigAttrib(_eglDisplay, config, EGL_SAMPLE_BUFFERS, &value);
    log_debug("\tEGL_SAMPLE_BUFFERS is %d", value);
    eglGetConfigAttrib(_eglDisplay, config, EGL_RENDERABLE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_OPENGL_ES_BIT) {
            str += " OpenGL-ES 1.1";
        }
        if (value & EGL_OPENVG_BIT) {
            str += " OpenVG";
        }
        if (value & EGL_OPENGL_BIT) {
            str += " OpenGL";
        }
        log_debug("\tEGL_RENDERABLE_TYPE = %s", str);
    } else {
        log_debug("\tEGL_RENDERABLE_TYPE (default)");
    }
    eglGetConfigAttrib(_eglDisplay, config, EGL_SURFACE_TYPE, &value);
    if (value > 0) {
        std::string str;
        if (value & EGL_WINDOW_BIT) {
            str += " Window";
        }
        if (value & EGL_PIXMAP_BIT) {
            str += " Pixmap";
        }
        if (value & EGL_PBUFFER_BIT) {
            str += " Pbuffer";
        }
        log_debug("\tEGL_SURFACE_TYPE = %s", str);
    } else {
        log_debug("\tEGL_SURFACE_TYPE (default)");
    }
}

void
GtkEGLGlue::printEGLContext(EGLContext context)
{
    EGLint value;
    eglQueryContext(_eglDisplay, context, EGL_CONFIG_ID, &value);
    log_debug("Context EGL_CONFIG_ID is %d", value);
    eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_TYPE, &value);
    log_debug("\tEGL_CONTEXT_CLIENT_TYPE is %d", (value == EGL_OPENVG_API)
              ? "EGL_OPENVG_API" : "EGL_OPENGL_ES_API");
    // eglQueryContext(_eglDisplay, context, EGL_CONTEXT_CLIENT_VERSION, &value);
    // log_debug("EGL_CONTEXT_CLIENT_VERSION is %d", value);
    eglQueryContext(_eglDisplay, context, EGL_RENDER_BUFFER, &value);
    log_debug("\tEGL_RENDER_BUFFER is %s", (value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER");
}

void
GtkEGLGlue::printEGLSurface(EGLSurface surface)
{
    EGLint value;
    eglQuerySurface(_eglDisplay, surface, EGL_CONFIG_ID, &value);
    log_debug("Surface EGL_CONFIG_ID is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_HEIGHT, &value);
    log_debug("\tEGL_HEIGHT is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_WIDTH, &value);
    log_debug("\tEGL_WIDTH is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_RENDER_BUFFER, &value);
    log_debug("\tEGL_RENDER_BUFFER is %s", (value == EGL_BACK_BUFFER)
              ? "EGL_BACK_BUFFER" : "EGL_SINGLE_BUFFER");
    eglQuerySurface(_eglDisplay, surface, EGL_VERTICAL_RESOLUTION, &value);
    log_debug("\tEGL_VERTICAL_RESOLUTION is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_HORIZONTAL_RESOLUTION, &value);
    log_debug("\tEGL_HORIZONTAL_RESOLUTION is %d", value);
    eglQuerySurface(_eglDisplay, surface, EGL_SWAP_BEHAVIOR, &value);
    log_debug("\tEGL_SWAP_BEHAVIOR is %d", (value == EGL_BUFFER_DESTROYED)
              ? "EGL_BUFFER_DESTROYED" : "EGL_BUFFER_PRESERVED");
    eglQuerySurface(_eglDisplay, surface, EGL_MULTISAMPLE_RESOLVE, &value);
    log_debug("\tEGL_MULTISAMPLE_RESOLVE is %d", (value == EGL_MULTISAMPLE_RESOLVE_BOX)
              ? "EGL_MULTISAMPLE_RESOLVE_BOX" : "EGL_MULTISAMPLE_RESOLVE_DEFAULT");
}

} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
