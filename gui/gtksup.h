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
// 
// Linking Gnash statically or dynamically with other modules is making
// a combined work based on Gnash. Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
// 
// In addition, as a special exception, the copyright holders of Gnash give
// you permission to combine Gnash with free software programs or
// libraries that are released under the GNU LGPL and/or with Mozilla, 
// so long as the linking with Mozilla, or any variant of Mozilla, is
// through its standard plug-in interface. You may copy and distribute
// such a system following the terms of the GNU GPL for Gnash and the
// licenses of the other code concerned, provided that you include the
// source code of that other code when and as the GNU GPL requires
// distribution of source code. 
// 
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is
// their choice whether to do so.  The GNU General Public License gives
// permission to release a modified version without this exception; this
// exception also makes it possible to release a modified version which
// carries forward this exception.
//

#ifndef __GTKSUP_H__
#define __GTKSUP_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <gtk/gtkgl.h>

#include <gtk/gtk.h>

#include "log.h"
#include "gui.h"

namespace gnash
{

//typedef void (*callback_t)(int x);

class GtkGui : public Gui
{
 public:
    GtkGui(unsigned long xid, float scale, bool loop, unsigned int depth);
    virtual ~GtkGui();
    virtual bool init(int argc, char **argv[]);
    virtual bool createWindow(int width, int height);
    virtual bool run();    
    virtual void resizeWindow();
    virtual bool createMenu();
    virtual bool setupEvents();
    virtual void drawTestGraphic();
    virtual void renderBuffer();
    virtual void setCallback(callback_t f, unsigned int interval);
    virtual void setTimeout(unsigned int timeout);
#if defined(RENDERER_OPENGL) && defined(FIX_I810_LOD_BIAS)
    virtual void setLodBias(float tex_lod_bias);
#endif


    // Menu Item callbacks

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
    static gboolean button_press_event(GtkWidget *widget, GdkEventButton *event,
                                gpointer data);
    static gboolean button_release_event(GtkWidget *widget, GdkEventButton *event,
                                  gpointer data);
    static gboolean motion_notify_event(GtkWidget *widget, GdkEventMotion *event,
                                 gpointer data);
    static gint popup_handler(GtkWidget *widget, GdkEvent *event);
    
    // GtkGLExt utility functions
    void print_gl_config_attrib (GdkGLConfig *glconfig,
                                 const gchar *attrib_str,
                                 int attrib, gboolean is_boolean);
    void examine_gl_config_attrib (GdkGLConfig *glconfig);
private:
    GtkWidget   *_window;
    GtkWidget   *_drawing_area;    
    GtkMenu     *_popup_menu;
#ifdef RENDERER_CAIRO
    cairo_t     *_cairo_handle;
#elif defined(RENDERER_OPENGL)
    GdkGLConfig *_glconfig;
#endif

};


// end of namespace gnash 
}

// end of __GTKSUP_H__
#endif
