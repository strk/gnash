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

#include "log.h"
#include "gtk_glue_cairo.h"
#include "Renderer_cairo.h"

#if GTK_MAJOR_VERSION == 2 && GTK_MINOR_VERSION < 8
# include "gtk_cairo_create.h"
#endif


namespace gnash
{

GtkCairoGlue::GtkCairoGlue()
  : _cairo_handle(0),
    _cairo_offscreen(0),
    _image(0)
{
    GNASH_REPORT_FUNCTION;
}

GtkCairoGlue::~GtkCairoGlue()
{
    if (_cairo_handle)  cairo_destroy(_cairo_handle);
    if (_cairo_offscreen)  cairo_destroy(_cairo_offscreen);
    if (_image) gdk_image_destroy(_image);
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
    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

Renderer*
GtkCairoGlue::createRenderHandler()
{
    _renderer = renderer::cairo::create_handler();

    return _renderer;
}

void 
GtkCairoGlue::beforeRendering()
{
  if (_image && _image->type == GDK_IMAGE_SHARED) {
    gdk_flush();
  }
}

void
GtkCairoGlue::render(int minx, int miny, int maxx, int maxy)
{
    if (!_cairo_offscreen) {
      return;
    }

    const int& x = minx;
    const int& y = miny;
    int width = maxx - minx;
    int height = maxy - miny;

    if (_image) {
      // Using GdkImage for our image buffer, use GdkRgb (potentially shm).
      GdkGC* gc = gdk_gc_new(_drawing_area->window);

      gdk_draw_image(_drawing_area->window, gc, _image, x, y, x, y, width,
                     height);
      gdk_gc_unref(gc);
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

bool
cairoFormatFromVisual(const GdkVisual* visual, cairo_format_t* format /*out*/)
{
  switch(visual->depth) {
    case 24:
      *format = CAIRO_FORMAT_RGB24;
      break;
    case 32:
      *format = CAIRO_FORMAT_ARGB32;
      break;
    default:
      format = NULL;
      return false;
  }
  return true;
}

cairo_surface_t*
GtkCairoGlue::createGdkImageSurface(const int& width, const int& height)
{
  GdkVisual* visual = gdk_drawable_get_visual(_drawing_area->window);
  assert(_drawing_area);
  assert(visual);
  cairo_format_t format;

  if (!cairoFormatFromVisual(visual, &format)) {
    return NULL;
  }

  _image = gdk_image_new (GDK_IMAGE_FASTEST, visual, width, height);
  if (!_image) {
    return NULL;
  }

  cairo_surface_t* surface =
    cairo_image_surface_create_for_data ((unsigned char *)_image->mem,
      format, _image->width, _image->height, _image->bpl);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(surface);
    gdk_image_destroy(_image);
    _image = 0;
    return NULL;
  }

  return surface;
}

cairo_surface_t*
GtkCairoGlue::createSimilarSurface(const int& width, const int& height)
{
  cairo_surface_t* target = cairo_get_target(_cairo_handle);

  cairo_surface_t* surface = cairo_surface_create_similar(target,
    cairo_surface_get_content(target), width, height);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(surface);
    return NULL;
  }
  return surface;
}

cairo_surface_t*
GtkCairoGlue::createMemorySurface(const int& width, const int& height)
{
  cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
                                                        width, height);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    cairo_surface_destroy(surface);
    return NULL;
  }
  return surface;
}

void
GtkCairoGlue::configure(GtkWidget *const /*widget*/,
    GdkEventConfigure *const event)
{
    if (!_drawing_area)  return;

    if (_image) {
      gdk_image_destroy(_image);
      _image = 0;
    }

    cairo_surface_t* surface = createGdkImageSurface(event->width, event->height);

    if (!surface) {

      if (!_cairo_handle) {
        _cairo_handle = gdk_cairo_create(_drawing_area->window);
      }

      surface = createMemorySurface(event->width, event->height);
    }

    if (!surface) {
      surface = createSimilarSurface(event->width, event->height);
    }

    if (!surface) {
      log_error("Cairo: failed to create a rendering buffer!");
      return;
    }

    _cairo_offscreen = cairo_create(surface);
    cairo_surface_destroy(surface);
    
    renderer::cairo::set_context(_renderer, _cairo_offscreen);
}

} // namespace gnash

