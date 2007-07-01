//
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
    _drawing_area = 0;
    _cairo_handle = 0;
    _cairo_offscreen = 0;
}

GtkCairoGlue::~GtkCairoGlue()
{
    if (_cairo_handle)  cairo_destroy(_cairo_handle);
    if (_cairo_offscreen)  cairo_destroy(_cairo_offscreen);
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
}

render_handler*
GtkCairoGlue::createRenderHandler()
{
    return renderer::cairo::create_handler();
}

void
GtkCairoGlue::render()
{
    if (!_cairo_offscreen)  return;

    // Blit offscreen image onto output window 
    cairo_set_source_surface(_cairo_handle,
    	cairo_get_target(_cairo_offscreen), 0, 0);
    cairo_paint(_cairo_handle);
}

void
GtkCairoGlue::configure(GtkWidget *const /*widget*/,
    GdkEventConfigure *const event)
{
    if (!_drawing_area)  return;

    // Create cairo handle for output window
    if (_cairo_handle)  cairo_destroy(_cairo_handle);
    _cairo_handle = gdk_cairo_create(_drawing_area->window);
    assert(_cairo_handle);

    // Create offscreen image for rendering
    if (_cairo_offscreen)  cairo_destroy(_cairo_offscreen);
    cairo_surface_t* surface = cairo_image_surface_create(
	CAIRO_FORMAT_RGB24, event->width, event->height);
    _cairo_offscreen = cairo_create(surface);
    assert(_cairo_offscreen);
    cairo_surface_destroy(surface);
    renderer::cairo::set_handle(_cairo_offscreen);
}



} // namespace gnash

