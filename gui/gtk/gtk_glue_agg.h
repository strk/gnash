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
//

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gtk_glue.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <boost/scoped_array.hpp>

namespace gnash
{

class GtkAggGlue : public GtkGlue
{
  public:
    GtkAggGlue();
    ~GtkAggGlue();

    bool init(int argc, char **argv[]);
    bool needsDrawingArea() { return true; };
    void prepDrawingArea(GtkWidget *drawing_area);
    Renderer* createRenderHandler();
    void setRenderHandlerSize(int width, int height);
    virtual void beforeRendering(movie_root* stage);
    void render();
    void render(int minx, int miny, int maxx, int maxy);
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);

  private:
    GdkImage* _offscreenbuf;
    Renderer *_agg_renderer;
};

} // namespace gnash

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
