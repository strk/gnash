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



/// \page gtk_shm_support GTK shared memory extension support
/// 
/// We use GdkImage to manage the rendering buffer for us. It supports
/// automatic detection and usage of MIT-ShM, so we don't have to worry about
/// it beyond synchronising the screen before rendering.
 
#include <cerrno>
#include <exception>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "log.h"
#include "Renderer.h"
#include "Renderer_agg.h"
#include "gtk_glue_agg.h"
#include "GnashException.h"

namespace gnash
{

GtkAggGlue::GtkAggGlue()
:   _offscreenbuf(NULL),
    _agg_renderer(NULL)
{
    GNASH_REPORT_FUNCTION;
}

GtkAggGlue::~GtkAggGlue()
{
    if (_offscreenbuf) {
        gdk_image_destroy(_offscreenbuf);
    }
}

bool
GtkAggGlue::init(int /*argc*/, char ** /*argv*/[])
{
    GNASH_REPORT_FUNCTION;
    return true;
}

void
GtkAggGlue::prepDrawingArea(GtkWidget *drawing_area)
{
    GNASH_REPORT_FUNCTION;
    _drawing_area = drawing_area;

    // Disable double buffering, otherwise gtk tries to update widget
    // contents from its internal offscreen buffer at the end of expose event
    gtk_widget_set_double_buffered(_drawing_area, FALSE);
}

Renderer*
GtkAggGlue::createRenderHandler()
{
    GNASH_REPORT_FUNCTION;
    GdkVisual* wvisual = gdk_drawable_get_visual(_drawing_area->window);

    GdkImage* tmpimage = gdk_image_new (GDK_IMAGE_FASTEST, wvisual, 1, 1);

    const GdkVisual* visual = tmpimage->visual;

    // FIXME: we use bpp instead of depth, because depth doesn't appear to
    // include the padding byte(s) the GdkImage actually has.
    const char *pixelformat = agg_detect_pixel_format(
        visual->red_shift, visual->red_prec,
        visual->green_shift, visual->green_prec,
        visual->blue_shift, visual->blue_prec,
        tmpimage->bpp * 8);

    gdk_image_destroy(tmpimage);

    _agg_renderer = create_Renderer_agg(pixelformat);
    if (! _agg_renderer) {
        boost::format fmt = boost::format(
            _("Could not create AGG renderer with pixelformat %s")
            ) % pixelformat;
        throw GnashException(fmt.str());
    }

    return _agg_renderer;
}

void
GtkAggGlue::setRenderHandlerSize(int width, int height)
{
    GNASH_REPORT_FUNCTION;

    assert(width > 0);
    assert(height > 0);
    assert(_agg_renderer != NULL);
    
    if (_offscreenbuf && _offscreenbuf->width == width &&
        _offscreenbuf->height == height) {
        return; 
    }

    if (_offscreenbuf) {
        gdk_image_destroy(_offscreenbuf);
    }

    GdkVisual* visual = gdk_drawable_get_visual(_drawing_area->window);

    _offscreenbuf = gdk_image_new (GDK_IMAGE_FASTEST, visual, width,
                                   height);

   	static_cast<Renderer_agg_base *>(_agg_renderer)->init_buffer(
        (unsigned char*) _offscreenbuf->mem,
        _offscreenbuf->bpl * _offscreenbuf->height,
        _offscreenbuf->width,
        _offscreenbuf->height,
        _offscreenbuf->bpl); 
}

void 
GtkAggGlue::beforeRendering(movie_root*)
{
    if (_offscreenbuf && _offscreenbuf->type == GDK_IMAGE_SHARED) {
         gdk_flush();
    }
}

void
GtkAggGlue::render()
{
    render(0, 0, _offscreenbuf->width, _offscreenbuf->height);
}


void
GtkAggGlue::render(int minx, int miny, int maxx, int maxy)
{
     if (!_offscreenbuf) {
         return;
     }

     const int& x = minx;
     const int& y = miny;
     size_t width = std::min(_offscreenbuf->width, maxx - minx);
     size_t height = std::min(_offscreenbuf->height, maxy - miny);

     GdkGC* gc = gdk_gc_new(_drawing_area->window);
    
     gdk_draw_image(_drawing_area->window, gc, _offscreenbuf, x, y, x, y, width,
                    height);
     gdk_gc_unref(gc);
}

void
GtkAggGlue::configure(GtkWidget *const /*widget*/, GdkEventConfigure *const event)
{
    if (_agg_renderer) {
        setRenderHandlerSize(event->width, event->height);
    }
}

} // namespace gnash

