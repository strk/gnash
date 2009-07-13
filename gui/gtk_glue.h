//
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "gnash.h"

#include <gtk/gtk.h>
#include <gdk/gdk.h>

namespace gnash
{

class GtkGlue
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

    virtual void configure(GtkWidget *const widget,
            GdkEventConfigure *const event) = 0;
    
    virtual void beforeRendering() {};

  protected:
    GtkWidget *_drawing_area;
};

} // namespace gnash

// end of GNASH_GTK_GLUE_H
#endif
