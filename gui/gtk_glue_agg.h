//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

//
//

#include "gtk_glue.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace gnash
{

class GtkAggGlue : public GtkGlue
{
  public:
    GtkAggGlue();
    ~GtkAggGlue();

    bool init(int argc, char **argv[]);
    void prepDrawingArea(GtkWidget *drawing_area);
    render_handler* createRenderHandler();
    void setRenderHandlerSize(int width, int height);
    void render();
    void render(int minx, int miny, int maxx, int maxy);
    void configure(GtkWidget *const widget, GdkEventConfigure *const event);
    
  private:
    unsigned char *_offscreenbuf;
    int _offscreenbuf_size;
    render_handler *_agg_renderer;
    int _width, _height, _bpp;
};

} // namespace gnash
