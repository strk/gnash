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

#ifdef RENDERER_OPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif // RENDERER_OPENGL

#include "gnash.h"
#include "Movie.h" // For movie_definition.
#include "log.h"

#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#ifdef RENDERER_OPENGL
#include <gtk/gtkgl.h>
#endif // RENDERER_OPENGL

#include "gui.h"
#include "gtksup.h"

#define OVERSIZE	1.0f

using namespace std;

namespace gnash 
{

GtkGui::~GtkGui()
{
}

bool
GtkGui::init(int xid, int argc, char **argv[])
{
  _xembed = true;
  _xid = xid;
  return init(argc, argv);
}

bool
GtkGui::init(int argc, char **argv[])
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL
    GdkGLConfigMode glcmode;
#endif
    gint major, minor;
    
    gtk_init (&argc, argv);
#ifdef RENDERER_OPENGL
    gtk_gl_init (&argc, argv);
    gdk_gl_query_version (&major, &minor);
    dbglogfile << "OpenGL extension version - "
               << (int)major << "." << (int)minor << endl;
    glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB
                                | GDK_GL_MODE_DEPTH
                                | GDK_GL_MODE_DOUBLE);
    _glconfig = gdk_gl_config_new_by_mode (glcmode);

    if (_glconfig == NULL) {
        dbglogfile << "Cannot find the double-buffered visual." << endl;
        dbglogfile << "Trying single-buffered visual." << endl;
        
        // Try single-buffered visual
        glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
        _glconfig = gdk_gl_config_new_by_mode (glcmode);
        if (_glconfig == NULL) {
            dbglogfile << "No appropriate OpenGL-capable visual found." << endl;
            exit (1);
        } else {
            dbglogfile << "Got single-buffered visual." << endl;
        }
    } else {
        dbglogfile << "Got double-buffered visual." << endl;
    }
#endif    
//    examine_gl_config_attrib (_glconfig);
  return true;
}

void
GtkGui::startGL()
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL   
    GdkGLContext *glcontext = gtk_widget_get_gl_context (_drawing_area);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
        dbglogfile << "ERROR: Couldn't start drawable!" << endl;
    }
#endif
}

void
GtkGui::swapBuffers()
{
#ifdef RENDERER_OPENGL
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);
    if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
        gdk_gl_drawable_swap_buffers (gldrawable);
    } else {
        glFlush();
    }
#endif
}

void
GtkGui::endGL()
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL    
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);
    if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
        gdk_gl_drawable_swap_buffers (gldrawable);
    } else {
        glFlush();
    }
    
    gdk_gl_drawable_gl_end (gldrawable);  
#endif
}

void
GtkGui::fixLodBias()
{
#ifdef RENDERER_OPENGL
	// If 2D textures weren't previously enabled, enable
	// them now and force the driver to notice the update,
	// then disable them again.
	if (!glIsEnabled(GL_TEXTURE_2D)) {
	// Clearing a mask of zero *should* have no
	// side effects, but coupled with enbling
	// GL_TEXTURE_2D it works around a segmentation
	// fault in the driver for the Intel 810 chip.
	glEnable(GL_TEXTURE_2D);
	glClear(0);
	glDisable(GL_TEXTURE_2D);
	}
#endif
}

bool
GtkGui::createWindow(int width, int height, long int xid)
{
  GNASH_REPORT_FUNCTION;
  if (xid) {
    _window = gtk_plug_new(xid);
    dbglogfile << "Created XEmbedded window" << endl;
  } else {
    _window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    dbglogfile << "Created top level window" << endl;
  }

  // XXX why do we need this?
  gtk_container_set_reallocate_redraws(GTK_CONTAINER (_window), TRUE);

  _drawing_area = gtk_drawing_area_new();

  if (!xid) {
    gtk_widget_set_size_request(_drawing_area, width, height);
  }

#ifdef RENDERER_OPENGL
  gtk_widget_set_gl_capability(_drawing_area, _glconfig,
                               NULL, TRUE, GDK_GL_RGBA_TYPE);
#endif
  createMenu();
  setupEvents();

  if (xid) {
    _xembed = true;
    _xid = xid;
  } else {
    _xembed = false;
  }
  _width = width;
  _height = height;
  
  gtk_widget_realize(_window);
  gtk_container_add(GTK_CONTAINER(_window), _drawing_area);
  gtk_widget_show(_drawing_area);
  gtk_widget_show(_window);

    gnash::render_handler *render;
#ifdef RENDERER_CAIRO
   cairo_t* cairohandle = gdk_cairo_create (_drawing_area->window);
   render = gnash::create_render_handler_cairo((void*)cairohandle);
#elif defined(RENDERER_OPENGL)
   render = gnash::create_render_handler_ogl();
#endif
   gnash::set_render_handler(render);

#ifdef RENDERER_OPENGL
        // Turn on alpha blending.
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Turn on line smoothing.  Antialiased lines can be used to
        // smooth the outsides of shapes.
        glEnable(GL_LINE_SMOOTH);
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);	// GL_NICEST, GL_FASTEST, GL_DONT_CARE
        
        glMatrixMode(GL_PROJECTION);
        glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        
        // We don't need lighting effects
        glDisable(GL_LIGHTING);
        // glColorPointer(4, GL_UNSIGNED_BYTE, 0, *);
        // glInterleavedArrays(GL_T2F_N3F_V3F, 0, *)
        glPushAttrib (GL_ALL_ATTRIB_BITS);		

#endif
  return true;
}

bool
GtkGui::poll(movie_interface* m, movie_definition*	md)
{
  GNASH_REPORT_FUNCTION;

  while (gtk_events_pending ()) {
//      dbglogfile << "Making GTK main iteration!" << endl;
          // Poll for events instead of letting gtk_main() handle them
	switch (menu_state) {
		case PLAY_MOVIE:
		m->set_play_state(gnash::movie_interface::PLAY);
		break;
		// Control-R restarts the movie
		case RESTART_MOVIE:
		m->restart();
		break;
		case STOP_MOVIE:
		m->set_play_state(gnash::movie_interface::STOP);
		break;
		case PAUSE_MOVIE:
		if (m->get_play_state() == gnash::movie_interface::STOP) {
			m->set_play_state(gnash::movie_interface::PLAY);
		} else {
			m->set_play_state(gnash::movie_interface::STOP);
		}
		break;
		// go backward one frame
		case STEP_BACKWARD:
		m->goto_frame(m->get_current_frame()-1);
		break;
		// go forward one frame
		case STEP_FORWARD:
		m->goto_frame(m->get_current_frame()+1);
		break;
		// jump goes backward 10 frames
		case JUMP_BACKWARD:
		m->goto_frame(m->get_current_frame()-10);
		break;
		// jump goes forward 10 frames
		case JUMP_FORWARD:
		{
			int forward_frame = m->get_current_frame()+10;
			if (forward_frame < md->get_frame_count()) {
				m->goto_frame(forward_frame);
			}
		}
		break;
		case QUIT_MOVIE:
		//goto done;
		break;
		default:
		break;
	}
      menu_state = IDLE_MOVIE;
      gtk_main_iteration();
  }
  return true;
}

bool
GtkGui::run()
{
  GNASH_REPORT_FUNCTION;
  gtk_main();
  return true;
}

void
GtkGui::resizeWindow()
{
  GNASH_REPORT_FUNCTION;
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
  
  g_signal_connect_after(G_OBJECT (_drawing_area), "realize",
                         G_CALLBACK (realize_event), this);
  g_signal_connect(G_OBJECT (_drawing_area), "configure_event",
                   G_CALLBACK (configure_event), this);
  g_signal_connect(G_OBJECT (_drawing_area), "expose_event",
                   G_CALLBACK (expose_event), this);
  
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
  
  
  return true;
}


// This pops up the menu when the right mouse button is clicked
gint
GtkGui::popup_handler(GtkWidget *widget, GdkEvent *event)
{
    GNASH_REPORT_FUNCTION;
    
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
GtkGui::menuitem_restart_callback(GtkMenuItem *menuitem, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = RESTART_MOVIE;
}

/// \brief quit complete, and close the application
void
GtkGui::menuitem_quit_callback(GtkMenuItem *menuitem, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = QUIT_MOVIE;
// Only use gtk_main_quit() if gtk_main() is used. For now, we're using
// a gtk_main_iteration() to do it in a polling fashion instead.    
//    gtk_main_quit();
    exit(0);
}

/// \brief Start the movie playing from the current frame.
void
GtkGui::menuitem_play_callback(GtkMenuItem *menuitem, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = PLAY_MOVIE;
}

/// \brief toggle that's playing or paused.
void
GtkGui::menuitem_pause_callback(GtkMenuItem * menuitem,
                        gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = PAUSE_MOVIE;
}

/// \brief stop the movie that's playing.
void
GtkGui::menuitem_stop_callback(GtkMenuItem *menuitem,
                       gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = STOP_MOVIE;
}

/// \brief step forward 1 frame
void
GtkGui::menuitem_step_forward_callback(GtkMenuItem *menuitem,
                               gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = STEP_FORWARD;
}

/// \brief step backward 1 frame
void
GtkGui::menuitem_step_backward_callback(GtkMenuItem *menuitem,
                                gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = STEP_BACKWARD;
}

/// \brief jump forward 10 frames
void
GtkGui::menuitem_jump_forward_callback(GtkMenuItem *menuitem,
                               gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = JUMP_FORWARD;
}

/// \brief jump backward 10 frames
void
GtkGui::menuitem_jump_backward_callback(GtkMenuItem *menuitem,
                                gpointer data)
{
    GNASH_REPORT_FUNCTION;
    menu_state = JUMP_BACKWARD;
}

//
// Event handlers
//

// Shut everything down and exit when we're destroyed as a window
gboolean
GtkGui::delete_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
// Only use gtk_main_quit() if gtk_main() is used. For now, we're using
// a gtk_main_iteration() to do it in a polling fashion instead.    
//    gtk_main_quit();
    exit(0);
    return TRUE;
}

// 
gboolean
GtkGui::realize_event(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GNASH_REPORT_FUNCTION;
    
    return true;
}

gboolean
GtkGui::expose_event(GtkWidget *const widget,
             GdkEventExpose *const event,
             const gpointer data)
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL   
    GdkGLDrawable *const gldrawable = gtk_widget_get_gl_drawable(widget);
    g_assert(gldrawable);
    GdkGLContext *const glcontext = gtk_widget_get_gl_context(widget);
    g_assert(glcontext);

   if (event->count == 0
        && gdk_gl_drawable_make_current(gldrawable, glcontext)) {
//        viewer.redraw();
    }
#endif  
    return true;
}

gboolean
GtkGui::configure_event(GtkWidget *const widget,
                GdkEventConfigure *const event,
                const gpointer data)
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);

    if (gdk_gl_drawable_make_current(gldrawable, glcontext)) {

        glViewport (event->x, event->y, event->width, event->height);
        // Reset the size of the frame. This is really ugly but these
        // global variables are used by the existing main event loop
        // in gnash.cpp to set the size of the rendered image.

    }
       // gnash::mwidth = event->width;
       // gnash::mheight = event->height;
#endif

    return true;
}

gboolean
GtkGui::key_press_event(GtkWidget *const widget,
                GdkEventKey *const event,
                const gpointer data)
{
    GNASH_REPORT_FUNCTION;

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
            dbglogfile << "Got Shift-key: " << key << endl;
        }
        if (event->state == GDK_CONTROL_MASK) {
            switch(key) {
              case 'r':
                  menu_state = RESTART_MOVIE;
                  break;
              case 'p':
                  menu_state = PAUSE_MOVIE;
                  break;
              default:
                  dbglogfile << "Got Control-key: " << key << endl;
                  break;
            }
        }
        if ((event->state != GDK_CONTROL_MASK) || !(event->state != GDK_SHIFT_MASK)) {
            dbglogfile << "Got key: " << key << endl;
        }
        
        gnash::key::code	c(gnash::key::INVALID);
        
        if (key >= 'a' && key <= 'z') {
            c = (gnash::key::code) ((key - 'a') + gnash::key::A);
//         } else if (key >= SDLK_F1 && key <= SDLK_F15)	{
//             c = (gnash::key::code) ((key - SDLK_F1) + gnash::key::F1);
//         } else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
//             c = (gnash::key::code) ((key - SDLK_KP0) + gnash::key::KP_0);
        }
        switch (key) {
          case '[':
              menu_state = STEP_FORWARD;
              break;
          case ']':
              menu_state = STEP_BACKWARD;
              break;
          default:
              break;
        }
        
        if (c != gnash::key::INVALID) {
//            gnash::notify_key_event(c, true);
        }
    };
        
    return true;
}

gboolean
GtkGui::button_press_event(GtkWidget *const widget,
                   GdkEventButton *const event,
                   const gpointer data)
{
    GNASH_REPORT_FUNCTION;
    
    Gui *obj = (Gui *)data;
    int	mask = 1 << (event->button - 1);
    int buttons = obj->getMouseButtons();
    obj->setMouseButtons(buttons |= mask);
    obj->setMouseX(int(event->x));
    obj->setMouseY(int(event->y));
    
    return true;
}

gboolean
GtkGui::button_release_event(GtkWidget * const widget,
                     GdkEventButton * const event,
                     const gpointer data)
{
    GNASH_REPORT_FUNCTION;
    Gui *obj = (Gui *)data;
    int	mask = 1 << (event->button - 1);
    int buttons = obj->getMouseButtons();
    obj->setMouseButtons(buttons &= ~mask);
    
    obj->setMouseX(int(event->x));
    obj->setMouseY(int(event->y));

    return true;
}

gboolean
GtkGui::motion_notify_event(GtkWidget *const widget,
                    GdkEventMotion *const event,
                    const gpointer data)
{
//    GNASH_REPORT_FUNCTION;
    Gui *obj = (Gui *)data;
//     if (event->state & Button1Mask) {
//         info.what = 0;
//     } else if (event->state & Button2Mask) {
//         info.what = 1;
//     } else if (event->state & Button3Mask) {
//         info.what = 2;
//     } else {
//         info.event = viewer::event_mouse_move;
//     }
    obj->setMouseX(int(event->x));
    obj->setMouseY(int(event->y));

    return true;
}


void
GtkGui::print_gl_config_attrib (GdkGLConfig *glconfig,
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
GtkGui::examine_gl_config_attrib (GdkGLConfig *glconfig)
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

void
GtkGui::drawTestGraphic()
{
    GNASH_REPORT_FUNCTION;
#ifdef RENDERER_OPENGL
    GdkGLContext *glcontext = gtk_widget_get_gl_context (_drawing_area);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (_drawing_area);
    
    GLUquadricObj *qobj;
    static GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
    static GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
    
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
        dbglogfile << "ERROR: Couldn't start drawable!" << endl;
        return;
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
    
    glViewport (0, 0, _width, _height);
    
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective (40.0, 1.0, 1.0, 10.0);
    
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
    gluLookAt (0.0, 0.0, 3.0,
               0.0, 0.0, 0.0,
               0.0, 1.0, 0.0);
    glTranslatef (0.0, 0.0, -3.0);
    
    if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
        gdk_gl_drawable_swap_buffers (gldrawable);
    } else {
        glFlush();
    }    
    gdk_gl_drawable_gl_end (gldrawable);    
#endif
}

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

// end of namespace gnash
}
