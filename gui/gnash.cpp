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

#ifndef USE_KDE
# ifdef GUI_GTK
#  include "gtksup.h"
#  define GUI_CLASS GtkGui
# elif defined(GUI_SDL)
#  include "sdlsup.h"
#  define GUI_CLASS SDLGui
# endif
#else
# ifdef HAVE_KDE
#  include "kdesup.h"
#  include <qapplication.h>
#  define GUI_CLASS KdeGui
# else
#  error "KDE development packages not installed!"
# endif
#endif

#include "gnash.h"
#include "movie_definition.h"

#include "URL.h"
#include "GnashException.h"

using namespace std;
using namespace gnash;

static void usage ();
static void version_and_copyright();

// we don't need to register a file opener anymore, the
// default gnash::globals::streamProvider is good enough
#if 0
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
#endif

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
    int render_arg; // XXX we probably want to be using this.
    char* infile = NULL;
    string url;

    unsigned long windowid = 0;
    bool do_render = true, do_sound = false, sdl_abort = true, 
    	 background = true, do_loop = true;
    unsigned int  delay = 0;
    float scale = 1.0f, exit_timeout = 0;
    long int width = 0, height = 0;
#ifdef USE_KDE
    QApplication *app = new QApplication(argc, argv);
#else
    void *app;
#endif
#if defined(RENDERER_CAIRO)
    unsigned int bit_depth = 32;
#else
    unsigned int bit_depth = 16;
#endif

    assert(tu_types_validate());

    
    // scan for the two main long GNU options
    int c;
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
    
    while ((c = getopt (argc, argv, "hvaps:cfd:x:r:t:b:1ewj:k:u:")) != -1) {
	switch (c) {
	  case 'h':
	      usage ();
          exit(0);
	  case 'v':
              dbglogfile.setVerbosity();
	      dbglogfile << "Verbose output turned on" << endl;
	      break;
	  case 'w':
              dbglogfile.setWriteDisk(true);
	      dbglogfile << "Logging to disk enabled." << endl;
	      break;
	  case 'a':
	      dbglogfile.setActionDump(true); //gnash::set_verbose_action(true);
	      break;
	  case 'p':
	      dbglogfile.setParserDump(true); // gnash::set_verbose_parse(true);
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
#if 0
          // This option is parsed in GtkGlExtGlue::init().
          case 'm':
              tex_lod_bias = (float) atof(optarg);
              break;
#endif
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
      if (argc > 2 && strchr(argv[optind], '=')) {
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

// we don't need to register a file opener anymore, the
// default gnash::globals::streamProvider is good enough
#if 0
    // strk removed this function..
    gnash::register_file_opener_callback(file_opener);
#endif
    gnash::register_fscommand_callback(fs_callback);

    std::auto_ptr<gnash::sound_handler>  sound;

    if (do_sound) {
#ifdef SOUND_SDL
      sound = std::auto_ptr<gnash::sound_handler>
        (gnash::create_sound_handler_sdl());
      gnash::set_sound_handler(sound.get());
#endif
#ifdef SOUND_GST
      sound = std::auto_ptr<gnash::sound_handler>
        (gnash::create_sound_handler_gst());
      gnash::set_sound_handler(sound.get());
#endif
    }

    // Get info about the width & height of the movie.
    int	movie_version = 0, movie_width = 0, movie_height = 0;
    float movie_fps = 30.0f;

    try {
        gnash::get_movie_info(URL(infile), &movie_version, &movie_width,
            &movie_height, &movie_fps, NULL, NULL);
    } catch (const GnashException& er) {
        fprintf(stderr, "%s\n", er.what());
        movie_version = 0;
    }

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

    GUI_CLASS gui(windowid, scale, do_loop, bit_depth);
#ifdef GUI_SDL
    if (!sdl_abort) {
      gui.disableCoreTrap();
    }
#endif

    gui.init(argc, &argv);

    gui.createWindow(width, height);

    // Load the actual movie.
    gnash::movie_definition *md;
 
    try {
      md = gnash::create_library_movie(URL(infile));
    } catch (const GnashException& er) {
      fprintf(stderr, "%s\n", er.what());
      md = NULL;
    }

    gnash::movie_interface *m = create_library_movie_inst(md);
    assert(m);

    gnash::set_current_root(m);

    m->set_display_viewport(0, 0, width, height);
    m->set_background_alpha(background ? 1.0f : 0.05f);

    if (!delay) {
      //delay = (unsigned int) (300 / movie_fps) ; // milliseconds per frame
      delay = (unsigned int) (1000 / movie_fps) ; // milliseconds per frame
    }
    gui.setCallback(delay);

    if (exit_timeout) {
      gui.setTimeout((unsigned int)(exit_timeout * 1000));
    }

    gui.run(app);

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

    return EXIT_SUCCESS;
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
#if 0
        "  -f          Run full speed (no sleep) and log frame rate\n"
        "  -e          Use SDL Event thread\n"
#endif
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
#if 0
        "  CTRL-A          Toggle antialiasing (doesn't work)\n"
        "  CTRL-T          Debug.  Test the set_variable() function\n"
        "  CTRL-G          Debug.  Test the get_variable() function\n"
        "  CTRL-M          Debug.  Test the call_method() function\n"
#endif
        "  CTRL-B          Toggle background color\n"
        );
}
