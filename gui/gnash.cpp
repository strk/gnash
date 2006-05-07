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
#include "sdlsup.h"
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
#include <sys/time.h>

using namespace std;
using namespace gnash;

static void usage ();
static void version_and_copyright();
static bool advance_movie(void* data);

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
    char* infile = NULL;
    string url;

    
    assert(tu_types_validate());

    unsigned long windowid = 0;
    float	exit_timeout = 0;
    bool do_render = true;
    bool do_sound = false;
    bool sdl_abort = true;
    unsigned int  delay = 0;
    float	tex_lod_bias;
    float	scale = 1.0f;
    bool background = true;
    long int width = 0, height = 0;
    bool     do_loop = true;
#if defined(RENDERER_CAIRO)
    unsigned int bit_depth = 32;
#else
    unsigned int bit_depth = 16;
#endif
    
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
#if 0
          case 'f':
              s_measure_performance = true;
              break;
#endif
          case 's':
              scale = fclamp((float) atof(optarg), 0.01f, 100.f);
              break;
          case 'c':
              sdl_abort = false;
              break;
          case 'd':
              delay = strtol(optarg, NULL, 0);
              break;
          case 'u':
              url = optarg;
              dbglogfile << "Setting root URL to: " << url << endl;
              break;
          case 'j':
              width = strtol(optarg, NULL, 0);
              dbglogfile << "Setting width to: " << width << endl;
              break;
          case 'k':
              height = strtol(optarg, NULL, 0);
              dbglogfile << "Setting height to: " << height << endl;
              break;
#if 0
          case 'e':
              s_event_thread = true;
              break;
#endif
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
                        render_arg << ") is invalid" << endl;
                    break;
              };
              break;
          case 't':
              exit_timeout = (float) atof(optarg);
              break;
          case 'b':
              bit_depth = atoi(optarg);
              assert (!bit_depth || bit_depth == 16 || bit_depth == 32);
              break;
	}
    }

    // get the file name from the command line
    while (optind < argc) {
      // Some options set variables, like ip=127.0.0.1
      if (strchr(argv[optind], '=')) {
	  dbglogfile << "Got variable option on command line!" << endl;
      } else {
	  infile = argv[optind];
	  break;
      }
      optind++;
    }

    // Remove the logfile that's created by default, since leaving a short
    // file is confusing.
    if (dbglogfile.getWriteDisk() == false) {
        dbglogfile.removeLog();
    }

    // No file name was supplied
    if (!infile) {
	std::cerr << "Error: no input file was specified." << endl << endl;
	usage();
	return EXIT_FAILURE;
    }

    gnash::register_file_opener_callback(file_opener);
    gnash::register_fscommand_callback(fs_callback);

    std::auto_ptr<gnash::sound_handler>  sound;

    if (do_sound) {
#ifdef HAVE_SDL_MIXER_H
      sound = std::auto_ptr<gnash::sound_handler>
        (gnash::create_sound_handler_sdl());
      gnash::set_sound_handler(sound.get());
#endif
    }

    // Get info about the width & height of the movie.
    int	movie_version = 0;
    int	movie_width = 0;
    int	movie_height = 0;
    float	movie_fps = 30.0f;
    gnash::get_movie_info(infile, &movie_version, &movie_width,
                          &movie_height, &movie_fps, NULL, NULL);
    if (movie_version == 0) {
      std::cerr << "Error: can't get info about " << infile << "." << endl;
      return EXIT_FAILURE;
    }

    if (!width) {
      width = int(movie_width * scale);
    }
    if (!height) {
      height = int(movie_height * scale);
    }

#define GUI_CLASS GtkGui
#define GUI_GTK 1


    GUI_CLASS gui(windowid, scale, do_loop, bit_depth);
#ifdef GUI_SDL
    if (!sdl_abort) {
      gui.disableCoreTrap();
    }
#endif

#if defined(RENDERER_OPENGL) && defined(FIX_I810_LOD_BIAS)
    gui->setLodBias(tex_lod_bias);
#endif

    gui.init(argc, &argv);

    gui.createWindow(width, height);

    // Load the actual movie.
    smart_ptr<gnash::movie_definition>	md = gnash::create_library_movie(infile);
    if (!md.get_ptr())
      return EXIT_FAILURE;

    smart_ptr<gnash::movie_interface>	m = create_library_movie_inst(md.get_ptr());
    assert(m.get_ptr());

    gnash::set_current_root(m.get_ptr());

    m->set_display_viewport(0, 0, width, height);
    m->set_background_alpha(background ? 1.0f : 0.05f);

    if (!delay) {
      delay = (unsigned int) (1000 / movie_fps) ; // milliseconds per frame
    }
    gui.setCallback(advance_movie, delay);

    if (exit_timeout) {
      gui.setTimeout((unsigned int)(exit_timeout * 1000));
    }

    gui.run();

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

    return EXIT_SUCCESS;
}

static bool
advance_movie(void* data)
{
      Gui *gui = static_cast<Gui*> (data);
      gnash::movie_interface* m = gnash::get_current_root();

      m->notify_mouse_state(gui->getMouseX(), gui->getMouseY(), gui->getMouseButtons());

      m->advance(1.0);
      m->display();

      gui->renderBuffer();

      if (!gui->loops()) {
        if (m->get_current_frame() + 1 ==
            m->get_root_movie()->get_movie_definition()->get_frame_count()) {
          exit(0); // TODO: quit in a more gentile fashion.
        }
      }

      return true;
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
        "  -m <bias>   Specify the texture LOD bias (float, default is -1.0)\n"
//         "  -f          Run full speed (no sleep) and log frame rate\n"
//         "  -e          Use SDL Event thread\n"
        "  -x <ID>     X11 Window ID for display\n"
        "  -w          Produce the disk based debug log\n"
        "  -1          Play once; exit when/if movie reaches the last frame\n"
        "  -r <0|1|2>  0 disables rendering & sound (good for batch tests)\n"
        "              1 enables rendering & sound\n"
        "              2 enables rendering & disables sound (default)\n"
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
