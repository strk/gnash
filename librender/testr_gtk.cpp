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

#include "log.h"
#include "SWFMatrix.h"
#include "Renderer.h"
#include "Transform.h"
#include "GnashVaapiImage.h"
#include "GnashVaapiImageProxy.h"

using namespace gnash; 
using namespace std; 

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

GtkWidget *
create_GTK_window(int argc, char **argv)
{
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
    
    return window;
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
