//
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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
    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

render_handler*
GtkCairoGlue::createRenderHandler()
{
    _renderer = renderer::cairo::create_handler();

    return _renderer;
}

void
GtkCairoGlue::render(int minx, int miny, int maxx, int maxy)
{
    if (!_cairo_offscreen) {
      return;
    }

    cairo_save(_cairo_offscreen);

    cairo_rectangle(_cairo_offscreen, minx, miny, maxx - minx, maxy - miny);
    cairo_clip(_cairo_offscreen);

    render();

    cairo_restore(_cairo_offscreen);
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
    _cairo_handle = gdk_cairo_create(_drawing_area->window);

    cairo_surface_t* target = cairo_get_target(_cairo_handle);

    cairo_surface_t* surface = cairo_surface_create_similar(target,
      cairo_surface_get_content(target), event->width, event->height);

    if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
      // fallback image surface
      surface = cairo_image_surface_create(
        CAIRO_FORMAT_ARGB32, event->width, event->height);
    }

    _cairo_offscreen = cairo_create(surface);
    cairo_surface_destroy(surface);
    
    renderer::cairo::set_context(_renderer, _cairo_offscreen);
}



} // namespace gnash

