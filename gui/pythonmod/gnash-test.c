// gnash-window.c: Gtk canvas widget for gnash
// 
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

#include <gtk/gtk.h>

#include "gnash-view.h"

static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

int main( int   argc,
          char *argv[] )
{
    GtkWidget *window;
    GtkWidget *view;
    
    gtk_init (&argc, &argv);
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    
    g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);
        
    view = gnash_view_new ();

    gtk_container_add (GTK_CONTAINER (window), view);
    gtk_widget_show (view);
    
    gtk_widget_show (window);

    gnash_view_load_movie(GNASH_VIEW(view), argv[1]);
    gnash_view_start(GNASH_VIEW(view));

    gtk_main ();
    
    return 0;
}
