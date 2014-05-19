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
#include <boost/scoped_ptr.hpp>
#include "openvg/OpenVGRenderer.h"

#ifdef HAVE_VG_OPENVG_H
#include <VG/openvg.h>
#endif

#ifdef HAVE_EGL_EGL_H
# include <EGL/egl.h>
#else
# error "This file needs EGL, which is part of OpenGL-ES"
#endif

namespace gnash {

namespace gui {

class GtkOvgGlue : public GtkGlue
{
  public:
    GtkOvgGlue();
    ~GtkOvgGlue();

    // Initialize EGL
    bool init(int argc, char **argv[]);

    // Prepare the drawing area
    void prepDrawingArea(GtkWidget *drawing_area);

    // Create the renderer
    Renderer* createRenderHandler();

    // Set the size of the rendering window
    void setRenderHandlerSize(int width, int height);
    void beforeRendering();

    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

    // Render the drawing area to the display
    void render();
    void render(int minx, int miny, int maxx, int maxy);

  private:
    // offscreenbuf is only used with ENABLE_EGL_OFFSCREEN
    GdkImage            *_offscreenbuf;
    unsigned int        _bpp;
    unsigned int        _width;
    unsigned int        _height;

    std::shared_ptr<renderer::openvg::Renderer_ovg>  _renderer;
};

} // namespace gui
} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
