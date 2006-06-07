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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include "gnash.h"
#include "log.h"
#ifdef USE_GTKGLEXT
# include <gtk/gtk.h>
# include <gdk/gdkx.h>
# include <gtk/gtkgl.h>
#include "gtksup.h"
#endif

using namespace std;
using namespace gnash;

GdkGLConfig *glconfig = NULL;
GdkGLContext *glcontext = NULL;

int mouse_x = 0;
int mouse_y = 0;
int mouse_buttons = 0;
int width = 0;
int height = 0;

#if defined(USE_GTKGLEXT) && defined(HAVE_GTK2)
movie_state_e movie_menu_state;

// Define is you just want a hard coded OpenGL graphic
//#define TEST_GRAPHIC

//
// Popup menu support
//

// This pops up the menu when the right mouse button is clicked
gint
popup_handler(GtkWidget *widget, GdkEvent *event)
{
//    GNASH_REPORT_FUNCTION;
    
    GtkMenu *menu;
    GdkEventButton *event_button;

    menu = GTK_MENU(widget);
//    printf("event type # %i\n", event->type);
    if (event->type == GDK_BUTTON_PRESS) {
        event_button = (GdkEventButton *) event;
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
menuitem_restart_callback(GtkMenuItem *menuitem, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = RESTART_MOVIE;
}

/// \brief quit complete, and close the application
void
menuitem_quit_callback(GtkMenuItem *menuitem, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = QUIT_MOVIE;
    delete_event(GTK_WIDGET(menuitem), NULL, data);
}

/// \brief Start the movie playing from the current frame.
void
menuitem_play_callback(GtkMenuItem *menuitem, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = PLAY_MOVIE;
}

/// \brief toggle that's playing or paused.
void
menuitem_pause_callback(GtkMenuItem * menuitem,
                        gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = PAUSE_MOVIE;
}

/// \brief stop the movie that's playing.
void
menuitem_stop_callback(GtkMenuItem *menuitem,
                       gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STOP_MOVIE;
}

/// \brief step forward 1 frame
void
menuitem_step_forward_callback(GtkMenuItem *menuitem,
                               gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STEP_FORWARD;
}

/// \brief step backward 1 frame
void
menuitem_step_backward_callback(GtkMenuItem *menuitem,
                                gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = STEP_BACKWARD;
}

/// \brief jump forward 10 frames
void
menuitem_jump_forward_callback(GtkMenuItem *menuitem,
                               gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = JUMP_FORWARD;
}

/// \brief jump backward 10 frames
void
menuitem_jump_backward_callback(GtkMenuItem *menuitem,
                                gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    movie_menu_state = JUMP_BACKWARD;
}

/// \brief setup the menu for the right-click mouse button
void
add_menuitems(GtkMenu *popup_menu)
{
    GtkMenuItem *menuitem_play =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Play Movie"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_play));
    gtk_widget_show(GTK_WIDGET(menuitem_play));    
    GtkMenuItem *menuitem_pause =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Pause Movie"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_pause));
    gtk_widget_show(GTK_WIDGET(menuitem_pause));
    GtkMenuItem *menuitem_stop =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Stop Movie"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_stop));
    gtk_widget_show(GTK_WIDGET(menuitem_stop));
    GtkMenuItem *menuitem_restart =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Restart Movie"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_restart));
    gtk_widget_show(GTK_WIDGET(menuitem_restart));
    GtkMenuItem *menuitem_step_forward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Forward Frame"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_step_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_forward));
    GtkMenuItem *menuitem_step_backward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Backward Frame"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_step_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_backward));
    GtkMenuItem *menuitem_jump_forward =
        GTK_MENU_ITEM(gtk_menu_item_new_with_label("Jump Forward 10 Frames"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_jump_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_forward));
    GtkMenuItem *menuitem_jump_backward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Jump Backward 10 Frames"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_jump_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_jump_backward));
    GtkMenuItem *menuitem_quit =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Quit Gnash"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_quit));
    gtk_widget_show(GTK_WIDGET(menuitem_quit));
    g_signal_connect(GTK_OBJECT(menuitem_play), "activate",
                     G_CALLBACK(menuitem_play_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_pause), "activate",
                     G_CALLBACK(menuitem_pause_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_stop), "activate",
                     G_CALLBACK(menuitem_stop_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_restart), "activate",
                     G_CALLBACK(menuitem_restart_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_step_forward), "activate",
                     G_CALLBACK(menuitem_step_forward_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_step_backward), "activate",
                     G_CALLBACK(menuitem_step_backward_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_jump_forward), "activate",
                     G_CALLBACK(menuitem_jump_forward_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_jump_backward), "activate",
                     G_CALLBACK(menuitem_jump_backward_callback), NULL);
    g_signal_connect(GTK_OBJECT(menuitem_quit), "activate",
                     G_CALLBACK(menuitem_quit_callback), NULL);
}

//
// Event handlers
//

gboolean
unrealize_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if (glcontext) {
      g_object_unref (G_OBJECT (glcontext));
      glcontext = NULL;
    }

    if (glconfig) {
      g_object_unref (G_OBJECT (glconfig));
      glconfig = NULL;
    }
}

// Shut everything down and exit when we're destroyed as a window
gboolean
delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
//    GNASH_REPORT_FUNCTION;
// Only use gtk_main_quit() if gtk_main() is used. For now, we're using
// a gtk_main_iteration() to do it in a polling fashion instead.    
//    gtk_main_quit();
    exit(0);
    return TRUE;
}

// 
gboolean
realize_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
//   GNASH_REPORT_FUNCTION;
    
#ifdef TEST_GRAPHIC
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

    GLUquadricObj *qobj;
    static GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
    static GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
    
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
        dbglogfile << "ERROR: Couldn't start drawable!" << endl;
        return false;
    }
    
    qobj = gluNewQuadric ();
    gluQuadricDrawStyle (qobj, GLU_FILL);
    glNewList (1, GL_COMPILE);
    gluSphere (qobj, 1.0, 20, 20);
    glEndList ();
    
    glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv (GL_LIGHT0, GL_POSITION, light_position);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);
    glEnable (GL_DEPTH_TEST);
    
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClearDepth (1.0);
    
    glViewport (0, 0,
                widget->allocation.width, widget->allocation.height);
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective (40.0, 1.0, 1.0, 10.0);
    
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (0.0, 0.0, 3.0,
               0.0, 0.0, 0.0,
               0.0, 1.0, 0.0);
    glTranslatef (0.0, 0.0, -3.0);
    
    gdk_gl_drawable_gl_end (gldrawable);
    
// end of TEST_GRAPHIC
#endif
    
    // OpenGL END
}

gboolean
expose_event(GtkWidget *const widget,
             GdkEventExpose *const event,
             const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    
    GdkGLDrawable *const gldrawable = gtk_widget_get_gl_drawable(widget);
    g_assert(gldrawable);
    GdkGLContext *const glcontext = gtk_widget_get_gl_context(widget);
    g_assert(glcontext);

#ifdef TEST_GRAPHIC
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
        return FALSE;
    
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glCallList (1);
    
    if (gdk_gl_drawable_is_double_buffered (gldrawable))
        gdk_gl_drawable_swap_buffers (gldrawable);
    else
        glFlush ();
    
    gdk_gl_drawable_gl_end (gldrawable);
    // OpenGL END
#else
   if (event->count == 0
        && gdk_gl_drawable_make_current(gldrawable, glcontext)) {
//        viewer.redraw();
    }
 
// end of TEST_GRAPHIC
#endif
    
    return true;
}

gboolean
configure_event(GtkWidget *const widget,
                GdkEventConfigure *const event,
                const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

#ifdef TEST_GRAPHIC
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
        return FALSE;
    glViewport (0, 0,
                widget->allocation.width, widget->allocation.height);
    gdk_gl_drawable_gl_end (gldrawable);
    // OpenGL END
#else
    if (gdk_gl_drawable_make_current(gldrawable, glcontext)) {
        glViewport (event->x, event->y, event->width, event->height);
        // Reset the size of the frame. This is really ugly but these
        // global variables are used by the existing main event loop
        // in gnash.cpp to set the size of the rendered image.
        width = event->width;
        height = event->height;
    }
 
// end of TEST_GRAPHIC
#endif
    
    return true;
}

gboolean
key_press_event(GtkWidget *const widget,
                GdkEventKey *const event,
                const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    static bool shift_state = false;
    static bool control_state = false;    

    switch (event->keyval) {
    case XK_Home:
//        info.what = viewer::key_home;
        break;

    case XK_Left:
//        info.what = viewer::key_left;
        break;

    case XK_Up:
//        info.what = viewer::key_up;
        break;

    case XK_Right:
//        info.what = viewer::key_right;
        break;

    case XK_Down:
//        info.what = viewer::key_down;
        break;

    case XK_Page_Up:
//        info.what = viewer::key_page_up;
        break;

    case XK_Page_Down:
//        info.what = viewer::key_page_down;
        break;

    default:
        if (event->length <= 0) {
            return true;
        }
        char key = gdk_unicode_to_keyval(event->keyval);
        if (event->state == GDK_SHIFT_MASK) {
            shift_state = true;
            dbglogfile << "Got Shift-key: " << key << endl;
        }
        if (event->state == GDK_CONTROL_MASK) {
            switch(key) {
              case 'r':
                  movie_menu_state = RESTART_MOVIE;
                  break;
              case 'p':
                  movie_menu_state = PAUSE_MOVIE;
                  break;
              default:
                  dbglogfile << "Got Control-key: " << key << endl;
                  control_state = true;
                  break;
            }
        }
        if ((event->state != GDK_CONTROL_MASK) || !(event->state != GDK_SHIFT_MASK)) {
            dbglogfile << "Got key: " << key << endl;
        }
        
        gnash::key::code	c(gnash::key::INVALID);
        
        if (key >= 'a' && key <= 'z') {
            c = (gnash::key::code) ((key - 'a') + gnash::key::A);
        }
        // FIXME: we don't do anything with the state for now
        if (control_state) {
            control_state = false;
        }
        if (shift_state) {
            shift_state = false;
        }
        
        switch (key) {
          case '[':
              movie_menu_state = STEP_FORWARD;
              break;
          case ']':
              movie_menu_state = STEP_BACKWARD;
              break;
          default:
              break;
        }
        
        if (c != gnash::key::INVALID) {
            gnash::notify_key_event(c, true);
        }
    };
        
    return true;
}

gboolean
button_press_event(GtkWidget *const widget,
                   GdkEventButton *const event,
                   const gpointer data)
{
//    GNASH_REPORT_FUNCTION;

    int	mask = 1 << (event->button - 1);
    mouse_buttons |= mask;    

    mouse_x = (int)event->x;
    mouse_y = (int)event->y;

    return true;
}

gboolean
button_release_event(GtkWidget * const widget,
                     GdkEventButton * const event,
                     const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    int	mask = 1 << (event->button - 1);
    mouse_buttons &= ~mask;    

    mouse_x = (int)event->x;
    mouse_y = (int)event->y;

    return true;
}

gboolean
motion_notify_event(GtkWidget *const widget,
                    GdkEventMotion *const event,
                    const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
//     if (event->state & Button1Mask) {
//         info.what = 0;
//     } else if (event->state & Button2Mask) {
//         info.what = 1;
//     } else if (event->state & Button3Mask) {
//         info.what = 2;
//     } else {
//         info.event = viewer::event_mouse_move;
//     }

    mouse_x = (int)event->x;
    mouse_y = (int)event->y;
    
    return true;
}


void
print_gl_config_attrib (GdkGLConfig *glconfig,
                        const gchar *attrib_str,
                        int          attrib,
                        gboolean     is_boolean)
{
  int value;

  g_print ("%s = ", attrib_str);
  if (gdk_gl_config_get_attrib (glconfig, attrib, &value))
    {
      if (is_boolean)
        g_print ("%s\n", value == TRUE ? "TRUE" : "FALSE");
      else
        g_print ("%d\n", value);
    }
  else
    g_print ("*** Cannot get %s attribute value\n", attrib_str);
}

void
examine_gl_config_attrib (GdkGLConfig *glconfig)
{
  g_print ("\nOpenGL visual configurations :\n\n");

  g_print ("gdk_gl_config_is_rgba (glconfig) = %s\n",
           gdk_gl_config_is_rgba (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_is_double_buffered (glconfig) = %s\n",
           gdk_gl_config_is_double_buffered (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_is_stereo (glconfig) = %s\n",
           gdk_gl_config_is_stereo (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_has_alpha (glconfig) = %s\n",
           gdk_gl_config_has_alpha (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_has_depth_buffer (glconfig) = %s\n",
           gdk_gl_config_has_depth_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_has_stencil_buffer (glconfig) = %s\n",
           gdk_gl_config_has_stencil_buffer (glconfig) ? "TRUE" : "FALSE");
  g_print ("gdk_gl_config_has_accum_buffer (glconfig) = %s\n",
           gdk_gl_config_has_accum_buffer (glconfig) ? "TRUE" : "FALSE");

  g_print ("\n");

  print_gl_config_attrib (glconfig, "GDK_GL_USE_GL",           GDK_GL_USE_GL,           TRUE);
  print_gl_config_attrib (glconfig, "GDK_GL_BUFFER_SIZE",      GDK_GL_BUFFER_SIZE,      FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_LEVEL",            GDK_GL_LEVEL,            FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_RGBA",             GDK_GL_RGBA,             TRUE);
  print_gl_config_attrib (glconfig, "GDK_GL_DOUBLEBUFFER",     GDK_GL_DOUBLEBUFFER,     TRUE);
  print_gl_config_attrib (glconfig, "GDK_GL_STEREO",           GDK_GL_STEREO,           TRUE);
  print_gl_config_attrib (glconfig, "GDK_GL_AUX_BUFFERS",      GDK_GL_AUX_BUFFERS,      FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_RED_SIZE",         GDK_GL_RED_SIZE,         FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_GREEN_SIZE",       GDK_GL_GREEN_SIZE,       FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_BLUE_SIZE",        GDK_GL_BLUE_SIZE,        FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_ALPHA_SIZE",       GDK_GL_ALPHA_SIZE,       FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_DEPTH_SIZE",       GDK_GL_DEPTH_SIZE,       FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_STENCIL_SIZE",     GDK_GL_STENCIL_SIZE,     FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_ACCUM_RED_SIZE",   GDK_GL_ACCUM_RED_SIZE,   FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_ACCUM_GREEN_SIZE", GDK_GL_ACCUM_GREEN_SIZE, FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_ACCUM_BLUE_SIZE",  GDK_GL_ACCUM_BLUE_SIZE,  FALSE);
  print_gl_config_attrib (glconfig, "GDK_GL_ACCUM_ALPHA_SIZE", GDK_GL_ACCUM_ALPHA_SIZE, FALSE);

  g_print ("\n");
}

#if 0
// This is from OpenVRML
void swap_buffers()
{
    GtkWidget * const widget = GTK_WIDGET(this->drawing_area);
    GdkGLDrawable * const gl_drawable = gtk_widget_get_gl_drawable(widget);
    gdk_gl_drawable_swap_buffers(gl_drawable);
}
 
() throw ()
{
    if (this->timer) { g_source_remove(timer); }
}

void post_redraw()
{
    if (!this->redrawNeeded) {
        this->redrawNeeded = true;
        gtk_widget_queue_draw(GTK_WIDGET(this->drawing_area));
    }
}
 
gint
timeout_callback(const gpointer ptr)
{
    assert(ptr);
    GtkGLViewer & viewer = *static_cast<GtkGLViewer *>(ptr);
    viewer.timer_update();
    return false;
}

void
set_timer(const double t)
{
    if (!this->timer) {
        this->timer = g_timeout_add(guint(10.0 * (t + 1)),
                                    GtkFunction(timeout_callback),
                                    this);
    }
}

void
timer_update()
{
    this->timer = 0;
    this->viewer::update();
}
#endif

#if 0
// This is actually an Xt event handler, not a GTK one.

/// \brief Handle X events
///
/// This C function handles events from X, like keyboard events, or
/// Expose events that we're interested in.
void
xt_event_handler(Widget xtwidget, nsPluginInstance *plugin,
		 XEvent *xevent, Boolean *b)
{
    GNASH_REPORT_FUNCTION;

    int        keycode;
    KeySym     keysym;
#if 0
    SDL_Event  sdl_event;
    SDL_keysym sdl_keysym;

    //    handleKeyPress((SDL_keysym)keysym);
    log_msg("Peep Event returned %d", SDL_PeepEvents(&sdl_event, 1, SDL_PEEKEVENT, SDL_USEREVENT|SDL_ACTIVEEVENT|SDL_KEYDOWN|SDL_KEYUP|SDL_MOUSEBUTTONUP|SDL_MOUSEBUTTONDOWN));
  
    if (SDL_PollEvent(&sdl_event)) {
        switch(sdl_event.type) {
          case SDL_ACTIVEEVENT:
          case SDL_VIDEORESIZE:
          case SDL_KEYDOWN:
              /* handle key presses */
              handleKeyPress( &sdl_event.key.keysym );
              break;
          default:
              break;
      
        }
    }
#endif
  
    switch (xevent->type) {
      case Expose:
          // get rid of all other exposure events
          if (plugin) {
// 	      if (_glInitialized) {
// 		  plugin->setGL();
// #ifdef TEST_GRAPHIC
// 		  plugin->drawTestScene();
// 		  plugin->swapBuffers();
// 		  plugin->freeX();
// #else
// 		  gnash::movie_interface *m = gnash::get_current_root();
// 		  if (m != NULL) {
// 		      m->display();
// 		  }
// #endif
// 		  log_msg("Drawing GL Scene for expose event!");
// 	      } else {
 		  log_msg("GL Surface not initialized yet, ignoring expose event!");
// 	      }
          }
          break;
      case ButtonPress:
//     fe.type = FeButtonPress;
          log_msg("Button Press");
          break;
      case ButtonRelease:
          //     fe.type = FeButtonRelease;
          log_msg("Button Release");
          break;
      case KeyPress:
          keycode = xevent->xkey.keycode;
		plugin->lockX();
          keysym = XLookupKeysym((XKeyEvent*)xevent, 0);
          log_msg ("%s(%d): Keysym is %s", __PRETTY_FUNCTION__, __LINE__,
                  XKeysymToString(keysym));
		plugin->freeX();

          switch (keysym) {
            case XK_Up:
                log_msg("Key Up");
                break;
            case XK_Down:
                log_msg("Key Down");
                break;
            case XK_Left:
                log_msg("Key Left");
                break;
            case XK_Right:
                log_msg("Key Right");
                break;
            case XK_Return:
                log_msg("Key Return");
                break;
      
            default:
                break;
          }
    }
}
#endif

// end of HAVE_GTK2
#endif

