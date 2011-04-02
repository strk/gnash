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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtk_glue.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <boost/scoped_array.hpp>
#include "openvg/Renderer_ovg.h"

#ifdef HAVE_VG_OPENVG_H
#include <VG/openvg.h>
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

namespace gnash
{

class GtkEGLGlue : public GtkGlue
{
  public:
    GtkEGLGlue();
    ~GtkEGLGlue();

    // Initialize EGL
    bool init(int argc, char **argv[]);

    // Prepare the drawing area
    void prepDrawingArea(GtkWidget *drawing_area);

    // Create the renderer
    Renderer* createRenderHandler();

    // Set the size of the rendering window
    void setRenderHandlerSize(int width, int height);
    void beforeRendering();

    // Render the drawing area to the display
    void render();
    void render(int minx, int miny, int maxx, int maxy);

    // Configure EGL
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

    // Utility methods not in the base class
    /// Return a string with the error code as text, instead of a numeric value
    const char *getErrorString(int error);
    /// Check the requested EGl configuration against the current one
    bool checkEGLConfig(EGLConfig config);
    /// Query the system for all supported configs
    int queryEGLConfig(EGLDisplay display);
    void printEGLConfig(EGLConfig config);
    void printEGLContext(EGLContext context);
    void printEGLSurface(EGLSurface surface);

  private:
    // offscreenbuf is only used with ENABLE_EGL_OFFSCREEN
    GdkImage            *_offscreenbuf;
    renderer::openvg::Renderer_ovg        *_renderer;
    
    EGLConfig           _eglConfig;
    EGLContext          _eglContext;
    EGLSurface          _eglSurface;
    EGLDisplay          _eglDisplay;
    EGLint              _eglNumOfConfigs;
    EGLNativeWindowType _nativeWindow;
    EGLint              _max_num_config;
    unsigned int        _bpp;
    unsigned int        _width;
    unsigned int        _height;
};

#define DUMP_CURRENT_SURFACE printEGLSurface(eglGetCurrentSurface(EGL_DRAW))
#define DUMP_CURRENT_CONTEXT printEGLContext(eglGetCurrentContext())

} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
