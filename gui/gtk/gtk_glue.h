//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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

#ifndef GNASH_GTK_GLUE_H
#define GNASH_GTK_GLUE_H

#include <cassert>

#include <gtk/gtk.h>
#if !defined(_WIN32) && !defined(__APPLE__)
#include <gdk/gdkx.h>
#else
#include <gdk/gdk.h>
#endif

#include "DeviceGlue.h"

namespace gnash {
    class Renderer;
    class movie_root;
}

namespace gnash {

class GtkGlue : public DeviceGlue
{
  public:
    GtkGlue() : _drawing_area(0) { }
    virtual ~GtkGlue() { }
    virtual bool init(int argc, char **argv[]) = 0;

    virtual void prepDrawingArea(GtkWidget *drawing_area) = 0;
    virtual Renderer* createRenderHandler() = 0;
    virtual void setRenderHandlerSize(int /*width*/, int /*height*/) {}
    virtual void render() = 0;
    
    virtual void render(int /*minx*/, int /*miny*/, int /*maxx*/, int /*maxy*/)
    {
        render();	
    }

    virtual void render(GdkRegion * const region)
    {
        GdkRectangle* rects;
        gint num_rects;

        gdk_region_get_rectangles(region, &rects, &num_rects);
        assert(num_rects);

        for (gint i = 0; i < num_rects; ++i) {
            GdkRectangle const & r = rects[i];
            render(r.x, r.y, r.x + r.width, r.y + r.height);
        }

        g_free(rects);
    }

    virtual void configure(GtkWidget *const widget,
            GdkEventConfigure *const event) = 0;
    
    virtual void beforeRendering(movie_root*) {};

  protected:
    GtkWidget *_drawing_area;
};

} // namespace gnash

// end of GNASH_GTK_GLUE_H
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
