//
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#include "gtk_glue_cairo.h"
#include "render_handler_cairo.h"

#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 8
# include "gtk_cairo_create.h"
#endif


namespace gnash
{

GtkCairoGlue::GtkCairoGlue()
{
}

GtkCairoGlue::~GtkCairoGlue()
{
    cairo_destroy(_cairo_handle);
}

bool
GtkCairoGlue::init(int /*argc*/, char *** /*argv*/)
{
    return true;
}

void
GtkCairoGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    _drawing_area = drawing_area;
    assert(_drawing_area);
    assert(_drawing_area->window);
    _cairo_handle = gdk_cairo_create (_drawing_area->window);
    assert(_cairo_handle);
    renderer::cairo::set_handle(_cairo_handle);
}

render_handler*
GtkCairoGlue::createRenderHandler()
{
    //_cairo_handle = gdk_cairo_create (_drawing_area->window);
    //return create_render_handler_cairo((void*)_cairo_handle);
    return renderer::cairo::create_handler();
}

void
GtkCairoGlue::render()
{
}

void
GtkCairoGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const /*event*/)
{
}



} // namespace gnash

