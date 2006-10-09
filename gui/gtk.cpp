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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gnash.h"
//#include "movie_definition.h" 
#include "log.h"

#include "gui.h"
#include "gtksup.h"

#include <iostream>
#include <X11/keysym.h>

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>


using namespace std;

namespace gnash 
{

GtkGui::~GtkGui()
{
}

GtkGui::GtkGui(unsigned long xid, float scale, bool loop, unsigned int depth)
 : Gui(xid, scale, loop, depth)
{
}


bool
GtkGui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;


    gtk_init (&argc, argv);

    glue.init (argc, argv);

    add_pixmap_directory (PKGDATADIR);

    if (_xid) {
      _window = gtk_plug_new(_xid);
      dbglogfile << "Created XEmbedded window" << endl;
    } else {
      _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
      dbglogfile << "Created top level window" << endl;
    }

    // XXXbjacques: why do we need this?
    gtk_container_set_reallocate_redraws(GTK_CONTAINER (_window), TRUE);

    _window_icon_pixbuf = create_pixbuf ("gnash_128_96.ico");
    if (_window_icon_pixbuf) {
        gtk_window_set_icon (GTK_WINDOW (_window), _window_icon_pixbuf);
	gdk_pixbuf_unref (_window_icon_pixbuf);
    }
    _drawing_area = gtk_drawing_area_new();

    createMenu();
#ifdef RENDERER_OPENGL
    // OpenGL glue needs to prepare the drawing area for OpenGL rendering before
    // widgets are realized and before the configure event is fired.
    glue.prepDrawingArea(_drawing_area);
#endif
    setupEvents();

    gtk_widget_realize(_window);
    gtk_container_add(GTK_CONTAINER(_window), _drawing_area);
    gtk_widget_show(_drawing_area);
    gtk_widget_show(_window);

#ifdef RENDERER_CAIRO
    // cairo needs the _drawing_area.window to prepare it ..
    glue.prepDrawingArea(_drawing_area);
#endif


    _renderer = glue.createRenderHandler();
    set_render_handler(_renderer);

    return true;
}


bool
GtkGui::createWindow(const char *title, int width, int height)
{
//First call the old createWindow function and then set the title.
//In case there's some need to not setting the title.

    bool ret = createWindow(width, height);
    gtk_window_set_title(GTK_WINDOW(_window), title);

    if (!_xid) {
      gtk_widget_set_size_request(_drawing_area, width, height);
    }

    return ret;
}

static GList *pixmaps_directories = NULL;

/* Use this function to set the directory containing installed pixmaps. */
void
GtkGui::add_pixmap_directory                   (const gchar     *directory)
{
    pixmaps_directories = g_list_prepend (pixmaps_directories, g_strdup (directory));
}


/* This is an internally used function to find pixmap files. */
gchar*
GtkGui::find_pixmap_file                       (const gchar     *filename)
{
    GList *elem;

    /* We step through each of the pixmaps directory to find it. */
    elem = pixmaps_directories;
    while (elem) {
        gchar *pathname = g_strdup_printf ("%s%s%s", (gchar*)elem->data,
                G_DIR_SEPARATOR_S, filename);
        if (g_file_test (pathname, G_FILE_TEST_EXISTS))
            return pathname;
        g_free (pathname);
        elem = elem->next;
    }
    return NULL;
}



/* This is an internally used function to create pixmaps. */
GdkPixbuf*
GtkGui::create_pixbuf                          (const gchar     *filename)
{
    gchar *pathname = NULL;
    GdkPixbuf *pixbuf;
    GError *error = NULL;

    if (!filename || !filename[0])
       return NULL;

    pathname = find_pixmap_file (filename);

    if (!pathname) {
        dbglogfile << "Couldn't find pixmap file: " << filename << endl;
        g_warning ("Couldn't find pixmap file: %s", filename);
        return NULL;
    }

    pixbuf = gdk_pixbuf_new_from_file (pathname, &error);
    if (!pixbuf) {
        dbglogfile << "Failed to load pixbuf file: " <<pathname << error->message << endl;
        //fprintf (stderr, "Failed to load pixbuf file: %s: %s\n", pathname, error->message);
        g_error_free (error);
    }
    g_free (pathname);
    return pixbuf;
}


bool
GtkGui::createWindow(int width, int height)
{
    GNASH_REPORT_FUNCTION;
    _width = width;
    _height = height;

    return true;
}

void
GtkGui::renderBuffer()
{
    glue.render();
}

void
GtkGui::setTimeout(unsigned int timeout)
{
    g_timeout_add(timeout, (GSourceFunc)gtk_main_quit, NULL);
}

void
GtkGui::setInterval(unsigned int interval)
{
    _interval = interval;
    g_timeout_add_full (G_PRIORITY_LOW, interval, (GSourceFunc)advance_movie,
                        this, NULL);
}

bool
GtkGui::run(void*)
{
    GNASH_REPORT_FUNCTION;
    gtk_main();
    return true;
}

bool
GtkGui::createMenu()
{
    GNASH_REPORT_FUNCTION;

    _popup_menu = GTK_MENU(gtk_menu_new());
    
    GtkMenuItem *menuitem_play =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Play Movie"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_play));
    gtk_widget_show(GTK_WIDGET(menuitem_play));    
    GtkMenuItem *menuitem_pause =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Pause Movie"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_pause));
    gtk_widget_show(GTK_WIDGET(menuitem_pause));
    GtkMenuItem *menuitem_stop =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Stop Movie"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_stop));
    gtk_widget_show(GTK_WIDGET(menuitem_stop));
    GtkMenuItem *menuitem_restart =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Restart Movie"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_restart));
    gtk_widget_show(GTK_WIDGET(menuitem_restart));
    GtkMenuItem *menuitem_step_forward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Forward Frame"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_step_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_forward));
    GtkMenuItem *menuitem_step_backward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Backward Frame"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_step_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_backward));
    GtkMenuItem *menuitem_jump_forward =
        GTK_MENU_ITEM(gtk_menu_item_new_with_label("Jump Forward 10 Frames"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_jump_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_forward));
    GtkMenuItem *menuitem_jump_backward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Jump Backward 10 Frames"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_jump_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_backward));
    GtkMenuItem *menuitem_quit =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Quit Gnash"));
    gtk_menu_append(_popup_menu, GTK_WIDGET(menuitem_quit));
    gtk_widget_show(GTK_WIDGET(menuitem_quit));    
    g_signal_connect(GTK_OBJECT(menuitem_play), "activate",
                     G_CALLBACK(&menuitem_play_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_pause), "activate",
                     G_CALLBACK(&menuitem_pause_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_stop), "activate",
                     G_CALLBACK(&menuitem_stop_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_restart), "activate",
                     G_CALLBACK(&menuitem_restart_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_step_forward), "activate",
                     G_CALLBACK(&menuitem_step_forward_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_step_backward), "activate",
                     G_CALLBACK(&menuitem_step_backward_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_jump_forward), "activate",
                     G_CALLBACK(&menuitem_jump_forward_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_jump_backward), "activate",
                     G_CALLBACK(&menuitem_jump_backward_callback), this);
    g_signal_connect(GTK_OBJECT(menuitem_quit), "activate",
                     G_CALLBACK(&menuitem_quit_callback), this);

    return true;
}

bool
GtkGui::setupEvents()
{
  GNASH_REPORT_FUNCTION;

  g_signal_connect(G_OBJECT(_window), "delete_event",
                   G_CALLBACK(delete_event), this);
  g_signal_connect(G_OBJECT(_window), "key_press_event",
                   G_CALLBACK(key_press_event), this);
  g_signal_connect(G_OBJECT(_window), "key_release_event",
                   G_CALLBACK(key_release_event), this);

   gtk_widget_add_events(_drawing_area, GDK_EXPOSURE_MASK
                        | GDK_BUTTON_PRESS_MASK
                        | GDK_BUTTON_RELEASE_MASK
                        | GDK_KEY_RELEASE_MASK
                        | GDK_KEY_PRESS_MASK        
                        | GDK_POINTER_MOTION_MASK);
  
   g_signal_connect_swapped(G_OBJECT(_drawing_area),
                            "button_press_event",
                            G_CALLBACK(popup_handler),
                            GTK_OBJECT(_popup_menu));
  
  g_signal_connect(G_OBJECT(_drawing_area), "button_press_event",
                   G_CALLBACK(button_press_event), this);
  g_signal_connect(G_OBJECT(_drawing_area), "button_release_event",
                   G_CALLBACK(button_release_event), this);
  g_signal_connect(G_OBJECT(_drawing_area), "motion_notify_event",
                   G_CALLBACK(motion_notify_event), this);
  
  g_signal_connect_after(G_OBJECT (_drawing_area), "realize",
                         G_CALLBACK (realize_event), NULL);
  g_signal_connect(G_OBJECT (_drawing_area), "configure_event",
                   G_CALLBACK (configure_event), this);
//   g_signal_connect(G_OBJECT (_drawing_area), "expose_event",
//                    G_CALLBACK (expose_event), NULL);
//   g_signal_connect(G_OBJECT (_drawing_area), "unrealize",
//                           G_CALLBACK (unrealize_event), NULL);

  return true;
}


// This pops up the menu when the right mouse button is clicked
gint
GtkGui::popup_handler(GtkWidget *widget, GdkEvent *event)
{
//    GNASH_REPORT_FUNCTION;

    GtkMenu *menu = GTK_MENU(widget);
//    printf("event type # %i\n", event->type);
    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *event_button = (GdkEventButton *) event;
        if (event_button->button == 3) {
            gtk_menu_popup(menu, NULL, NULL, NULL, NULL,
                           event_button->button, event_button->time);
            return TRUE;
        }
    }
    return FALSE;
}

/// \brief restart the movie from the beginning
void
GtkGui::menuitem_restart_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_restart();
}

/// \brief quit complete, and close the application
void
GtkGui::menuitem_quit_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;

    gtk_main_quit();
}

/// \brief Start the movie playing from the current frame.
void
GtkGui::menuitem_play_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_play();
}

/// \brief toggle that's playing or paused.
void
GtkGui::menuitem_pause_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    dbglogfile << "menuitem_pause_callback: " << endl;
    menu_pause();
}

/// \brief stop the movie that's playing.
void
GtkGui::menuitem_stop_callback(GtkMenuItem* /*menuitem*/, gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_stop();
}

/// \brief step forward 1 frame
void
GtkGui::menuitem_step_forward_callback(GtkMenuItem* /*menuitem*/,
		gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_step_forward();
}

/// \brief step backward 1 frame
void
GtkGui::menuitem_step_backward_callback(GtkMenuItem* /*menuitem*/,
		gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_step_backward();
}

/// \brief jump forward 10 frames
void
GtkGui::menuitem_jump_forward_callback(GtkMenuItem* /*menuitem*/,
                               gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_jump_forward();
}

/// \brief jump backward 10 frames
void
GtkGui::menuitem_jump_backward_callback(GtkMenuItem* /*menuitem*/,
                                gpointer /*data*/)
{
//    GNASH_REPORT_FUNCTION;
    menu_jump_backward();
}

//
// Event handlers
//


#if 0
// These event handlers are never used.

gboolean
GtkGui::expose_event(GtkWidget *const widget,
             GdkEventExpose *const event,
             const gpointer data)
{
    GNASH_REPORT_FUNCTION;

    GdkGLDrawable *const gldrawable = gtk_widget_get_gl_drawable(widget);
    g_assert(gldrawable);
    GdkGLContext *const glcontext = gtk_widget_get_gl_context(widget);
    g_assert(glcontext);

    if (event->count == 0
        && gdk_gl_drawable_make_current(gldrawable, glcontext)) {
    }

    return TRUE;
}

gboolean
GtkGui::unrealize_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    
    return TRUE;
}

#endif

gboolean
GtkGui::configure_event(GtkWidget *const widget,
                GdkEventConfigure *const event,
                const gpointer data)
{
    GNASH_REPORT_FUNCTION;

	GtkGui* obj = static_cast<GtkGui*>(data);

#ifdef RENDERER_CAIRO
    GtkCairoGlue& glue = obj->glue;
#elif defined(RENDERER_OPENGL)
    GtkGlExtGlue& glue = obj->glue;
#endif

    glue.configure(widget, event);
    obj->resize_view(event->width, event->height);

    return TRUE;
}


gboolean
GtkGui::realize_event(GtkWidget* /*widget*/, GdkEvent* /*event*/,
		gpointer /*data*/)
{
    GNASH_REPORT_FUNCTION;

    return TRUE;
}

// Shut everything down and exit when we're destroyed as a window
gboolean
GtkGui::delete_event(GtkWidget* /*widget*/, GdkEvent* /*event*/,
			gpointer /*data*/)
{
    GNASH_REPORT_FUNCTION;

    gtk_main_quit();
    return TRUE;
}


gnash::key::code
GtkGui::gdk_to_gnash_key(guint key)
{
    gnash::key::code  c(gnash::key::INVALID);
    
    if (key >= GDK_0 && key <= GDK_9)	{
        c = (gnash::key::code) ((key - GDK_0) + gnash::key::_0);
	} else if (key >= GDK_a && key <= GDK_z) {
        c = (gnash::key::code) ((key - GDK_a) + gnash::key::A);
    } else if (key >= GDK_F1 && key <= GDK_F15)	{
        c = (gnash::key::code) ((key - GDK_F1) + gnash::key::F1);
    } else if (key >= GDK_KP_0 && key <= GDK_KP_9) {
        c = (gnash::key::code) ((key - GDK_KP_0) + gnash::key::KP_0);
    } else {
        // many keys don't correlate, so just use a look-up table.
        struct {
            guint             gdk;
            gnash::key::code  gs;
        } table[] = {
            { GDK_BackSpace, gnash::key::BACKSPACE },
            { GDK_Tab, gnash::key::TAB },
            { GDK_Clear, gnash::key::CLEAR },
            { GDK_Return, gnash::key::ENTER },
            
            { GDK_Shift_L, gnash::key::SHIFT },
            { GDK_Shift_R, gnash::key::SHIFT },
            { GDK_Control_L, gnash::key::CONTROL },
            { GDK_Control_R, gnash::key::CONTROL },
            { GDK_Alt_L, gnash::key::ALT },
            { GDK_Alt_R, gnash::key::ALT },
            { GDK_Caps_Lock, gnash::key::CAPSLOCK },
            
            { GDK_Escape, gnash::key::ESCAPE },
            { GDK_space, gnash::key::SPACE },
            
            { GDK_Page_Down, gnash::key::PGDN },
            { GDK_Page_Up, gnash::key::PGUP },
            { GDK_Home, gnash::key::HOME },
            { GDK_End, gnash::key::END },
            { GDK_Left, gnash::key::LEFT },
            { GDK_Up, gnash::key::UP },
            { GDK_Right, gnash::key::RIGHT },
            { GDK_Down, gnash::key::DOWN },
            { GDK_Insert, gnash::key::INSERT },
            { GDK_Delete, gnash::key::DELETEKEY },
            
            { GDK_Help, gnash::key::HELP },
            { GDK_Num_Lock, gnash::key::NUM_LOCK },
            { GDK_semicolon, gnash::key::SEMICOLON },
            { GDK_equal, gnash::key::EQUALS },
            { GDK_minus, gnash::key::MINUS },
            { GDK_slash, gnash::key::SLASH },
            /* Backtick */
            { GDK_bracketleft, gnash::key::LEFT_BRACKET },
            { GDK_backslash, gnash::key::BACKSLASH },
            { GDK_bracketright, gnash::key::RIGHT_BRACKET },
            { GDK_quotedbl, gnash::key::QUOTE },
            { GDK_VoidSymbol, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].gdk != GDK_VoidSymbol; i++) {
            if (key == table[i].gdk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    return c;
}

gboolean
GtkGui::key_press_event(GtkWidget *const /*widget*/,
                GdkEventKey *const event,
                const gpointer /*data*/)
{
    GNASH_REPORT_FUNCTION;

    /* Forward key event to gnash */
    gnash::key::code	c = gdk_to_gnash_key(event->keyval);
    
    if (c != gnash::key::INVALID) {
        gnash::notify_key_event(c, true);
    }

    /* Handle GUI shortcuts */
    if (event->length <= 0) {
        return true;
    }
    unsigned int key = gdk_unicode_to_keyval(event->keyval);
    if (event->state == GDK_SHIFT_MASK) {
        dbglogfile << "Got Shift-key: " << key << endl;
    }
    if (event->state == GDK_CONTROL_MASK) {
        switch( (char)key) {
          case 'q':
          case 'w':
              menuitem_quit_callback(NULL, NULL);
              break;
          case 'r':
              menuitem_restart_callback(NULL, NULL);
              break;
          case 'p':
              menuitem_pause_callback(NULL, NULL);
              break;
          default:
              dbglogfile << "Got Control-key: " << key << endl;
              break;
        }
    }
    if ( event->hardware_keycode == 9 )
        menuitem_quit_callback(NULL,NULL); //Only hardware_keycode worked to detect Escape key pressed.
    if ((event->state != GDK_CONTROL_MASK) || !(event->state != GDK_SHIFT_MASK)) {
        dbglogfile << "Got key: '" << (char) key << "' its name is: " << gdk_keyval_name(key) << " hwkeycode: " << event->hardware_keycode << endl;
    }
        
    switch (key) {
      case '[':
          menuitem_step_forward_callback(NULL, NULL);
          break;
      case ']':
          menuitem_step_backward_callback(NULL, NULL);
          break;
      default:
          break;
    }
        
    return true;
}


gboolean
GtkGui::key_release_event(GtkWidget *const /*widget*/,
                GdkEventKey *const event,
                const gpointer /*data*/)
{
    GNASH_REPORT_FUNCTION;

    /* Forward key event to gnash */
    gnash::key::code	c = gdk_to_gnash_key(event->keyval);
    
    if (c != gnash::key::INVALID) {
        gnash::notify_key_event(c, false);
    }
    
    return true;
}


gboolean
GtkGui::button_press_event(GtkWidget *const /*widget*/,
                           GdkEventButton *const event,
                           const gpointer data)
{
    GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

    int	mask = 1 << (event->button - 1);
    obj->notify_mouse_clicked(true, mask);
    return true;
}

gboolean
GtkGui::button_release_event(GtkWidget * const /*widget*/,
                             GdkEventButton * const event,
                             const gpointer data)
{
    GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

    int	mask = 1 << (event->button - 1);
    obj->notify_mouse_clicked(false, mask);
    return true;
}

gboolean
GtkGui::motion_notify_event(GtkWidget *const /*widget*/,
                            GdkEventMotion *const event,
                            const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui *obj = static_cast<Gui *>(data);

    float xscale = obj->getXScale();
    float yscale = obj->getYScale();
    obj->notify_mouse_moved(int(event->x / xscale), int(event->y / yscale));
    return true;
}


// end of namespace gnash
}
