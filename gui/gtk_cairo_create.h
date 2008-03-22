/* GIMP Drawing Kit
 * Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <gdk/gdkdrawable.h>

#include <cairo-xlib.h>
#include <gtk/gtk.h>
#ifndef _WIN32
#include <gdk/gdkx.h>
#else
#include <gdk/gdk.h>
#endif


/* copied from gtk+/gdk/gdkcairo.c and gtk+/gdk/x11/gdkdrawable-x11.c
 * gdk_cairo_create() is be available in gtk 2.8
 */
static cairo_t *
gdk_cairo_create (GdkDrawable *target)
{
    int width, height;
    int x_off=0, y_off=0;
    cairo_t *cr;
    cairo_surface_t *surface;
    GdkDrawable *drawable = target;
    GdkVisual *visual;

    if (GDK_IS_WINDOW(target)) {
        /* query the window's backbuffer if it has one */
	GdkWindow *window = GDK_WINDOW(target);
	gdk_window_get_internal_paint_info (window,
					    &drawable, &x_off, &y_off);
    }
    visual = gdk_drawable_get_visual (drawable);
    gdk_drawable_get_size (drawable, &width, &height);

    if (visual) {
	surface = cairo_xlib_surface_create (GDK_DRAWABLE_XDISPLAY (drawable),
					     GDK_DRAWABLE_XID (drawable),
					     GDK_VISUAL_XVISUAL (visual),
					     width, height);
    } else if (gdk_drawable_get_depth (drawable) == 1) {
	surface = cairo_xlib_surface_create_for_bitmap
	    (GDK_PIXMAP_XDISPLAY (drawable),
	     GDK_PIXMAP_XID (drawable),
	     GDK_SCREEN_XSCREEN (gdk_drawable_get_screen (drawable)),
	     width, height);
    } else {
	g_warning ("Using Cairo rendering requires the drawable argument to\n"
		   "have a specified colormap. All windows have a colormap,\n"
		   "however, pixmaps only have colormap by default if they\n"
		   "were created with a non-NULL window argument. Otherwise\n"
		   "a colormap must be set on them with "
		   "gdk_drawable_set_colormap");
	return NULL;
    }
    cairo_surface_set_device_offset (surface, -x_off, -y_off);
    cr = cairo_create (surface);
    cairo_surface_destroy (surface);
    return cr;
}
