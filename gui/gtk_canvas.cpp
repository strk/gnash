// gnash-canvas.cpp: Gtk canvas widget for gnash
// 
//   Copyright (C) 2009 Free Software Foundation, Inc.
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

#include "gtk_canvas.h"

#include "rc.h"
#include "log.h"
#include "gtk_glue.h"

#ifdef RENDERER_OPENGL
#include "gtk_glue_gtkglext.h"
#endif

#ifdef RENDERER_CAIRO
#include "gtk_glue_cairo.h"
#endif

#ifdef RENDERER_AGG
#include "gtk_glue_agg.h"
#ifdef HAVE_XV
#include "gtk_glue_agg_xv.h"
#endif // HAVE_XV
#endif

struct _GnashCanvas {
	GtkDrawingArea base_instance;

    std::auto_ptr<gnash::GtkGlue> glue;
    gnash::Renderer *renderer;
};

G_DEFINE_TYPE(GnashCanvas, gnash_canvas, GTK_TYPE_DRAWING_AREA)

static GObjectClass *parent_class = NULL;

static void gnash_canvas_class_init(GnashCanvasClass *gnash_canvas_class);
static void gnash_canvas_init(GnashCanvas *canvas);
static void gnash_canvas_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static gboolean gnash_canvas_expose_event(GtkWidget *widget, GdkEventExpose *event);
static gboolean gnash_canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event);
static void gnash_canvas_realize(GtkWidget *widget);
static void gnash_canvas_after_realize(GtkWidget *widget);

GtkWidget *
gnash_canvas_new ()
{
    GNASH_REPORT_FUNCTION;
    return GTK_WIDGET(g_object_new (GNASH_TYPE_CANVAS, NULL));
}

static void
gnash_canvas_class_init(GnashCanvasClass *gnash_canvas_class)
{
    GNASH_REPORT_FUNCTION;
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(gnash_canvas_class);

    parent_class = (GObjectClass *) g_type_class_peek_parent(gnash_canvas_class);

    widget_class->size_allocate = gnash_canvas_size_allocate;
    widget_class->expose_event = gnash_canvas_expose_event;
    widget_class->configure_event = gnash_canvas_configure_event;
    widget_class->realize = gnash_canvas_realize;
}

static void
gnash_canvas_init(GnashCanvas *canvas)
{
    GNASH_REPORT_FUNCTION;

    canvas->renderer = NULL;

    gtk_widget_set_double_buffered(GTK_WIDGET(canvas), FALSE);

    g_signal_connect_after(G_OBJECT(canvas), "realize",
                           G_CALLBACK(gnash_canvas_after_realize), NULL);

    // If we don't set this flag we won't be able to grab focus
    // ( grabFocus() would be a no-op )
    GTK_WIDGET_SET_FLAGS (GTK_WIDGET(canvas), GTK_CAN_FOCUS);
}

static void
gnash_canvas_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    GNASH_REPORT_FUNCTION;
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    gnash::log_debug("gnash_canvas_size_allocate %d %d", allocation->width, allocation->height);

    if (canvas->renderer != NULL)
        canvas->glue->setRenderHandlerSize(allocation->width, allocation->height);
    
    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
}

static gboolean
gnash_canvas_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    GnashCanvas *canvas = GNASH_CANVAS(widget);
    gint num_rects;
    GdkRectangle* rects;

    // In some versions of GTK this can't be const...
    GdkRegion* nonconst_region = const_cast<GdkRegion*>(event->region);

    gdk_region_get_rectangles (nonconst_region, &rects, &num_rects);
    assert(num_rects);

    for (int i=0; i<num_rects; ++i) {
      const GdkRectangle& cur_rect = rects[i];
      canvas->glue->render(cur_rect.x, cur_rect.y, cur_rect.x + cur_rect.width,
                    cur_rect.y + cur_rect.height);
    }

    g_free(rects);

    return TRUE;
}

static gboolean
gnash_canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
    GNASH_REPORT_FUNCTION;
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    canvas->glue->configure(widget, event);

    return FALSE;
}

static void
gnash_canvas_realize(GtkWidget *widget)
{
    GNASH_REPORT_FUNCTION;
    GnashCanvas *canvas = GNASH_CANVAS(widget);
    GdkWindowAttr attributes;
    gint attributes_mask;

    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                    &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

#if defined(RENDERER_CAIRO) || defined(RENDERER_AGG)
    // cairo needs the _drawingArea.window to prepare it ..
    // TODO: find a way to make 'glue' use independent from actual renderer in use
    canvas->glue->prepDrawingArea(GTK_WIDGET(canvas));
#endif
}

static void
gnash_canvas_after_realize(GtkWidget *widget)
{
    GNASH_REPORT_FUNCTION;
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    canvas->renderer = canvas->glue->createRenderHandler();
    set_Renderer(canvas->renderer);

    canvas->glue->setRenderHandlerSize(widget->allocation.width,
                                        widget->allocation.height);
}

void
gnash_canvas_setup(GnashCanvas *canvas, int argc, char **argv[])
{

    GNASH_REPORT_FUNCTION;
    // TODO: don't rely on a macro to select renderer
#ifdef RENDERER_CAIRO
    canvas->glue.reset(new gnash::GtkCairoGlue);
#elif defined(RENDERER_OPENGL)
    canvas->glue.reset(new gnash::GtkGlExtGlue);
#elif defined(RENDERER_AGG) && !defined(HAVE_XV)
    canvas->glue.reset(new gnash::GtkAggGlue);
#elif defined(RENDERER_AGG) && defined(HAVE_XV)
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    if (rcfile.useXv()) {
        canvas->glue.reset(new gnash::GtkAggXvGlue);
        if (!canvas->glue->init (argc, argv)) {
            canvas->glue.reset(new gnash::GtkAggGlue);
            canvas->glue->init(argc, argv);
        }
    } else {
        canvas->glue.reset(new gnash::GtkAggGlue);
        canvas->glue->init(argc, argv);
    }
#endif

#if ! (defined(HAVE_XV) && defined(RENDERER_AGG))
    canvas->glue->init (argc, argv);
#endif

#ifdef RENDERER_OPENGL
    // OpenGL glue needs to prepare the drawing area for OpenGL rendering before
    // widgets are realized and before the configure event is fired.
    // TODO: find a way to make '_glue' use independent from actual renderer in use
    canvas->glue->prepDrawingArea(GTK_WIDGET(canvas));
#endif
}

void
gnash_canvas_before_rendering(GnashCanvas *canvas)
{
    canvas->glue->beforeRendering();
}

gnash::Renderer *
gnash_canvas_get_renderer(GnashCanvas *canvas)
{
    return canvas->renderer;
}

