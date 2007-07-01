// gnash.cpp:  Main routine for top-level flash player, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Player.h"
#include "log.h"
#include "rc.h" // for use of rcfile
#include "debugger.h"

#ifdef HAVE_FFMPEG_AVCODEC_H
# include "ffmpeg/avcodec.h"
#endif

#ifdef HAVE_GST_GST_H
# include "gst/gst.h"
# include "gst/gstversion.h"
#endif

#if defined(_WIN32) || defined(WIN32)
        #include "getopt_win32.h"
#else
extern "C"{
        #include <unistd.h>
#ifdef HAVE_GETOPT_H
	#include <getopt.h>
#endif
#ifndef __GNUC__
        extern int optind, getopt(int, char *const *, const char *);
	extern char *optarg; // global argument pointer
#endif
}
#endif // ! Win32


#include <string>
#include <iostream>

#ifdef ENABLE_NLS
#include <locale.h>
#endif

using namespace gnash; // for log_*

using namespace std;

char* infile = NULL;
char* url = NULL;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
#ifdef USE_DEBUGGER
gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

//extern bool g_debug;

static void
usage()
{
    printf("%s%s%s%s%s%s%s%s", _(
        "usage: gnash [options] movie_file.swf\n"
        "\n"
        "Plays a SWF (Shockwave Flash) movie\n"
        "options:\n"
        "\n"), _(
        "  -h, --help   Print this info.\n"
        "  -s <factor>  Scale the movie up/down by the specified factor\n"
        "  -c           Produce a core file instead of letting SDL trap it\n"
        "  -d num       Number of milliseconds to delay in main loop\n"
        "  -v           Be verbose; i.e. print log messages to stdout\n"
		),
#if VERBOSE_ACTION
      _("  -va          Be verbose about movie Actions\n"),
#else
	"",
#endif
#if VERBOSE_PARSE
      _("  -vp          Be verbose about parsing the movie\n"),
#else
	"",
#endif
		  _(
        "  -m <bias>    Specify the texture LOD bias (float, default is -1.0)\n"
        "  -x <ID>      X11 Window ID for display\n"
        "  -w           Produce the disk based debug log\n"
		"  -j <width>   Set window width\n"
		"  -k <height>  Set window height\n"
        "  -1           Play once; exit when/if movie reaches the last frame\n"
		), _(
        "  -g           Turn on the Flash debugger\n"
        "  -r <0|1|2|3>\n"
		"               0 disables both rendering & sound (good for batch tests)\n"
        "               1 enables rendering & disables sound\n"
        "               2 enables sound & disables rendering\n"
        "               3 enables both rendering & sound (default)\n"
		), _(
        "  -t <sec>     Timeout and exit after the specified number of seconds\n"
        "  -b <bits>    Bit depth of output window (16 or 32, default is 16)\n"
        "  -u <url>     Set \"real\" url of the movie\n"
		"               (useful for downloaded movies)\n"
        "  -U <url>     Set \"base\" url for this run\n"
		"               (used to resolve relative urls, defaults to movie url)\n"
        "  -P <param>   Set parameter (ie. \"FlashVars=A=1&b=2\")\n"
        "  -V,--version Print gnash's version number and exit\n"
		), _(
        "\n"
        "keys:\n"
        "  CTRL-Q, CTRL-W, ESC   Quit/Exit\n"
        "  CTRL-P          Toggle Pause\n"
        "  CTRL-R          Restart the movie\n"
        "  CTRL-[ or kp-   Step back one frame\n"
        "  CTRL-] or kp+   Step forward one frame\n"
        "  CTRL-B          Toggle background color\n"
/*
        "  CTRL-A          Toggle antialiasing (doesn't work)\n"
        "  CTRL-T          Debug.  Test the set_variable() function\n"
        "  CTRL-G          Debug.  Test the get_variable() function\n"
        "  CTRL-M          Debug.  Test the call_method() function\n"
*/
        ));
}

static void version_and_copyright()
{
    printf (_(
"Gnash " VERSION "\n"
"Copyright (C) 2005-2007 Free Software Foundation, Inc.\n"
"Gnash comes with NO WARRANTY, to the extent permitted by law.\n"
"You may redistribute copies of Gnash under the terms of the GNU General\n"
"Public License.  For more information, see the file named COPYING.\n"
	));
}


static void build_options()
{
    printf (_("Build options %s\n"
              "   Target: %s\n"
              "   Renderer: %s   GUI: %s   Media handler: %s\n"
              "   Configured with: %s\n"),
		VERSION, TARGET_CONFIG, RENDERER_CONFIG, GUI_CONFIG,
                MEDIA_CONFIG, CONFIG_CONFIG);
#ifdef HAVE_FFMPEG_AVCODEC_H
    printf(_("Ffmpeg version is: %s\n"), LIBAVCODEC_IDENT);
#endif
#ifdef HAVE_GST_GST_H
    printf(_("Gstreamer version is: %d.%d.%d."), GST_VERSION_MAJOR,
           GST_VERSION_MINOR, GST_VERSION_MICRO);
#endif
}


static void
parseCommandLine(int argc, char* argv[], gnash::Player& player)
{
    bool specified_rendering_flag=false;
    bool called_by_plugin=false;
    bool width_given=false, height_given=false;

    int c = 0;
    // scan for the two main long GNU options
    for (; c < argc; c++) {
        if (strcmp("--help", argv[c]) == 0) {
            version_and_copyright();
            printf("\n");
            usage();
            build_options();
	    exit(EXIT_SUCCESS);
        }
        if (strcmp("--version", argv[c]) == 0) {
            version_and_copyright();
            build_options();
	    exit(EXIT_SUCCESS);
        }
    }
    
    while ((c = getopt (argc, argv, "hvaps:cd:x:r:t:b:1wj:k:u:P:U:g:V")) != -1)
    {
	switch (c) {
    	  // case 'c' (Disable SDL core dumps) is decoded in sdl.cpp:init()
	  // case 'm' (Set LOD bias) is decoded in various GUIs (grep getopt)
	  case 'h':
	      version_and_copyright();
          printf("\n"); 
	      usage ();
          	exit(EXIT_SUCCESS);
	  case 'v':
              dbglogfile.setVerbosity();
	      log_msg (_("Verbose output turned on"));
	      break;
	  case 'V':
        version_and_copyright();
        build_options();
	    	exit(EXIT_SUCCESS);	      
	  case 'w':
              dbglogfile.setWriteDisk(true);
	      log_msg (_("Logging to disk enabled"));
	      break;
	  case 'a':
#if VERBOSE_ACTION
	      dbglogfile.setActionDump(true); //gnash::set_verbose_action(true);
#else
              log_error (_("No verbose actions; disabled at compile time"));
#endif
	      break;
	  case 'p':
#if VERBOSE_PARSE
	      dbglogfile.setParserDump(true); // gnash::set_verbose_parse(true);
#else
              log_error (_("No verbose parsing; disabled at compile time"));
#endif
	      break;
          case 's':
              player.setScale( fclamp((float) atof(optarg), 0.01f, 100.f) );
              break;
          case 'd':
              player.setDelay( strtol(optarg, NULL, 0) );
              break;
          case 'u':
              url = optarg;
              log_msg (_("Setting root URL to %s"), url);
              break;
          case 'U':
	  {
		const char* baseurl = optarg;
		player.setBaseUrl(baseurl);
		log_msg (_("Setting base URL to %s"), baseurl);
		break;
	  }
          case 'j':
              width_given = true;
              player.setWidth ( strtol(optarg, NULL, 0) );
              log_msg (_("Setting width to " SIZET_FMT), player.getWidth());
              break;
          case 'g':
#ifdef USE_DEBUGGER
              log_msg (_("Setting debugger ON"));
              debugger.enabled(true);
//              debugger.startServer(&debugger);
              debugger.console();
#else
              log_error (_("No debugger; disabled at compile time"));
#endif
              break;
          case 'k':
              height_given = true;
              player.setHeight ( strtol(optarg, NULL, 0) );
              log_msg (_("Setting height to " SIZET_FMT), player.getHeight());
              break;
          case 'x':
              called_by_plugin=true;
              player.setWindowId(strtol(optarg, NULL, 0));
              break;
          case '1':
              player.setDoLoop(false);
              break;
          case 'r':
	{
              specified_rendering_flag=true;

              long int render_arg = strtol(optarg, NULL, 0);
              switch (render_arg) {
                case 0:
                    // Disable both
                    player.setDoRender(false);
                    player.setDoSound(false);
                    break;
                case 1:
                    // Enable rendering, disable sound
                    player.setDoRender(true);
                    player.setDoSound(false);
                    break;
                case 2:
                    // Enable sound, disable rendering
                    player.setDoRender(false);
                    player.setDoSound(true);
                    break;
                case 3:
                    // Enable render & sound
                    player.setDoRender(true);
                    player.setDoSound(true);
                    break;
                default:
                    log_error (_("-r must be followed by 0, 1, 2 or 3 "
			"(%ld is invalid)"), render_arg);
                    break;
              }
              break;
	}
          case 't':
              player.setExitTimeout( (float) atof(optarg) );
              break;
          case 'b':
          {
		int bit_depth;
              bit_depth = atoi(optarg);
		player.setBitDepth(bit_depth);
              break;
          }
          case 'P':
		string param = optarg;
		size_t eq = param.find("=");
		string name, value;
		if ( eq == string::npos ) {
			name = param;
			value = "true";
		} else {
			name = param.substr(0, eq);
			value = param.substr(eq+1);
		}
		//cerr << "Param name = "<<name<<" val="<<value<<std::endl;
		player.setParam(name, value);
		//params[name] = value;
		break;
	}
    }

    if ( ! specified_rendering_flag ) {
	log_msg (_("No rendering flags specified, using rcfile"));
        if ( called_by_plugin ) {
            player.setDoSound( rcfile.usePluginSound() );
        } else {
            player.setDoSound( rcfile.useSound() );
        }
    }

    if (called_by_plugin && height_given && width_given && !player.getHeight() && 
        !player.getWidth()) {
        // We were given dimensions of 0x0 to render to (probably the plugin
        // is playing an "invisible" movie. Disable video rendering.
        player.setDoRender(false);
    }

    // get the file name from the command line
    while (optind < argc) {

#if 0 // Options setting variables should use the getopt style!
      // Some options set variables, like ip=127.0.0.1
      if (argc > 2 && strchr(argv[optind], '=')) {
	  log_error (_("Got variable option (%s) on command line"),
			 argv[optind]);
      } else {
#endif
	  infile = argv[optind];
	  break;
#if 0 // see above
      }
#endif
      optind++;
    }
}


int
main(int argc, char *argv[])
{
	gnash::Player player;

	// Enable native language support, i.e. internationalization
	setlocale (LC_MESSAGES, "");
	bindtextdomain (PACKAGE, LOCALEDIR);
	textdomain (PACKAGE);

	rcfile.loadFiles();

	parseCommandLine(argc, argv, player);

	// No file name was supplied
	if (!infile) {
		std::cerr << "Error: no input file was specified."
			<< endl << endl;
		usage();
		return EXIT_FAILURE;
	}

	return player.run(argc, argv, infile, url);
}

