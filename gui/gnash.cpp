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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Player.h"
#include "log.h" // for dbglogfile (I hate this)
#include "rc.h" // for use of rcfile 

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


#include <iostream>

//using namespace gnash; // for dbglogfile

using namespace std;

char* infile = NULL;
char* url = NULL;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

static void
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
#if VERBOSE_ACTION
        "  -va         Be verbose about movie Actions\n"
#endif
#if VERBOSE_PARSE
        "  -vp         Be verbose about parsing the movie\n"
#endif
        "  -m <bias>   Specify the texture LOD bias (float, default is -1.0)\n"
#if 0
        "  -f          Run full speed (no sleep) and log frame rate\n"
        "  -e          Use SDL Event thread\n"
#endif
        "  -x <ID>     X11 Window ID for display\n"
        "  -w          Produce the disk based debug log\n"
	"  -j <width>  Set window width\n"
	"  -k <height> Set window height\n"
        "  -1          Play once; exit when/if movie reaches the last frame\n"
        "  -r <0|1|2|3>\n"
	"              0 disables both rendering & sound (good for batch tests)\n"
        "              1 enables rendering & disables sound\n"
        "              2 enables sound & disables rendering\n"
        "              3 enables both rendering & sound (default)\n"
        "  -t <sec>    Timeout and exit after the specified number of seconds\n"
        "  -b <bits>   Bit depth of output window (16 or 32, default is 16)\n"
        "  -u <url>    Set \"real\" url of the movie\n"
	"              (useful for downloaded movies)\n"
        "  -U <url>    Set \"base\" url for this run\n"
	"              (used to resolve relative urls, defaults to movie url)\n"
        "  -P <param>  Set parameter (ie. \"FlashVars=A=1&b=2\")\n"
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

static void version_and_copyright()
{
    printf (
"Gnash " VERSION "\n"
"Copyright (C) 2005-2007 Free Software Foundation, Inc.\n"
"Gnash comes with NO WARRANTY, to the extent permitted by law.\n"
"You may redistribute copies of Gnash under the terms of the GNU General\n"
"Public License.  For more information, see the file named COPYING.\n"
	);
}

static void build_options()
{
    cout << "Build options " << VERSION << endl
         << "   Target: " << TARGET_CONFIG << endl
         << "   Renderer: " << RENDERER_CONFIG
         << "   GUI: " << GUI_CONFIG
         << "   Sound handler: " << SOUND_CONFIG
         << "   Decoder: " << DECODER_CONFIG
         << endl;
}


static void
parseCommandLine(int argc, char* argv[], gnash::Player& player)
{
    bool specified_rendering_flag=false;
    bool called_by_plugin=false;

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
    
    while ((c = getopt (argc, argv, "hvaps:cfd:x:r:t:b:1ewj:k:u:P:U:")) != -1)
    {
	switch (c) {
	  case 'h':
	      usage ();
          exit(0);
	  case 'v':
              dbglogfile.setVerbosity();
	      dbglogfile << "Verbose output turned on" << std::endl;
	      break;
	  case 'w':
              dbglogfile.setWriteDisk(true);
	      dbglogfile << "Logging to disk enabled." << std::endl;
	      break;
	  case 'a':
#if VERBOSE_ACTION
	      dbglogfile.setActionDump(true); //gnash::set_verbose_action(true);
#else
              dbglogfile << "Verbose actions disabled at compile time" << std::endl;
#endif
	      break;
	  case 'p':
#if VERBOSE_PARSE
	      dbglogfile.setParserDump(true); // gnash::set_verbose_parse(true);
#else
              dbglogfile << "Verbose parsing disabled at compile time" << std::endl;
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
              dbglogfile << "Setting root URL to: " << url << std::endl;
              break;
          case 'U':
	  {
		const char* baseurl = optarg;
		player.setBaseUrl(baseurl);
		dbglogfile << "Setting base URL to: " << baseurl << std::endl;
		break;
	  }
          case 'j':
              player.setWidth ( strtol(optarg, NULL, 0) );
              dbglogfile << "Setting width to: " << player.getWidth() << std::endl;
              break;
          case 'k':
              player.setHeight ( strtol(optarg, NULL, 0) );
              dbglogfile << "Setting height to: " << player.getHeight() << std::endl;
              break;
#if 0
          case 'e':
              s_event_thread = true;
              break;
#endif
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
                    cerr << "-r must be followed by 0, 1, 2 or 3 (" << 
                        render_arg << ") is invalid" << std::endl;
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
	gnash::log_msg("no rendering flags specified, using rcfile");
        if ( called_by_plugin ) {
            player.setDoSound( rcfile.usePluginSound() );
        } else {
            player.setDoSound( rcfile.useSound() );
        }
    }

    // get the file name from the command line
    while (optind < argc) {

#if 0 // Options setting variables should use the getopt style!
      // Some options set variables, like ip=127.0.0.1
      if (argc > 2 && strchr(argv[optind], '=')) {
	  dbglogfile << "Got variable option on command line!" << std::endl;
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

