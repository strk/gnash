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

#ifdef HAVE_SDL_H
#include "SDL.h"
#include "SDL_thread.h"
#endif

#ifdef HAVE_EXTENSIONS
#	include "mysql_db.h"
#endif


#if defined(_WIN32) || defined(WIN32)
# include "getopt.c"
	int mouse_x;
	int mouse_y;
	int mouse_buttons;

	int width;
	int height;
#else
# include <unistd.h>
	extern int mouse_x;
	extern int mouse_y;
	extern int mouse_buttons;

	extern int width;
	extern int height;
#endif

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>

#include <GL/gl.h>
#include <GL/glu.h>
#ifdef HAVE_GTK2
#include <gtk/gtk.h>
#include "gtksup.h"
# ifdef USE_GTKGLEXT
#  include <gdk/gdkx.h>
#  include <gdk/gdkgl.h>
#  include <gtk/gtkgl.h>
# endif
#endif

#include "log.h"
#include "gnash.h"
#include "ogl.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "xmlsocket.h"
#include "movie_definition.h"
#include "URL.h"
#include "GnashException.h"
#include "rc.h"

using namespace std;
using namespace gnash;

static void usage ();
static void version_and_copyright();
static int runThread(void *nothing);

int xml_fd;                     // FIXME: this is the file descriptor
				// from XMLSocket::connect(). This
				// needs to be propogated up through
				// the layers properly, but first I
				// want to make sure it all works.

#define OVERSIZE	1.0f

static int doneYet = 0;

static float	s_scale = 1.0f;
static bool	s_antialiased = false;
static int	s_bit_depth = 16;
static bool	s_background = true;
static bool	s_measure_performance = false;
static bool	s_event_thread = false;
static bool	s_start_waiting = false;

#ifdef GUI_GTK
extern movie_state_e movie_menu_state;
#else
extern SDL_mutex *glMutex;
#endif

#ifndef GUI_GTK
extern int windowid;
#else
extern GdkNativeWindow windowid;
#endif

// Define is you just want a hard coded OpenGL graphic
//#define TEST_GRAPHIC

static void
fs_callback(gnash::movie_interface* movie, const char* command, const char* args)
// For handling notification callbacks from ActionScript.
{
    log_msg("fs_callback: '");
    log_msg(command);
    log_msg("' '");
    log_msg(args);
    log_msg("'\n");
}

#ifndef GUI_GTK
static void
key_event(SDLKey key, bool down)
// For forwarding SDL key events.
{
    gnash::key::code	c(gnash::key::INVALID);
    
    if (key >= SDLK_0 && key <= SDLK_9)	{
        c = (gnash::key::code) ((key - SDLK_0) + gnash::key::_0);
	} else if (key >= SDLK_a && key <= SDLK_z) {
        c = (gnash::key::code) ((key - SDLK_a) + gnash::key::A);
    } else if (key >= SDLK_F1 && key <= SDLK_F15)	{
        c = (gnash::key::code) ((key - SDLK_F1) + gnash::key::F1);
    } else if (key >= SDLK_KP0 && key <= SDLK_KP9) {
        c = (gnash::key::code) ((key - SDLK_KP0) + gnash::key::KP_0);
    } else {
        // many keys don't correlate, so just use a look-up table.
        struct {
            SDLKey	sdlk;
            gnash::key::code	gs;
        } table[] = {
            { SDLK_SPACE, gnash::key::SPACE },
            { SDLK_PAGEDOWN, gnash::key::PGDN },
            { SDLK_PAGEUP, gnash::key::PGUP },
            { SDLK_HOME, gnash::key::HOME },
            { SDLK_END, gnash::key::END },
            { SDLK_INSERT, gnash::key::INSERT },
            { SDLK_DELETE, gnash::key::DELETEKEY },
            { SDLK_BACKSPACE, gnash::key::BACKSPACE },
            { SDLK_TAB, gnash::key::TAB },
            { SDLK_RETURN, gnash::key::ENTER },
            { SDLK_ESCAPE, gnash::key::ESCAPE },
            { SDLK_LEFT, gnash::key::LEFT },
            { SDLK_UP, gnash::key::UP },
            { SDLK_RIGHT, gnash::key::RIGHT },
            { SDLK_DOWN, gnash::key::DOWN },
            // @@ TODO fill this out some more
            { SDLK_UNKNOWN, gnash::key::INVALID }
        };
        
        for (int i = 0; table[i].sdlk != SDLK_UNKNOWN; i++) {
            if (key == table[i].sdlk) {
                c = table[i].gs;
                break;
            }
        }
    }
    
    if (c != gnash::key::INVALID) {
        gnash::notify_key_event(c, down);
    }
}
#endif

int
main(int argc, char *argv[])
{
    int c;
    int render_arg;
    std::vector<const char*> infiles;
    string url;
#ifdef GUI_GTK
    GdkGLConfigMode glcmode;
    gint major, minor;

    GtkWidget *window;
    GtkWidget *drawing_area;

    gtk_init (&argc, &argv);
    gtk_gl_init (&argc, &argv);
    gdk_gl_query_version (&major, &minor);
    dbglogfile << "OpenGL extension version - "
               << (int)major << "." << (int)minor << endl;
    glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB
                                | GDK_GL_MODE_DEPTH
                                | GDK_GL_MODE_DOUBLE);
    glconfig = gdk_gl_config_new_by_mode (glcmode);

    if (glconfig == NULL) {
        dbglogfile << "Cannot find the double-buffered visual." << endl;
        dbglogfile << "Trying single-buffered visual." << endl;
        
        // Try single-buffered visual
        glcmode = (GdkGLConfigMode)(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH);
        glconfig = gdk_gl_config_new_by_mode (glcmode);
        if (glconfig == NULL) {
            dbglogfile << "No appropriate OpenGL-capable visual found." << endl;
            exit (1);
        } else {
            dbglogfile << "Got single-buffered visual." << endl;
        }
    } else {
        dbglogfile << "Got double-buffered visual." << endl;
    }  
//    examine_gl_config_attrib (glconfig);
#endif
    
   assert(tu_types_validate());
    
    float	exit_timeout = 0;
    bool do_render = true;
    bool do_sound = false;
    bool do_loop = true;
    bool sdl_abort = true;
    int  delay = 31;

    float	tex_lod_bias;
    
    // -1.0 tends to look good.
    tex_lod_bias = -1.2f;
    
    // scan for the two main long GNU options
    for (c=0; c<argc; c++) {
        if (strcmp("--help", argv[c]) == 0) {
            version_and_copyright();
            printf("\n");
            usage();
            exit(0);
        }
        if (strcmp("--version", argv[c]) == 0) {
            version_and_copyright();
            exit(0);
        }
    }

    dbglogfile.setWriteDisk(false);

    rcfile.loadFiles();
//    rcfile.dump();

    if (rcfile.useWriteLog()) {
        dbglogfile.setWriteDisk(true);
    }
    
    if (rcfile.verbosityLevel() > 0) {
        dbglogfile.setVerbosity(rcfile.verbosityLevel());
    }
    
    if (rcfile.useActionDump()) {
        dbglogfile.setActionDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.useParserDump()) {
        dbglogfile.setParserDump(true);
        dbglogfile.setVerbosity();
    }
    
    if (rcfile.getTimerDelay() > 0) {
        delay = rcfile.getTimerDelay();
        dbglogfile << "Timer delay set to " << delay << "milliseconds" << endl;
    }
    
    while ((c = getopt (argc, argv, "hvaps:cfd:m:x:r:t:b:1ewj:k:u:")) != -1) {
	switch (c) {
	  case 'h':
	      usage ();
	      break;
	  case 'v':
              dbglogfile.setVerbosity();
	      dbglogfile << "Verbose output turned on" << endl;
	      break;
	  case 'w':
              dbglogfile.setWriteDisk(true);
	      dbglogfile << "Logging to disk enabled." << endl;
	      break;
	  case 'a':
	      dbglogfile.setActionDump(true);
	      break;
	  case 'p':
	      dbglogfile.setParserDump(true);
	      break;
          case 'f':
              s_measure_performance = true;
              break;
          case 's':
              s_scale = fclamp((float) atof(optarg), 0.01f, 100.f);
              break;
          case 'c':
              sdl_abort = false;
              break;
          case 'd':
              delay = strtol(optarg, NULL, 0);
              break;
          case 'u':
              url = optarg;
              dbglogfile << "Setting root URL to: " << width << endl;
              break;
          case 'j':
              width = strtol(optarg, NULL, 0);
              dbglogfile << "Setting width to: " << width << endl;
              break;
          case 'k':
              height = strtol(optarg, NULL, 0);
              dbglogfile << "Setting height to: " << height << endl;
              break;
          case 'e':
              s_event_thread = true;
              break;
          case 'x':
              windowid = strtol(optarg, NULL, 0);
              break;
          case 'm':
              tex_lod_bias = (float) atof(optarg);
              break;
          case '1':
              do_loop = false;
              break;
          case 'r':
              render_arg = strtol(optarg, NULL, 0);
              switch (render_arg) {
                case 0:
                    // Disable both
                    do_render = false;
                    do_sound = false;
                    break;
                case 1:
                    // Enable both
                    do_render = true;
                    do_sound = true;
                    break;
                case 2:
                    // Disable just sound
                    do_render = true;
                    do_sound = false;
                    break;
                default:
                    cerr << "-r must be followed by 0, 1 or 2 (" << 
                        render_arg << " is invalid" << endl;
                    
                    break;
              };
              break;
          case 't':
              exit_timeout = (float) atof(optarg);
              break;
          case 'b':
              s_bit_depth = atoi(optarg);
              break;
	}
    }
    
    // get the file name from the command line
    while (optind < argc) {
        // Some options set variables, like ip=127.0.0.1
        if (strchr(argv[optind], '=')) {
            dbglogfile << "Got variable option on command line!" << endl;
        } else {
            infiles.push_back(argv[optind]);
        }
	optind++;
    }

    // Remove the logfile that's created by default, since leaving a short
    // file is confusing.
    if (dbglogfile.getWriteDisk() == false) {
        dbglogfile.removeLog();
    }

    // No file names were supplied
    if (infiles.size() == 0) {
	printf("no input files\n");
	usage();
	exit(1);
    }

    //gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);
	
#ifdef HAVE_EXTENSIONS
		gnash::register_component("mysql_db", mysqldb::constructor);
#endif

    gnash::sound_handler  *sound = NULL;
    if (do_render) {
        if (do_sound) {
#ifdef SOUND_SDL
            sound = gnash::create_sound_handler_sdl();
            gnash::set_sound_handler(sound);
#endif
#ifdef SOUND_GST
            sound = gnash::create_sound_handler_gst();
            gnash::set_sound_handler(sound);
#endif
        }
    }
    
    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float	movie_fps = 30.0f;
    try {
      gnash::get_movie_info(URL(infiles[0]), &movie_version, &movie_width,
                          &movie_height, &movie_fps, NULL, NULL);
    } catch (const GnashException& er) {
        fprintf(stderr, "%s\n", er.what());
        movie_version = 0;
    }
    if (movie_version == 0) {
        fprintf(stderr, "error: can't get info about %s\n", infiles[0]);
        exit(1);
    }

    if (!width) {    
        width = int(movie_width * s_scale);
    }
    if (!height) {
        height = int(movie_height * s_scale);
    }

    gnash::render_handler* render = NULL;
    
    if (do_render) {
#ifndef GUI_GTK
        if (windowid) {
            char SDL_windowhack[32];
            sprintf (SDL_windowhack,"SDL_WINDOWID=%d", windowid);
            putenv (SDL_windowhack);
        }
        
        // Initialize the SDL subsystems we're using. Linux
        // and Darwin use Pthreads for SDL threads, Win32
        // doesn't. Otherwise the SDL event loop just polls.
        if (sdl_abort) {
            //  Other flags are SDL_INIT_JOYSTICK | SDL_INIT_CDROM
#if defined(_WIN32) || defined(WIN32)
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
#else
                if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTTHREAD ))
#endif
                    {
                        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
                        exit(1);
                    }
        } else {
            fprintf(stderr, "warning: SDL won't trap core dumps \n");
#if defined(_WIN32) || defined(WIN32)
            if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE  | SDL_INIT_EVENTTHREAD))
#else
                if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_NOPARACHUTE))
#endif
                    {
                        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
                        exit(1);
                    }
        }
        
        atexit(SDL_Quit);
        
        SDL_EnableKeyRepeat(250, 33);
        
        
        if (s_bit_depth == 16) {
            // 16-bit color, surface creation is likely to succeed.
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 15);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
        } else {
            assert(s_bit_depth == 32);
            
            // 32-bit color etc, for getting dest alpha,
            // for MULTIPASS_ANTIALIASING (see
            // render_handler_ogl.cpp).
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);
        }
#else
        if (windowid) {
            window = gtk_plug_new(windowid);
            dbglogfile << "Created GTK Plug window" << endl;
        } else {
            window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
            dbglogfile << "Created top level window" << endl;
        }
//         if (!glconfig) {
//             static const int attrib_list[] = {
//                 // GDK_GL_ALPHA_SIZE, 1,
//                 GDK_GL_DOUBLEBUFFER,
//                 GDK_GL_DEPTH_SIZE, 1,
//                 GDK_GL_RGBA,
//                 GDK_GL_RED_SIZE, 8,
//                 GDK_GL_ATTRIB_LIST_NONE
//             };
//             glconfig = gdk_gl_config_new(attrib_list);
//         }
        
        gtk_window_set_title(GTK_WINDOW (window), "Gnash Player");
        gtk_container_set_reallocate_redraws(GTK_CONTAINER (window), TRUE);
        g_signal_connect(G_OBJECT(window), "delete_event",
                         G_CALLBACK(delete_event), NULL);
        g_signal_connect(G_OBJECT(window), "key_press_event",
                         G_CALLBACK(key_press_event), NULL);

        GtkMenu *popup_menu = GTK_MENU(gtk_menu_new());
        
        drawing_area = gtk_drawing_area_new();
        if (!windowid) {
            gtk_widget_set_size_request(drawing_area, width, height);
        }
        // Set OpenGL-capability to the widget.
        gtk_widget_set_gl_capability(drawing_area, glconfig,
                                      NULL, TRUE, GDK_GL_RGBA_TYPE);
        g_signal_connect_after(G_OBJECT (drawing_area), "realize",
                                G_CALLBACK (realize_event), NULL);
        g_signal_connect(G_OBJECT (drawing_area), "configure_event",
                          G_CALLBACK (configure_event), NULL);
        g_signal_connect(G_OBJECT (drawing_area), "expose_event",
                          G_CALLBACK (expose_event), NULL);
        g_signal_connect(G_OBJECT (drawing_area), "unrealize",
                          G_CALLBACK (unrealize_event), NULL);
        
        gtk_widget_add_events(drawing_area, GDK_EXPOSURE_MASK
                              | GDK_BUTTON_PRESS_MASK
                              | GDK_BUTTON_RELEASE_MASK
                              | GDK_KEY_RELEASE_MASK
                              | GDK_KEY_PRESS_MASK        
                              | GDK_POINTER_MOTION_MASK);
        
        g_signal_connect_swapped(G_OBJECT(drawing_area),
                                 "button_press_event",
                                 G_CALLBACK(popup_handler),
                                 GTK_OBJECT(popup_menu));

        add_menuitems(popup_menu);
        gtk_widget_realize(window);
      
        g_signal_connect(G_OBJECT(drawing_area), "button_press_event",
                         G_CALLBACK(button_press_event), NULL);
        g_signal_connect(G_OBJECT(drawing_area), "button_release_event",
                         G_CALLBACK(button_release_event), NULL);
        g_signal_connect(G_OBJECT(drawing_area), "motion_notify_event",
                         G_CALLBACK(motion_notify_event), NULL);

        gtk_container_add(GTK_CONTAINER(window), drawing_area);
        gtk_widget_show(drawing_area);      
        gtk_widget_show(window);
#endif
	render = gnash::create_render_handler_ogl();
	gnash::set_render_handler(render);


        // Change the LOD BIAS values to tweak blurriness.
        if (tex_lod_bias != 0.0f) {
            glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, tex_lod_bias);
        }
#ifndef GUI_GTK
        // Set the video mode.
        if (SDL_SetVideoMode(width, height, s_bit_depth, SDL_OPENGL) == 0) {
            fprintf(stderr,
	    	"SDL_SetVideoMode(%d, %d, %d, SDL_OPENGL) failed: %s\n",
		width, height, s_bit_depth, SDL_GetError());
            exit(1);
        }

				glOrtho(-OVERSIZE, OVERSIZE, OVERSIZE, -OVERSIZE, -1, 1);

        // Set the window title
        char *window_title = new char[1+strlen("gnash: ")+strlen(infiles[0])];
        strcpy(window_title, "gnash: ");
        strcat(window_title, infiles[0]);
        SDL_WM_SetCaption(window_title, window_title);
        //
        ogl::open();
#endif
    }
    
    // Load the actual movie.
    gnash::movie_definition* md;
    try {
      md = gnash::create_library_movie(URL(infiles[0]));
    } catch (const GnashException& er) {
      fprintf(stderr, "%s\n", er.what());
      md = NULL;
    }
    if (md == NULL) {
        fprintf(stderr, "error: can't create a movie from '%s'\n", infiles[0]);
        exit(1);
    }
    gnash::movie_interface*	m = create_library_movie_inst(md);
    if (m == NULL) {
        fprintf(stderr, "error: can't create movie instance\n");
        exit(1);
    }
    gnash::set_current_root(m);
    
    // Mouse state.
    float	speed_scale = 1.0f;
    uint32_t	start_ticks = 0;
    if (do_render) {
        start_ticks = SDL_GetTicks();
        
    }
    uint32_t	last_ticks = start_ticks;
    int	frame_counter = 0;
    int	last_logged_fps = last_ticks;
    
    
    SDL_Thread *thread = NULL;
    if (s_event_thread) {
        thread = SDL_CreateThread(runThread, NULL);
        if ( thread == NULL) {
            fprintf(stderr, "Unable to create thread: %s\n", SDL_GetError());
        }
    }
    
    
    for (;;) {
        uint32_t	ticks;
        if (do_render) {
            ticks = SDL_GetTicks();
        } else {
            // Simulate time.
            ticks = last_ticks + (uint32_t) (1000.0f / movie_fps);
        }
        int	delta_ticks = ticks - last_ticks;
        float	delta_t = delta_ticks / 1000.f;
        last_ticks = ticks;
        
        // Check auto timeout counter.
        if (exit_timeout > 0
            && ticks - start_ticks > (uint32_t) (exit_timeout * 1000)) {
            // Auto exit now.
            break;
        }
        
        if (do_render) {
#ifndef GUI_GTK
            SDL_Event	event;
            bool ret = true;
            // Handle input.
            while (ret) {
//           printf("xml_fd is %d, gofast is %d, s_start_waiting is %d, s_event_thread is %d\n",
//                  xml_fd, gofast, s_start_waiting, s_event_thread);
#ifdef HAVE_LIBXML
                if (s_event_thread && s_start_waiting && (xml_fd > 0)) {
                    // 				if (s_event_thread && (xml_fd > 0)) {
//            printf("SDL_WaitEvent!\n");
                    ret = SDL_WaitEvent(&event);
                } else {
//          if (gofast) {
//               printf("SDL_PollEvent GOFAST!\n");
//           } else {
//               printf("SDL_PollEvent!\n");
//           }
                    ret = SDL_PollEvent(&event) ? true : false;
                }
#else
                // If we have no libxml, obey what the gofast variable is set to
//                 if (gofast)
                    ret = SDL_PollEvent(&event) ? true : false;
//                 else
//                     ret = SDL_WaitEvent(&event);
                    if (ret == false) break;
#endif
                
//        printf("EVENT Type is %d\n", event.type);
                switch (event.type) {
                  case SDL_NOEVENT:
                      ret = false;
                      break;
                  case SDL_USEREVENT:
//              printf("SDL_USER_EVENT at %s, code %d%d\n", __FUNCTION__, __LINE__, event.user.code);
                      ret = false;
                      break;
                  case SDL_KEYDOWN:
                  {
                      SDLKey	key = event.key.keysym.sym;
                      bool	ctrl = (event.key.keysym.mod & KMOD_CTRL) != 0;
                      
                      if (key == SDLK_ESCAPE
                          || (ctrl && key == SDLK_q)
                          || (ctrl && key == SDLK_w)) {
                          goto done;
                      } else if (ctrl && key == SDLK_p) {
                          // Toggle paused state.
                          if (m->get_play_state() == gnash::movie_interface::STOP) {
                              m->set_play_state(gnash::movie_interface::PLAY);
                          } else {
                              m->set_play_state(gnash::movie_interface::STOP);
                          }
                      } else if (ctrl && key == SDLK_r) {
                          // Restart the movie.
                          m->restart();
                      } else if (ctrl && (key == SDLK_LEFTBRACKET || key == SDLK_KP_MINUS)) {
                          m->goto_frame(m->get_current_frame()-1);
                      } else if (ctrl && (key == SDLK_RIGHTBRACKET || key == SDLK_KP_PLUS)) {
                          m->goto_frame(m->get_current_frame()+1);
                      } else if (ctrl && key == SDLK_a) {
                          // Toggle antialiasing.
                          s_antialiased = !s_antialiased;
                          //gnash::set_antialiased(s_antialiased);
                      } else if (ctrl && key == SDLK_t) {
                          // test text replacement / variable setting:
                          m->set_variable("test.text", "set_edit_text was here...\nanother line of text for you to see in the text box\nSome UTF-8: Г±Г¶ВЈГ§В°Г„ГЂГ”Вї");
                      } else if (ctrl && key == SDLK_g) {
                          // test get_variable.
                          log_msg("testing get_variable: '");
                          log_msg(m->get_variable("test.text"));
                          log_msg("'\n");
                      } else if (ctrl && key == SDLK_m) {
                          // Test call_method.
                          const char* result = m->call_method(
                              "test_call",
                              "%d, %f, %s, %ls",
                              200,
                              1.0f,
                              "Test string",
                              L"Test long string");
                          
                          if (result) {
                              log_msg("call_method: result = ");
                              log_msg(result);
                              log_msg("\n");
                          } else {
                              log_msg("call_method: null result\n");
                          }
                      } else if (ctrl && key == SDLK_b) {
                          // toggle background color.
                          s_background = !s_background;
                      } else if (ctrl && key == SDLK_f) {
                          extern bool gnash_debug_show_paths;
                          gnash_debug_show_paths = !gnash_debug_show_paths;
                      } else if (ctrl && key == SDLK_EQUALS) {
                          float	f = gnash::get_curve_max_pixel_error();
                          f *= 1.1f;
                          gnash::set_curve_max_pixel_error(f);
                          printf("curve error tolerance = %f\n", f);
                      } else if (ctrl && key == SDLK_MINUS) {
                          float	f = gnash::get_curve_max_pixel_error();
                          f *= 0.9f;
                          gnash::set_curve_max_pixel_error(f);
                          printf("curve error tolerance = %f\n", f);
                      }
                      
                      key_event(key, true);
                      
                      break;
                  }
                  
                  case SDL_KEYUP:
                  {
                      SDLKey	key = event.key.keysym.sym;
                      key_event(key, false);
                      break;
                  }
                  
                  case SDL_MOUSEMOTION:
                      mouse_x = (int) event.motion.x;
	                    mouse_y = (int) event.motion.y;
											break;
                      
                  case SDL_MOUSEBUTTONDOWN:
                  case SDL_MOUSEBUTTONUP:
                  {
                      int	mask = 1 << (event.button.button - 1);
                      if (event.button.state == SDL_PRESSED) {
                          mouse_buttons |= mask;
                      } else {
                          mouse_buttons &= ~mask;
                      }
                      break;
                  }
                  
                  case SDL_QUIT:
                      goto done;
                      break;
                      
                  default:
                      break;
                }
            }
#else
            // Poll for events instead of letting gtk_main() handle them
            while (gtk_events_pending ()) {
//                dbglogfile << "Making GTK main iteration!" << endl;
                switch (movie_menu_state) {
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
                      if ((m->get_current_frame()+10) < md->get_frame_count()) {
                          m->goto_frame(m->get_current_frame()+10);
                      }
                      break;
                  case QUIT_MOVIE:
                      goto done;
                      break;
                  default:
                      break;
                };
                movie_menu_state = IDLE_MOVIE;
                gtk_main_iteration();
            }
#endif
        }

#ifndef TEST_GRAPHIC
//    printf("%s(%d): Frame count is %d\n", __PRETTY_FUNCTION__, __LINE__,
//           md->get_frame_count());
        m = gnash::get_current_root();
        gnash::delete_unused_root();
#ifdef GUI_GTK
        glcontext = gtk_widget_get_gl_context (drawing_area);
        GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (drawing_area);
        if (!gdk_gl_drawable_gl_begin (gldrawable, glcontext)) {
            dbglogfile << "ERROR: Couldn't start drawable!" << endl;
        }
#endif
        m->set_display_viewport(0, 0, width, height);
        m->set_background_alpha(s_background ? 1.0f : 0.05f);
        
				m->notify_mouse_state((int) (mouse_x * ((float)movie_width / width)),
															(int) (mouse_y * ((float)movie_height / height)),
															mouse_buttons);
        
        m->advance(delta_t *speed_scale);
        
        if (do_render) {
            glDisable(GL_DEPTH_TEST);	// Disable depth testing.
            glDrawBuffer(GL_BACK);
        }
        m->display();
        frame_counter++;

#ifdef GUI_GTK
        if (gdk_gl_drawable_is_double_buffered (gldrawable)) {
            gdk_gl_drawable_swap_buffers (gldrawable);
        } else {
            glFlush();
        }

        gdk_gl_drawable_gl_end (gldrawable);
#endif
#else
        GdkGLContext *glcontext = gtk_widget_get_gl_context (drawing_area);
        GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable (drawing_area);

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
    
        glViewport (0, 0, width, height);
        
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
        
        if (do_render) {
#ifndef GUI_GTK
            SDL_GL_SwapBuffers();
            //glPopAttrib ();
#endif
            
            if (s_measure_performance == false) {
                // Don't hog the CPU.
                // 				if (!nodelay)
//                 if (!gofast) {
                    SDL_Delay(delay);
//                 }
            } else {
                // Log the frame rate every second or so.
                if (last_ticks - last_logged_fps > 1000) {
                    float	delta = (last_ticks - last_logged_fps) / 1000.f;
                    
                    if (delta > 0) {
                        printf("fps = %3.1f\n", frame_counter / delta);
                    } else {
                        printf("fps = *inf*\n");
                    }
                    
                    last_logged_fps = last_ticks;
                    frame_counter = 0;
                }
            }
        }

        // See if we should exit.
        if (do_loop == false
            && m->get_current_frame() + 1 == md->get_frame_count())
            {
                // We're reached the end of the movie; exit.
                break;
            }
    }
    
  done:
    
    doneYet = 1;
    SDL_KillThread(thread);	// kill the network read thread
    //SDL_Quit();
    
    if (md) {
        md->drop_ref();
    }
    
    if (m) {
        m->drop_ref();
    }
    delete sound;
    gnash::set_sound_handler(NULL);
    delete render;
		gnash::set_render_handler(NULL);
    
    // For testing purposes, throw some keypresses
    // to make sure the key handler is properly using weak
    // references to listeners.
    gnash::notify_key_event(gnash::key::A, true);
    gnash::notify_key_event(gnash::key::B, true);
    gnash::notify_key_event(gnash::key::C, true);
    
    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();
    
    return 0;
}

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
    
    printf("Initializing event thread...\n");
    
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
    
    printf("Enabling Event Wait Mode...\n");
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

void
version_and_copyright()
{
    printf (
"Gnash " VERSION "\n"
"Copyright (C) 2006 Free Software Foundation, Inc.\n"
"Gnash comes with NO WARRANTY, to the extent permitted by law.\n"
"You may redistribute copies of Gnash under the terms of the GNU General\n"
"Public License.  For more information, see the file named COPYING.\n"
	);
}


void
usage()
{
    printf(
        "usage: gnash [options] movie_file.swf\n"
        "\n"
        "Plays a SWF (Shockwave Flash) movie\n"
        "options:\n"
        "\n"
        "  -h, --help  Print this info.\n"
        "  -s <factor> Scale the movie up/down by the specified factor\n"
        "  -c          Produce a core file instead of letting SDL trap it\n"
        "  -d num      Number of milliseconds to delay in main loop\n"
        "  -v          Be verbose; i.e. print log messages to stdout\n"
        "  -va         Be verbose about movie Actions\n"
        "  -vp         Be verbose about parsing the movie\n"
        "  -m <bias>   Specify the texture LOD bias (float, default is -1)\n"
        "  -f          Run full speed (no sleep) and log frame rate\n"
//         "  -e          Use SDL Event thread\n"
        "  -x <ID>     X11 Window ID for display\n"
        "  -w          Produce the disk based debug log\n"
        "  -1          Play once; exit when/if movie reaches the last frame\n"
        "  -r <0|1|2>  0 disables rendering & sound (good for batch tests)\n"
        "              1 enables rendering & sound (default setting)\n"
        "              2 enables rendering & disables sound\n"
        "  -t <sec>    Timeout and exit after the specified number of seconds\n"
        "  -b <bits>   Bit depth of output window (16 or 32, default is 16)\n"
        "  --version   Print gnash's version number and exit\n"
        "\n"
        "keys:\n"
        "  CTRL-Q, CTRL-W, ESC   Quit/Exit\n"
        "  CTRL-P          Toggle Pause\n"
        "  CTRL-R          Restart the movie\n"
        "  CTRL-[ or kp-   Step back one frame\n"
        "  CTRL-] or kp+   Step forward one frame\n"
//        "  CTRL-A          Toggle antialiasing (doesn't work)\n"
//        "  CTRL-T          Debug.  Test the set_variable() function\n"
//        "  CTRL-G          Debug.  Test the get_variable() function\n"
//        "  CTRL-M          Debug.  Test the call_method() function\n"
        "  CTRL-B          Toggle background color\n"
        );
}
