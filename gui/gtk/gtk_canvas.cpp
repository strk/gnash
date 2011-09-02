// gnash-canvas.cpp: Gtk canvas widget for gnash
// 
//   Copyright (C) 2009, 2010, 2011 Free Software Foundation, Inc.
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

#include <string>

#include "gtk_canvas.h"
#include "Renderer.h"
#include "rc.h"
#include "log.h"
#include "gtk_glue.h"
#include "GnashException.h"

#ifdef HAVE_VA_VA_H
#include "vaapi_utils.h"
#endif

// OpenGL support for rendering in the canvas. This also requires
// the GtkGL widget for GTK2.
#ifdef HAVE_GTK_GTKGL_H
#include "gtk_glue_gtkglext.h"
#endif

// Cairo support for rendering in the canvas.
#ifdef RENDERER_CAIRO
#include "gtk_glue_cairo.h"
#endif

// Cairo support for rendering in the canvas.
#ifdef RENDERER_OPENVG
#include "gtk_glue_ovg.h"
#endif

// AGG support, which is the default, for rendering in the canvas.
#include "gtk_glue_agg.h"

#ifdef HAVE_VA_VA_H
#include "gtk_glue_agg_vaapi.h"
#endif

struct _GnashCanvas
{
    GtkDrawingArea base_instance;
    std::auto_ptr<gnash::GtkGlue> glue;
    boost::shared_ptr<gnash::Renderer> renderer;
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

namespace {
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

GtkWidget *
gnash_canvas_new ()
{
    return GTK_WIDGET(g_object_new (GNASH_TYPE_CANVAS, NULL));
}

static void
gnash_canvas_class_init(GnashCanvasClass *gnash_canvas_class)
{
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(gnash_canvas_class);

    parent_class = (GObjectClass *)g_type_class_peek_parent(gnash_canvas_class);

    widget_class->size_allocate = gnash_canvas_size_allocate;
    widget_class->expose_event = gnash_canvas_expose_event;
    widget_class->configure_event = gnash_canvas_configure_event;
    widget_class->realize = gnash_canvas_realize;
}

static void
gnash_canvas_init(GnashCanvas *canvas)
{

    canvas->renderer.reset();

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
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    gnash::log_debug("gnash_canvas_size_allocate %d %d", allocation->width,
            allocation->height);

    if (canvas->renderer.get()) {
        canvas->glue->setRenderHandlerSize(allocation->width,
                allocation->height);
    }
    
    GTK_WIDGET_CLASS(parent_class)->size_allocate (widget, allocation);
}

static gboolean
gnash_canvas_expose_event(GtkWidget *widget, GdkEventExpose *event)
{
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    // In some versions of GTK this can't be const...
    GdkRegion* nonconst_region = const_cast<GdkRegion*>(event->region);

    canvas->glue->render(nonconst_region);

    return TRUE;
}

static gboolean
gnash_canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    canvas->glue->configure(widget, event);

    return FALSE;
}

static void
gnash_canvas_realize(GtkWidget *widget)
{
    GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

    GdkWindowAttr attributes;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual (widget);
    attributes.colormap = gtk_widget_get_colormap (widget);
    attributes.event_mask = gtk_widget_get_events (widget) | GDK_EXPOSURE_MASK;

    gint attributes_mask =
        GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

    widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
                                    &attributes, attributes_mask);
    gdk_window_set_user_data (widget->window, widget);

#if defined(RENDERER_CAIRO) || defined(RENDERER_AGG)
    GnashCanvas *canvas = GNASH_CANVAS(widget);
    // cairo needs the _drawingArea.window to prepare it ..
    // TODO: find a way to make 'glue' use independent from actual
    // renderer in use
    canvas->glue->prepDrawingArea(GTK_WIDGET(canvas));
#endif
}

static void
gnash_canvas_after_realize(GtkWidget *widget)
{
    GnashCanvas *canvas = GNASH_CANVAS(widget);

    canvas->renderer.reset(canvas->glue->createRenderHandler());

    canvas->glue->setRenderHandlerSize(widget->allocation.width,
                                        widget->allocation.height);
}

void
gnash_canvas_setup(GnashCanvas *canvas, std::string& hwaccel,
        std::string& renderer, int argc, char **argv[])
{

    // Order should be VAAPI, Xv, X11
    bool initialized_renderer = false;

    // If a renderer hasn't been defined in gnashrc, or on the command
    // line, pick a sensible default.
    if (renderer.empty()) {
#ifdef RENDERER_AGG
        renderer = "agg";
#elif defined (RENDERER_CAIRO)
        renderer = "cairo";
#elif defined (RENDERER_OGL)
        renderer = "opengl";
#elif defined (RENDERER_OPENVG)
        renderer = "openvg";
#endif
    }

    // If the Hardware acceleration isn't defined in gnashrc, or on
    // the command line, pick a sensible default.
    if (hwaccel.empty()) {
        hwaccel = "none";
    }

#ifdef HAVE_VA_VA_H
    // Global enable VA-API, if requested
    gnash::vaapi_set_is_enabled(hwaccel == "vaapi");
#endif

    // Use the Cairo renderer. Cairo is also used by GTK2, so using
    // Cairo makes much sense. Unfortunately, our implementation seems
    // to have serious performance issues, although it does work.
    if (renderer == "cairo") {
#ifdef RENDERER_CAIRO
        canvas->glue.reset(new gnash::GtkCairoGlue);
#else
        boost::format fmt = boost::format("Support for renderer %1% "
                " was not built") % renderer;
        throw gnash::GnashException(fmt.str());
#endif
    }

    else if (renderer == "opengl") {
#ifdef RENDERER_OPENGL
        canvas->glue.reset(new gnash::GtkGlExtGlue);
#else
        boost::format fmt = boost::format("Support for renderer %1% "
                " was not built") % renderer;
        throw gnash::GnashException(fmt.str());
#endif
    }
    else if ((renderer == "openvg") || (renderer == "ovg")) {
	renderer = "openvg";
#ifdef RENDERER_OPENVG
        canvas->glue.reset(new gnash::gui::GtkOvgGlue);
#else
        boost::format fmt = boost::format("Support for renderer %1% "
                " was not built") % renderer;
        throw gnash::GnashException(fmt.str());
#endif
    }
    else if (renderer == "agg") {
        // Use the AGG software library for rendering. While this runs
        // on any hardware platform, it does have performance issues
        // on low-end platforms without a GPU. So while AGG may render
        // streaming video over a network connection just fine,
        // anything below about 600Mhz CPU may have buffering and
        // rendering performance issues.
        // Use LibVva, which works on Nvidia, AT, or Intel 965 GPUs
        // with AGG or OpenGL.
#ifdef RENDERER_AGG
#ifdef HAVE_VA_VA_H
        if (hwaccel == "vaapi") {
            canvas->glue.reset(new gnash::GtkAggVaapiGlue);
            // Set the hardware acclerator to the next one to try
            // if initializing fails.
        } else
#endif
        {
            canvas->glue.reset(new gnash::GtkAggGlue);
        }
#else // ndef RENDERER_AGG
        boost::format fmt = boost::format("Support for renderer %1% "
                "was not built") % renderer;
        throw gnash::GnashException(fmt.str());
#endif
    }
    else {
        boost::format fmt = boost::format("Non-existent renderer %1% "
            "specified") % renderer;
        throw gnash::GnashException(fmt.str());
    }

    // Initialize the canvas for rendering into
    initialized_renderer = canvas->glue->init(argc, argv);
    // If the renderer with the least dependencies fails, we can't
    // proceed.
    if (!initialized_renderer) {
        boost::format fmt = boost::format("Requested renderer %1% (hwaccel: "
                "%2%) could not be initialized") % renderer % hwaccel;
        throw gnash::GnashException(fmt.str());
    }
        
    if (renderer == "opengl") {
        // OpenGL glue needs to prepare the drawing area for OpenGL
        // rendering before
        // widgets are realized and before the configure event is fired.
        // TODO: find a way to make '_glue' use independent from
        // actual renderer in use
        canvas->glue->prepDrawingArea(GTK_WIDGET(canvas));
    }
}

void
gnash_canvas_before_rendering(GnashCanvas *canvas, gnash::movie_root* stage)
{
    canvas->glue->beforeRendering(stage);
}

boost::shared_ptr<gnash::Renderer>
gnash_canvas_get_renderer(GnashCanvas *canvas)
{
    return canvas->renderer;
}

