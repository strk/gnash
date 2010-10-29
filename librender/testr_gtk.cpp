// 
//   Copyright (C) 2010 Free Software Foundation, Inc
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

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <cassert>
#include <regex.h>
#include <boost/assign/list_of.hpp>

#ifdef HAVE_GTK2
# include <gtk/gtk.h>
# include <gdk/gdk.h>
#endif
#ifdef RENDERER_AGG
#include "Renderer_agg.h"
#endif
#ifdef RENDERER_OPENGL
#include "Renderer_ogl.h"
#endif
#ifdef RENDERER_OPENVG
#include "Renderer_ovg.h"
#include <VG/openvg.h>
#include <VG/vgu.h>
#endif
#ifdef RENDERER_GLES1
#include "Renderer_gles1.h"
#endif
#ifdef RENDERER_GLES2
#include "Renderer_gles2.h"
#endif
#ifdef RENDERER_CAIRO
#include "Renderer_cairo.h"
#endif

#include "log.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"

using namespace gnash; 
using namespace std; 

const VGfloat white_color[4] = {1.0, 1.0, 1.0, 1.0};
const VGfloat color[4] = {0.4, 0.1, 1.0, 1.0};

VGPath path;
VGPaint paint;

// The debug log used by all the gnash libraries.
static LogFile& dbglogfile = LogFile::getDefaultInstance();

void on_window1_activate_default(GtkWindow *window, gpointer user_data);
void on_canvas_realize (GtkWidget *widget, gpointer user_data);
gboolean on_canvas_expose_event (GtkWidget *widget, GdkEventExpose *event,
                                        gpointer user_data);
gboolean on_canvas_configure_event (GtkWidget *widget, GdkEventConfigure *event,
                                          gpointer user_data);
void on_canvas_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
                                        gpointer user_data);
gboolean on_window1_configure_event (GtkWidget *widget, GdkEventConfigure *event,
                                     gpointer user_data);
gboolean on_window1_expose_event (GtkWidget *widget, GdkEventExpose *event,
                                        gpointer user_data);
void on_window1_realize (GtkWidget *widget, gpointer user_data);
void on_window1_size_allocate (GtkWidget *widget, GdkRectangle *allocation,
                                        gpointer user_data);

static void
initVGDemo(void)
{
    VGfloat clearColor[] = {0.0f, 0.0f, 0.0f, 1.0f};/* black color */
    VGfloat greenColor[] = {0.0f, 1.0f, 0.0f, 1.0f};/* green color */
    VGint arcType = VGU_ARC_OPEN;
    VGfloat x, y, w, h, startAngle, angleExtent;

    x = 150;
    y = 150;
    w = 150;
    h = 150;
#if 0
    startAngle  = -540.0f;
    angleExtent = 270.0f;
#else
    startAngle  = 270.0f;
    angleExtent = 90.0f;
#endif
    paint = vgCreatePaint();

    vgSetPaint(paint, VG_STROKE_PATH);
    vgSetParameterfv(paint, VG_PAINT_COLOR, 4, greenColor);
    vgSetParameteri( paint, VG_PAINT_TYPE, VG_PAINT_TYPE_COLOR);
    vgSetf(VG_STROKE_LINE_WIDTH, 6.0f);
    vgSeti(VG_RENDERING_QUALITY, VG_RENDERING_QUALITY_NONANTIALIASED);
    vgSetfv(VG_CLEAR_COLOR, 4, clearColor);

    path  = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
                         1.0f, 0.0f, 0, 0, VG_PATH_CAPABILITY_ALL);

    vguArc(path, x, y, w, h, startAngle, angleExtent, (VGUArcType)arcType);

    vgSeti(VG_STROKE_CAP_STYLE, VG_CAP_BUTT);
    vgSeti(VG_STROKE_JOIN_STYLE, VG_JOIN_BEVEL);
    vgSetf(VG_STROKE_MITER_LIMIT, 4.0f);
}

/* new window size or exposure */
static void
reshape(int w, int h)
{
   vgLoadIdentity();
}

static void
draw(void)
{
   vgClear(0, 0, 500, 500);
   vgDrawPath(path, VG_STROKE_PATH);

   vgFlush();
}

GtkWidget *
create_GTK_window()
{
    // As gtk_init() wants the command line arguments, we have to create
    // fake ones, as we don't care about the X11 options at this point.
    int argc = 0;
    char **argv = 0;    
    gtk_init(&argc, &argv);

    GtkWidget *window;
    GtkWidget *canvas;

    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_title(GTK_WINDOW(window), "testR");

#if 0
    gtk_widget_push_visual(gdk_rgb_get_visual());
    gtk_widget_push_colormap(gdk_rgb_get_cmap());
    gtk_widget_pop_colormap();
    gtk_widget_pop_visual();
#endif
    canvas = gtk_drawing_area_new();
    gtk_widget_show(canvas);
    gtk_container_add (GTK_CONTAINER (window), canvas);
    // Disable double buffering, otherwise gtk tries to update widget
    // contents from its internal offscreen buffer at the end of expose event
    gtk_widget_set_double_buffered(canvas, FALSE);
    
    g_signal_connect(GTK_OBJECT(window), "delete_event",
                     (GCallback)gtk_main_quit, NULL);
    g_signal_connect ((gpointer) window, "configure_event",
                      G_CALLBACK (on_window1_configure_event), NULL);
    g_signal_connect ((gpointer) window, "expose_event",
                    G_CALLBACK (on_window1_expose_event), NULL);
    g_signal_connect ((gpointer) window, "realize",
                      G_CALLBACK (on_window1_realize), NULL);
    g_signal_connect ((gpointer) window, "size_allocate",
                      G_CALLBACK (on_window1_size_allocate), NULL);
    
    g_signal_connect ((gpointer) canvas, "realize",
                      G_CALLBACK (on_canvas_realize), NULL);
    g_signal_connect ((gpointer) canvas, "expose_event",
                      G_CALLBACK (on_canvas_expose_event), NULL);
    g_signal_connect ((gpointer) canvas, "configure_event",
                      G_CALLBACK (on_canvas_configure_event), NULL);
    g_signal_connect ((gpointer) canvas, "size_allocate",
                      G_CALLBACK (on_canvas_size_allocate), NULL);

    // Add the window frame and icons
    gtk_window_set_decorated(GTK_WINDOW(window), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(window),500,500);

    gtk_window_set_skip_taskbar_hint(GTK_WINDOW(window), TRUE);
    gtk_window_set_skip_pager_hint(GTK_WINDOW(window), TRUE);

    // Open window at this location
    gtk_window_move(GTK_WINDOW(window), 100, 100);
    
    gtk_widget_show(window);
    
    // Just do one iteration of the main loop
    gtk_main_iteration_do(true);
    
    return canvas;
}

void
on_canvas_realize(GtkWidget *widget, gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
}

gboolean
on_canvas_expose_event(GtkWidget *widget, GdkEventExpose  *event,
                       gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
    
    return FALSE;
}

gboolean
on_canvas_configure_event(GtkWidget *widget, GdkEventConfigure *event,
                          gpointer user_data)
{
    GNASH_REPORT_FUNCTION;

    return FALSE;
}


void
on_canvas_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
                        gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
}

// Window Events
gboolean
on_window1_configure_event(GtkWidget *widget, GdkEventConfigure *event,
                           gpointer user_data)
{
    GNASH_REPORT_FUNCTION;

    return FALSE;
}


gboolean
on_window1_expose_event(GtkWidget *widget, GdkEventExpose  *event,
                        gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
    vgLoadIdentity();
    
    return FALSE;
}

void
on_window1_realize(GtkWidget *widget, gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
}

void
on_window1_size_allocate(GtkWidget *widget, GdkRectangle *allocation,
                         gpointer user_data)
{
    GNASH_REPORT_FUNCTION;
}


// Local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
