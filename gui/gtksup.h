// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef __GTKSUP_H__
#define __GTKSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
#include "tu_config.h"

#include <gdk/gdkx.h>
#include <gtk/gtk.h>

#ifdef RENDERER_OPENGL
#include <gtk/gtkgl.h>
#include "gtk_glue_gtkglext.h"
#elif defined(RENDERER_CAIRO)
#include <cairo.h>
#include "gtk_glue_cairo.h"
#elif defined(RENDERER_AGG)
#include "gtk_glue_agg.h"
#endif

#include <gtk/gtk.h>

#include "gui.h"


namespace gnash
{

//typedef void (*callback_t)(int x);

class DSOEXPORT GtkGui : public Gui
{
 public:
    GtkGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~GtkGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool createWindow(const char *title, int width, int height);
    virtual bool run();    
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void renderBuffer();
    virtual void setInterval(unsigned int interval);
    virtual void setTimeout(unsigned int timeout);

    // Menu Item callbacks

    static void menuitem_sound_callback(GtkMenuItem *menuitem,
                                   gpointer instance);
    static void menuitem_restart_callback(GtkMenuItem *menuitem,
                                   gpointer instance);
    static void menuitem_quit_callback(GtkMenuItem *menuitem,
                                       gpointer instance);
    static void menuitem_play_callback(GtkMenuItem *menuitem,
                                       gpointer instance);
    static void menuitem_pause_callback(GtkMenuItem *menuitem,
                                        gpointer instance);
    static void menuitem_stop_callback(GtkMenuItem *menuitem,
                                       gpointer instance);
    static void menuitem_step_forward_callback(GtkMenuItem *menuitem,
                                        gpointer instance);
    static void menuitem_step_backward_callback(GtkMenuItem *menuitem,
                                         gpointer instance);
    static void menuitem_jump_forward_callback(GtkMenuItem *menuitem,
                                        gpointer instance);
    static void menuitem_jump_backward_callback(GtkMenuItem *menuitem,
                                         gpointer instance);

    // GTK Event handlers
    static gboolean unrealize_event(GtkWidget *widget, GdkEvent *event,
                                    gpointer data);
    static gboolean realize_event(GtkWidget *widget, GdkEvent *event,
                                  gpointer data);
    static gboolean delete_event(GtkWidget *widget, GdkEvent *event,
                                 gpointer data);
    static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event,
                                 gpointer data);
    static gboolean configure_event(GtkWidget *widget, GdkEventConfigure *event,
                                    gpointer data);
    static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event,
                                    gpointer data);
    static gboolean key_release_event(GtkWidget *widget, GdkEventKey *event,
                                    gpointer data);
    static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event,
                                       gpointer data);
    static gboolean button_release_event(GtkWidget *widget, GdkEventButton *event,
                                         gpointer data);
    static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
                                        gpointer data);
    static gint popup_handler(GtkWidget *widget, GdkEvent *event);    

    void add_pixmap_directory(const gchar *directory);

    gchar* find_pixmap_file(const gchar *filename);

    GdkPixbuf* create_pixbuf(const gchar *filename);
    
    void set_invalidated_region(const rect& bounds);

    virtual void setCursor(gnash_cursor_type newcursor);

 private:
    GtkWidget   *_window;
    GdkPixbuf 	*_window_icon_pixbuf;
    GtkWidget   *_drawing_area;    
    GtkMenu     *_popup_menu;
    int 				m_draw_minx;
    int 				m_draw_miny;
    int 				m_draw_maxx;
    int 				m_draw_maxy;

  	int valid_coord(int coord, int max);
#ifdef RENDERER_CAIRO
    cairo_t     *_cairo_handle;
    GtkCairoGlue glue;
#elif defined(RENDERER_OPENGL)
    GdkGLConfig *_glconfig;
    GtkGlExtGlue glue;
#elif defined(RENDERER_AGG)
    GtkAggGlue  glue;
#endif
    static gnash::key::code gdk_to_gnash_key(guint key);
};


// end of namespace gnash 
}

// end of __GTKSUP_H__
#endif
