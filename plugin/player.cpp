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

// This has to be defined or we have typedef problems with Mozilla's
// headers for 64 bit types. According the the header file, the fix is
// to define this constant to turn off the older behaviour that we
// don't care about.
#define NO_NSPR_10_SUPPORT

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <signal.h>

#include "log.h"
#include "gnash.h"
#include "plugin.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "xmlsocket.h"
#include "Movie.h"
#include "ogl.h"

// Mozilla SDK headers
#include "prinit.h"
#include "plugin.h"
#include "prlock.h"
#include "prcvar.h"
#include "prthread.h"

#include <GL/gl.h>
#ifdef HAVE_GTK2
# include <gtk/gtk.h>
#endif
#ifdef USE_GTKGLEXT
# include <gdk/gdkx.h>
# include <gdk/gdkgl.h>
# include <gtk/gtkgl.h>
#endif
#include "gtksup.h"

// Define is you just want a hard coded OpenGL graphic
//#define TEST_GRAPHIC

#ifdef HAVE_LIBXML

bool gofast = false;		// FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,
bool nodelay = false;           // FIXME: this flag gets set based on
				// an XML message written using
				// SendCommand(""). This way a movie
				// can optimize it's own performance
				// when needed,
extern int xml_fd;              // FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.
#endif // HAVE_LIBXML

bool GLinitialized = false;
bool processing = false;

using namespace std;
using namespace gnash;

#ifdef HAVE_GTKGLEXT
static void realize (GtkWidget *widget, gpointer data);
static gboolean configure_event (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
static gboolean expose_event (GtkWidget *widget, GdkEventExpose *event, gpointer data);
static void examine_gl_config_attrib(GdkGLConfig *glconfig);
#endif

#define OVERSIZE	1.0f

//static int runThread(void *nothing);
//static int doneYet = 0;

static float	s_scale = 1.0f;
//static bool	s_antialiased = false;
//static int	s_bit_depth = 16;
static bool	s_verbose = false;
static bool	s_background = true;
//static bool	s_measure_performance = false;
//static bool	s_event_thread = false;
//static bool	s_start_waiting = false;

//static void interupt_handler (int);

static void
message_log(const char* message)
// Process a log message.
{
    if (s_verbose) {
	fputs(message, stdout);
        fflush(stdout); // needed on osx for some reason
    }
}

static tu_file*
file_opener(const char* url)
// Callback function.  This opens files for the library.
{
    return new tu_file(url, "rb");
}

static void
fs_callback(gnash::movie_interface* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    message_log("fs_callback: '"); // __GNASH_PLAYER_H__
    message_log(command);
    message_log("' '");
    message_log(args);
    message_log("'\n");
}

int
main_loop(nsPluginInstance *inst)
{

    // add xt event handler#
//    long event_mask = ExposureMask|KeyPress|KeyRelease|ButtonPress|ButtonRelease;
//    Widget xtwidget;    
//     xtwidget =  XtWindowToWidget(gxDisplay, inst->getWindow());
//     XtAddEventHandler(xtwidget, event_mask, FALSE,
// 		      (XtEventHandler) xt_event_handler, inst);

#ifdef USE_GTKGLEXT
    int argc = 0;
    char *argv[5];
    memset(argv, 0, sizeof(char *)*5);
    argv[0] = new char(20);
    strcpy(argv[0], "./gnash");
    argv[1] = new char(20);
    strcpy(argv[1], "-v");
    gtk_gl_init(&argc, (char***)argv);

    int major, minor;
    gdk_gl_query_version (&major, &minor);
    dbglogfile << "OpenGL extension version - " << major
 	       << "." << minor << endl;
    static const int double_attrib_list[] = {
 	GDK_GL_MODE_DOUBLE,
 	GDK_GL_MODE_DEPTH,
 	GDK_GL_MODE_RGB
    };
    static const int single_attrib_list[] = {
 	GDK_GL_MODE_DOUBLE,
 	GDK_GL_MODE_DEPTH,
 	GDK_GL_MODE_RGB
    };

    GdkGLConfig *gl_config;
    gl_config = gdk_gl_config_new(double_attrib_list);
//     if (gl_config == NULL) {
// 	dbglogfile << "Cannot find the double-buffered visual." << endl;
// 	dbglogfile << "Trying single-buffered visual." << endl;
// 	// Try single-buffered visual
// 	gl_config = gdk_gl_config_new(single_attrib_list);
// 	if (gl_config == NULL) {
// 	    dbglogfile
// 		<< "ERROR: No appropriate OpenGL-capable visual found."
// 		<< endl;
// 	    exit (1);
//         } else {
// 	    dbglogfile << "Got single buffered visual" << endl;
// 	}
//     } else {
// 	dbglogfile << "Got double buffered visual" << endl;
//     }

//     examine_gl_config_attrib (gl_config);
#endif
    
#ifdef HAVE_GTK2
//  GtkWidget *gtkwidget = gtk_plug_new(inst->getWindow());
    GtkWidget *gtkwidget = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW(gtkwidget), "Gnash Player");
    // This is the right button menu
    GtkMenu   *popup_menu = GTK_MENU(gtk_menu_new());

    GtkWidget *vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (gtkwidget), vbox);
    gtk_widget_show (vbox); 

#ifdef GTKGLEXT
    GtkWidget *drawarea = gtk_drawing_area_new();
    static GdkGLContext *const share_list = NULL;
    static const gboolean direct = TRUE;
    static const int render_type = GDK_GL_RGBA_TYPE;
    gtk_widget_set_gl_capability(drawarea, gl_config, share_list,
				 direct, render_type);
     gtk_box_pack_start(GTK_BOX(vbox), drawarea, TRUE, TRUE, 0);
     gtk_container_add(GTK_CONTAINER(gtkwidget), drawarea);
#endif
     
    gtk_widget_add_events(gtkwidget, GDK_BUTTON_PRESS_MASK);
//    gtk_widget_add_events(gtkwidget, GDK_BUTTON_RELEASE_MASK);
    g_signal_connect_swapped(G_OBJECT(gtkwidget),
			     "button_press_event",
 			     G_CALLBACK(popup_handler),
 			     GTK_OBJECT(popup_menu));
    
//     gtk_signal_connect(GTK_OBJECT(gtkwidget), "delete_event",
// 		       GTK_SIGNAL_FUNC(destroy_callback), inst);    

//     g_signal_connect_after (G_OBJECT (drawarea), "realize",
// 			    G_CALLBACK (realize), NULL);
    
    gtk_widget_realize(gtkwidget);
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
    GtkMenuItem *menuitem_step_forward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Forward Frame"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_step_forward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_forward));
    GtkMenuItem *menuitem_step_backward =
 	GTK_MENU_ITEM(gtk_menu_item_new_with_label("Step Backward Frame"));
    gtk_menu_append(popup_menu, GTK_WIDGET(menuitem_step_backward));
    gtk_widget_show(GTK_WIDGET(menuitem_step_backward));
    
    g_signal_connect(GTK_OBJECT(menuitem_play), "activate",
		     G_CALLBACK(menuitem_play_callback), inst);
    g_signal_connect(GTK_OBJECT(menuitem_pause), "activate",
		     G_CALLBACK(menuitem_pause_callback), inst);
    g_signal_connect(GTK_OBJECT(menuitem_stop), "activate",
		     G_CALLBACK(menuitem_stop_callback), inst);
    g_signal_connect(GTK_OBJECT(menuitem_step_forward), "activate",
		     G_CALLBACK(menuitem_step_forward_callback), inst);
    g_signal_connect(GTK_OBJECT(menuitem_step_backward), "activate",
 		     G_CALLBACK(menuitem_step_backward_callback), inst);
#endif
    
//    gtk_widget_set_size_request(gtkwidget, inst->getWidth(),
//				inst->getHeight());    
//    gtk_widget_show(gtkwidget);	// gtk_widget_show_all(window)
// FIXME: We need a logo!   
//     logo = gdk_pixbuf_new_from_inline(-1, gtk_logo, FALSE, NULL);
//     image = gtk_image_new_from_pixbuf(logo);
    
    assert(tu_types_validate());
    float	exit_timeout = 0;
    bool	do_sound = false;
    int		delay = 100;	// was 31
#ifdef TEST_GRAPHIC
    int		retries = 0;
#endif
    float	tex_lod_bias;

    const char *infile = inst->getFilename();
    
    log_msg("%s: Playing %s\n", __PRETTY_FUNCTION__, infile);
    
    // -1.0 tends to look good.
    tex_lod_bias = -1.2f;  
    
    if (infile == NULL) {
        log_msg("no input file\n");
        exit(1);
    }
    
#if 0
    int stall = 0;
    while (stall++ < 3) {
        printf("Stalling for GDB at pid %ld\n", getpid());
        sleep(10);
    }
#endif
#if 0
    gnash::set_verbose_action(true);
    gnash::set_verbose_parse(true);
#endif
// Uncomment this if you want debug logs stored to disk.
// This is now the default
//    dbglogfile.setWriteDisk(true);
    
    gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);
    
    gnash::sound_handler  *sound = NULL;
    gnash::render_handler *render = NULL;
#ifdef HAVE_SDL_MIXER_H
    if (do_sound) {
	sound = gnash::create_sound_handler_sdl();
	gnash::set_sound_handler(sound);
    }
#endif
#ifdef HAVE_GST_GST_H
    if (do_sound) {
	sound = gnash::create_sound_handler_sdl();
	gnash::set_sound_handler(sound);
    }
#endif
    inst->lockDisplay();
    render = gnash::create_render_handler_ogl();
    gnash::set_render_handler(render);
    inst->freeDisplay();

    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float movie_fps = 30.0f;
    gnash::get_movie_info(infile, &movie_version, &movie_width, &movie_height, &movie_fps, NULL, NULL);
    if (movie_version == 0) {
        fprintf(stderr, "error: can't get info about %s\n", infile);
        exit(1);
    }
    log_msg("Movie %s: width is %d, height is %d, version is %d\n", infile,
	    movie_width, movie_height, movie_version);

#if 1
    int	width = int(movie_width * s_scale);
    int	height = int(movie_height * s_scale);
#else
    int	width = inst->getWidth();
    int	height = inst->getHeight();
#endif
    log_msg("Passed in width is %d, height is %d\n", inst->getWidth(),
	   inst->getHeight());
    log_msg("Calculated width is %d, height is %d\n",
	    int(movie_width * s_scale), int(movie_height * s_scale));

    if ((width != inst->getWidth()) && (height != inst->getHeight())) {
	dbglogfile << "WARNING: Movie size doesn't equal window size" << endl;
    }

    ogl::open();
    
    // Load the actual movie.
    inst->lockDisplay();
    gnash::movie_definition*	md = gnash::create_library_movie(infile);
    inst->freeDisplay();
    if (md == NULL) {
        fprintf(stderr, "error: can't create a movie from '%s'\n", infile);
        exit(1);
    }

    inst->lockDisplay();
    gnash::movie_interface*	m = create_library_movie_inst(md);
    inst->freeDisplay();
    if (m == NULL) {
        fprintf(stderr, "error: can't create movie instance\n");
//	inst->freeDisplay();
	exit(1);
    }
//    inst->freeDisplay();
    gnash::set_current_root(m);

    // Mouse state.

    int	mouse_x = 0;
    int	mouse_y = 0;
    int	mouse_buttons = 0;
    
    float	speed_scale = 1.0f;
    uint32_t	start_ticks = 0;
    start_ticks = SDL_GetTicks();
    uint32_t	last_ticks = start_ticks;
    int	frame_counter = 0;
//    int	last_logged_fps = last_ticks;

    // Trap ^C so we can kill all the threads
//    struct sigaction  act;
//    act.sa_handler = interupt_handler;
//    sigaction (SIGINT, &act, NULL);

    for (;;) {
	uint32_t	ticks;
	ticks = SDL_GetTicks();
	int	delta_ticks = ticks - last_ticks;
	float	delta_t = delta_ticks / 1000.f;
	last_ticks = ticks;
        
        // Check auto timeout counter.
	if (exit_timeout > 0
	    && ticks - start_ticks > (uint32_t) (exit_timeout * 1000)) {
	    dbglogfile << "Auto exiting now..." << endl;
	    break;
	}
        m = gnash::get_current_root();
        gnash::delete_unused_root();
	width = inst->getWidth();
	height = inst->getHeight();
	inst->lockDisplay();
	m->set_display_viewport(0, 0, width, height);
	inst->resizeWindow(width,height);

// // 	GLfloat ratio = (GLfloat)width / (GLfloat)height;
// // 	glViewport(0, 0, (GLint)width, (GLint)height);
// // 	gluPerspective(45.0f, ratio, 0.1f, 100.0f);

	m->set_background_alpha(s_background ? 1.0f : 0.05f);
	m->notify_mouse_state(mouse_x, mouse_y, mouse_buttons);    
	glDisable(GL_DEPTH_TEST);	// Disable depth testing.
	glDrawBuffer(GL_BACK);
	inst->freeDisplay();
        
        m->advance(delta_t * speed_scale);
#ifdef TEST_GRAPHIC
	dbglogfile << "We made it!!!" << endl;
	inst->drawTestScene();
#else
	dbglogfile << "Display rendered graphic!!!" << endl;
	inst->lockDisplay();
//	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
//	glLoadIdentity();
	m->display();
	inst->swapBuffers();
	inst->freeDisplay();
#endif

	frame_counter++;

#ifndef TEST_GRAPHIC
#if 1				// FIXME: run forever ?
	// See if we should exit
 	if (m->get_current_frame() + 1 == md->get_frame_count()) {
	    dbglogfile << "Reached the end of the movie..." << endl;
 	    break;
 	}
#endif
#else
	if (retries++ > 5vi ~/.en) {
	    break;   
	}
#endif
	// nsPluginInstance::shut() has been called for this instance.
	NPBool die = inst->getShutdown();
	if (die) {
	    dbglogfile << "Shutting down as requested..." << endl;
	    break;
	}
	
//	void *pd = PR_GetThreadPrivate(inst->getThreadKey());
//	dbglogfile << "Thread Data is: " << (char *)pd << endl;
	//glPopAttrib ();
	
	// Don't hog the CPU.
#ifdef TEST_GRAPHIC
	dbglogfile << "About to sleep for 1 second...!!!" << endl;
	sleep(1);
#else
	dbglogfile << "About to sleep for " << delay
		   << " milliseconds...!!!" << endl;
	PR_Sleep(delay);
#endif
    }
//    SDL_KillThread(thread);	// kill the network read thread
//    SDL_Quit();
    
    if (m) {
	m->drop_ref();
    }
    
    delete sound;
    delete render;
	
    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();
    
    return 0;
}

#if 0		/* No longer used?  FIXME */
static int
runThread(void *nothing)
{
#ifdef HAVE_LIBXML
    
    //int i = 123;
    int val;
    int count = 0;
    SDL_Event *ptr;
#if 1
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    ev.user.code  = 0;
    ev.user.data1 = 0;
    ev.user.data2 = 0;
    ptr = &ev;
#else
    ptr = (SDL_Event *)ev_ptr;
    ptr->type = SDL_USEREVENT;
    ptr->user.code  = 0;
    ptr->user.data1 = 0;
    ptr->user.data2 = 0;
#endif
    
    log_msg("Initializing event thread...\n");
    
    while (gnash::check_sockets(xml_fd) == -1) {
        sleep(10); // Delay to give the socket time to
        // connect.
        continue;
    }
    
    // give everything a chance to initialize.
    // since all the frames need to be set up,
    // and this is just a more runtime performance
    // issue with CPU load.
    sleep(20);
    
    log_msg("Enabling Event Wait Mode...\n");
    s_start_waiting = true;
    
    while (!doneYet) {
        //ptr->user.data1 = (void *)i;
        if ((val = gnash::check_sockets(xml_fd)) == -1) {
            return -1; // we shouldn't be seeing any errors
        }
        // Don't push an event if there is already one in the
        // queue. XMLSocket::onData() will come around and get
        // the data anyway.
        count = SDL_PeepEvents(ptr, 1, SDL_PEEKEVENT, SDL_USEREVENT);
        // printf("%d User Events in queue\n", count);
        if ((count == 0) && (val >= 0)) {
            //printf("Pushing User Event on queue\n");
            SDL_PushEvent(ptr);
            SDL_Delay(300);	// was 300
        }
    }
#endif // HAVE_LIBXML
    
    return 0;
}
#endif /* 0 */

void
playerThread(void *arg)
{
    nsPluginInstance *inst = (nsPluginInstance *)arg;    
    log_trace("%s: instance is %p for %s\n", __PRETTY_FUNCTION__, inst,
	   inst->getFilename());
    
//    SDL_CondWait(gCond, playerMutex);
//    inst->condWait();

    main_loop(inst);

    log_msg("%s: Done this = %p...\n", __PRETTY_FUNCTION__, inst);

    return;
}

#if 0
void
interupt_handler (int sig)
{
    dbglogfile << "Got a signal #" << sig << endl;
    
    exit(-1);
}
#endif /* 0 */

#ifdef HAVE_GTKGLEXT
static void
realize (GtkWidget *widget, gpointer   data)
{
    GNASH_REPORT_FUNCTION;
    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    
    GLUquadricObj *qobj;
    static GLfloat light_diffuse[] = {1.0, 0.0, 0.0, 1.0};
    static GLfloat light_position[] = {1.0, 1.0, 1.0, 0.0};
    
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
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
    // OpenGL END
}

static gboolean
configure_event (GtkWidget         *widget,
                 GdkEventConfigure *event,
                 gpointer           data)
{
    GNASH_REPORT_FUNCTION;

    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
	return FALSE;
    }
    glViewport (0, 0,
		widget->allocation.width, widget->allocation.height);
    
    gdk_gl_drawable_gl_end (gldrawable);
    // OpenGL END
    
    return TRUE;
}

static gboolean
expose_event (GtkWidget      *widget,
              GdkEventExpose *event,
              gpointer        data)
{
    GNASH_REPORT_FUNCTION;

    GdkGLContext *glcontext = gtk_widget_get_gl_context (widget);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (widget);
    
    // OpenGL BEGIN
    if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext))
	return FALSE;
    
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glCallList (1);
    
    if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
	gdk_gl_drawable_swap_buffers (gldrawable);
    } else {
	glFlush ();
    }
    
    gdk_gl_drawable_gl_end (gldrawable);
    // OpenGL END
    
    return TRUE;
}

static void
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

static void
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
    
    print_gl_config_attrib (glconfig, "GDK_GL_USE_GL", GDK_GL_USE_GL, TRUE);
    print_gl_config_attrib (glconfig, "GDK_GL_USE_GL", GDK_GL_USE_GL, TRUE);
    print_gl_config_attrib (glconfig, "GDK_GL_BUFFER_SIZE",      GDK_GL_BUFFER_SIZE,      FALSE);
    print_gl_config_attrib (glconfig, "GDK_GL_LEVEL",            GDK_GL_LEVEL,            FALSE);
    print_gl_config_attrib (glconfig, "GDK_GL_RGBA",             GDK_GL_RGBA,             TRUE);
    print_gl_config_attrib (glconfig, "GDK_GL_DOUBLEBUFFER",     GDK_GL_DOUBLEBUFFER,     TRUE);
    print_gl_config_attrib (glconfig, "GDK_GL_STEREO",           GDK_GL_STEREO,           TRUE);
    print_gl_config_attrib (glconfig, "GDK_GL_AUX_BUFFERS",      GDK_GL_AUX_BUFFERS,      FALSE);
    print_gl_config_attrib (glconfig, "GDK_GL_RED_SIZE",         GDK_GL_RED_SIZE,         FALSE);
    print_gl_config_attrib (glconfig, "GDK_GL_GREEN_SIZE",       GDK_GL_GREEN_SIZE,       FALSE);
    print_gl_config_attrib (glconfig, "GDK_GL_BLUE_SIZE",        GDK_GL_BLUE_SIZE,        FALSE);
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
#endif

// Local Variables:
// mode: C++
// indent-tabs-mode: t
// End:
