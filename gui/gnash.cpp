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

#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <vector>


#  ifdef HAVE_GTK2
#    include "gtksup.h"
#  endif

#include "gnash.h"
#include "log.h"
#include "utility.h"
#include "container.h"
#include "tu_file.h"
#include "tu_types.h"
#include "Movie.h"

using namespace std;
using namespace gnash;

static void usage ();
static void version_and_copyright();

static float	s_scale = 1.0f;
static int	s_bit_depth = 16;
static bool	s_background = true;
static bool	s_measure_performance = false;
static bool	s_event_thread = false;

static long int windowid = 0, width, height;

static tu_file*
file_opener(const char* url)
// Callback function.  This opens files for the library.
{
//    GNASH_REPORT_FUNCTION;

    if (strcmp(url, "-") == 0) {
        FILE *newin = fdopen(dup(0),"rb");
        return new tu_file(newin, false);
    } else {
        return new tu_file(url, "rb");
    }
}

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

int
main(int argc, char *argv[])
{
    int c;
    int render_arg;
    std::vector<const char*> infiles;
    string url;

    GtkGui gui;
    gui.init(argc, &argv);
    
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
	      gnash::set_verbose_action(true);
	      break;
	  case 'p':
	      gnash::set_verbose_parse(true);
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

    gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);

    std::auto_ptr<gnash::sound_handler>  sound;
    if (do_render) {
        if (do_sound) {
#ifdef HAVE_SDL_MIXER_H
            std::auto_ptr<gnash::sound_handler> tmp(gnash::create_sound_handler_sdl());
            sound = tmp;
            gnash::set_sound_handler(sound.get());
#endif
        }
    }

    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float	movie_fps = 30.0f;
    gnash::get_movie_info(infiles[0], &movie_version, &movie_width,
                          &movie_height, &movie_fps, NULL, NULL);
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

    if (do_render) {
      if (windowid) {
        gui.createWindow(width, height, windowid);
      } else {
        gui.createWindow(width, height);
      }

#ifdef FIX_I810_LOD_BIAS	
        // Change the LOD BIAS values to tweak blurriness.
        if (tex_lod_bias != 0.0f) {
          gui.fixLodBias();
        }
#endif // FIX_I810_LOD_BIAS
    }

    // Load the actual movie.
    smart_ptr<gnash::movie_definition>	md = gnash::create_library_movie(infiles[0]);
    if (!md.get_ptr()) {
        fprintf(stderr, "error: can't create a movie from '%s'\n", infiles[0]);
        exit(1);
    }
    smart_ptr<gnash::movie_interface>	m = create_library_movie_inst(md.get_ptr());
    if (!m.get_ptr()) {
        fprintf(stderr, "error: can't create movie instance\n");
        exit(1);
    }
    gnash::set_current_root(m.get_ptr());

    float	speed_scale = 1.0f;
    Uint32	start_ticks = 0;
    if (do_render) {
        start_ticks = SDL_GetTicks();
    }
    Uint32	last_ticks = start_ticks;
    int	frame_counter = 0;
    int	last_logged_fps = last_ticks;

    gui.setupEvents();

    // This is the main event loop.
    for (;;) {
        Uint32	ticks;
        if (do_render) {
            ticks = SDL_GetTicks();
        } else {
            // Simulate time.
            ticks = last_ticks + (Uint32) (1000.0f / movie_fps);
        }
        int	delta_ticks = ticks - last_ticks;
        float	delta_t = delta_ticks / 1000.f;
        last_ticks = ticks;

        // Check auto timeout counter.
        if (exit_timeout > 0
            && ticks - start_ticks > (Uint32) (exit_timeout * 1000)) {
            // Auto exit now.
            break;
        }

        if (do_render) {
            gui.poll(m.get_ptr(), md.get_ptr());
        }

#if 0
        printf("%s(%d): Frame count is %d\n", __PRETTY_FUNCTION__, __LINE__,
               md->get_frame_count());
#endif
        m = gnash::get_current_root();
        gnash::delete_unused_root();

        m->set_display_viewport(0, 0, width, height);
        m->set_background_alpha(s_background ? 1.0f : 0.05f);

        m->notify_mouse_state(gui.getMouseX(), gui.getMouseY(), gui.getMouseButtons());

        m->advance(delta_t *speed_scale);

        if (do_render) {
#ifdef RENDERER_OPENGL
// XXX we really want to do this in the opengl renderer handler itself
	gui.swapBuffers();
#endif
        }
        m->display();
        frame_counter++;

        
        if (do_render) {

            if (!s_measure_performance) {
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
            && m->get_current_frame() + 1 == md->get_frame_count()) {
          // We're reached the end of the movie; exit.
          break;
        }
    }
    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

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
