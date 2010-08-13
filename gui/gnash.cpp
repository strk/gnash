// gnash.cpp:  Main routine for top-level SWF player, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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
#include "gnashconfig.h"
#endif

#include <string>
#include <iostream>
#include <iterator>
#include <ios>
#include <boost/format.hpp>
#include <boost/algorithm/string/join.hpp>
#include <cstdlib>
#include <sys/types.h>
#include <fcntl.h>
#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "Player.h"
#include "log.h"
#include "rc.h" // for use of rcfile
#include "debugger.h"
#include "arg_parser.h"
#include "GnashNumeric.h" // for clamp
#include "GnashException.h"
#include "revno.h"
#include "MediaHandler.h"

#ifdef HAVE_FFMPEG_AVCODEC_H
extern "C" {
# include "ffmpeg/avcodec.h"
}
#endif

#ifdef HAVE_LIBAVCODEC_AVCODEC_H
extern "C" {
# include "libavcodec/avcodec.h"
}
#endif

#ifdef HAVE_GST_GST_H
# include "gst/gst.h"
# include "gst/gstversion.h"
#endif

#ifdef GUI_ALP
#include <alp/title.h>
#include <alp/menubar.h>
#include <alp/bundlemgr.h>
#include <alp/appmgr.h>
# define gnash_main alp_main
#else
# define gnash_main main
#endif

using std::cerr;
using std::endl;
using std::cout;

std::vector<std::string> infiles;
std::string url;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
#ifdef USE_DEBUGGER
gnash::Debugger& debugger = gnash::Debugger::getDefaultInstance();
#endif
}

static void
usage()
{
    std::vector<std::string> handlers;
    gnash::media::MediaFactory::instance().listKeys(back_inserter(handlers));

    cout << _("Usage: gnash [options] movie_file.swf\n")
    << "\n"
    << _("Plays a SWF (Shockwave Flash) movie\n")
    << _("Options:\n")
    << "\n"
    << _("  -h,  --help              Print this help and exit\n")
    << _("  -V,  --version           Print version information and exit\n") 
    << _("  -s,  --scale <factor>    Scale the movie by the specified factor\n") 
    << _("  -c                       Produce a core file instead of letting SDL trap it\n") 
    << _("  -d,  --delay num         Number of milliseconds to delay in main loop\n") 
    << _("  -v,  --verbose           Produce verbose output\n") 
#if VERBOSE_ACTION
    << _("  -va                      Be (very) verbose about action execution\n") 
#endif
#if VERBOSE_PARSE
    << _("  -vp                      Be (very) verbose about parsing\n") 
#endif
    << _("  -A <file>                Audio dump file (wave format)\n") 
    << _("  --hwaccel <none|vaapi|xv> Hardware Video Accelerator to use\n") 
    << _("                           none|vaapi|xv|omap (default: none)\n") 
    << _("  -x,  --xid <ID>          X11 Window ID for display\n") 
    << _("  -w,  --writelog          Produce the disk based debug log\n") 
    << _("  -j,  --width <width>     Set window width\n") 
    << _("  -k,  --height <height>   Set window height\n") 
    << _("  -X,  --x-pos <x-pos>     Set window x position\n") 
    << _("  -Y,  --y-pos <y-pos>     Set window y position\n") 
    << _("  -1,  --once              Exit when/if movie reaches the last "
            "frame\n") 
    << _("  -g,  --debugger          Turn on the SWF debugger\n") 
    << _("  -r,  --render-mode <0|1|2|3>\n") 
    << _("                           0 disable rendering and sound\n") 
    << _("                           1 enable rendering, disable sound\n") 
    << _("                           2 enable sound, disable rendering\n") 
    << _("                           3 enable rendering and sound (default)\n") 
    << _("  -M,  --media <") << boost::join(handlers, "|") << ">\n"
    << _("                           The media handler to use")
    << " (default " << handlers.front() << ")\n"
    // Only list the renderers that were configured in for this build
    << _("  -R,  --renderer <")
#ifdef RENDERER_OPENGL
     << _(" opengl")
#endif
#ifdef RENDERER_CAIRO
     << _(" cairo")
#endif
#ifdef RENDERER_AGG
    << _(" agg > (default: agg)\n")
#else
    << " >\n"
#endif
    << _("  -t,  --timeout <sec>     Exit after the specified number of "
            "seconds\n") 
    << _("  -u,  --real-url <url>    Set \"real\" URL of the movie\n") 
    << _("  -U,  --base-url <url>    Set \"base\" URL for resolving relative "
            "URLs\n") 
    << _("  -P,  --param <param>     Set parameter (e.g. "
            "\"FlashVars=A=1&b=2\")\n") 
    << _("  -F,  --fd <fd>:<fd>      Filedescriptor to use for external "
            "communications\n") 
#ifdef GNASH_FPS_DEBUG
    << _("  -f,  --debug-fps num     Print FPS every num seconds (float)\n") 
#endif // def GNASH_FPS_DEBUG
    
    << _("  --max-advances num       Exit after specified number of frame "
            "advances\n") 
    << _("  --fullscreen             Start in fullscreen mode\n") 
    << _("  --hide-menubar           Start without displaying the menu bar\n") 
    << _("  --screenshot <list>      List of frames to save as screenshots\n") 
    << _("  --screenshot-file <file> Filename pattern for screenshot images.\n")
    << "\n"
    << _("Keys:\n") 
    << "\n"
    << _("  CTRL-Q, CTRL-W           Quit\n") 
    << _("  CTRL-F                   Toggle fullscreen\n") 
    << _("  CTRL-P                   Toggle pause\n") 
    << _("  CTRL-R                   Restart the movie\n") 
    << _("  CTRL-O                   Take a screenshot\n") 
    << _("  CTRL-L                   Force immediate redraw\n") 
    << endl;
}

static void
version_and_copyright()
{
    cout << "Gnash " << VERSION << endl
        << endl
        << _("Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 "
                "Free Software Foundation, Inc.\n"
                "Gnash comes with NO WARRANTY, to the extent permitted "
                "by law.\nYou may redistribute copies of Gnash under the "
                "terms of the GNU General\nPublic License.  For more "
                "information, see the file named COPYING.\n") << endl;
}

static void
build_options()
{
    cout << _("Build options ") << VERSION << endl
	 << _("   Renderers: ") << RENDERER_CONFIG << endl
	 << _("   Hardware Acceleration: ") << HWACCEL_CONFIG << endl
	 << _("   GUI: ") << GUI_CONFIG << endl
	 << _("   Media handlers: ") << MEDIA_CONFIG << endl
        
	 << _("   Configured with: ") << CONFIG_CONFIG << endl
	 << _("   CXXFLAGS: ") << CXXFLAGS << endl
	 << "   Version: "  << BRANCH_NICK << ":" << BRANCH_REVNO << endl;
}

static void
parseCommandLine(int argc, char* argv[], gnash::Player& player)
{

    const Arg_parser::Option opts[] =
        {
        { 'h', "help",              Arg_parser::no  },
        { 'v', "verbose",           Arg_parser::no  },
        { 'a', 0,                   Arg_parser::no  },
        { 'p', 0,                   Arg_parser::no  },
        { 's', "scale",             Arg_parser::yes },
        { 256, "max-advances",      Arg_parser::yes },
        { 257, "fullscreen",        Arg_parser::no  },
        { 258, "hide-menubar",      Arg_parser::no  },                
        { 'c', 0,                   Arg_parser::no  },
        { 'd', "delay",             Arg_parser::yes },
        { 'x', "xid",               Arg_parser::yes },
        { 'R', "renderer",          Arg_parser::yes },
        { 'M', "media",             Arg_parser::yes },
        { 'r', "render-mode",       Arg_parser::yes },
        { 't', "timeout",           Arg_parser::yes },        
        { '1', "once",              Arg_parser::no  },        
        { 'w', "writelog",          Arg_parser::no  },
        { 'j', "width",             Arg_parser::yes },
        { 'k', "height",            Arg_parser::yes },
        { 'X', "x-position",        Arg_parser::yes },
        { 'Y', "y-position",        Arg_parser::yes },
        { 'u', "real-url",          Arg_parser::yes },
        { 'P', "param",             Arg_parser::yes },
        { 'U', "base-url",          Arg_parser::yes },  
        { 'g', "debugger",          Arg_parser::no  },
        { 'V', "version",           Arg_parser::no  },        
        { 'f', "debug-fps",         Arg_parser::yes },        
        { 'F', "fifo",              Arg_parser::yes },
        { 'A', "dump",              Arg_parser::yes },
        { 259, "screenshot",        Arg_parser::yes },
        { 260, "screenshot-file",   Arg_parser::yes },
        { 261, "hwaccel",           Arg_parser::yes },
        { 262, "flash-version",     Arg_parser::no },
        { 'D', 0,                   Arg_parser::yes }, // Handled in dump gui
        {   0, 0,                   Arg_parser::no  }
    };

    Arg_parser parser(argc, argv, opts);
    if (!parser.error().empty()) {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }

    bool renderflag = false;
    bool plugin = false;
    bool widthGiven = false, heightGiven = false;
    bool xPosGiven = false, yPosGiven = false;

    for (int i = 0; i < parser.arguments(); ++i) {
        const int code = parser.code(i);
        try {
            switch (code) {
                case 'h':
                    version_and_copyright();
                    usage ();
                    exit(EXIT_SUCCESS);
                case 'v':
                    dbglogfile.setVerbosity();
                    // This happens once per 'v' flag 
                    gnash::log_debug(_("Verbose output turned on"));
                    break;
                case 'V':
                    version_and_copyright();
                    build_options();
                    exit(EXIT_SUCCESS);          
                case 'w':
                    rcfile.useWriteLog(true); 
                    gnash::log_debug(_("Logging to disk enabled"));
                    break;
                case 'a':
#if VERBOSE_ACTION
                    dbglogfile.setActionDump(true); 
#else
                    gnash::log_error(_("No verbose actions; disabled at "
                                "compile time"));
#endif
                    break;
                case 'p':
#if VERBOSE_PARSE
                    dbglogfile.setParserDump(true); 
#else
                    gnash::log_error (_("No verbose parsing; disabled at "
                                "compile time"));
#endif
                    break;
                case 256:
                    player.setMaxAdvances(parser.argument<unsigned long>(i));
                    break;
                case 257:
                    player.setStartFullscreen(true);
                    break;
                case 258:
                    player.hideMenu(true);
                    break;
                case 's':
                    player.setScale(gnash::clamp<float>(
                                    parser.argument<float>(i), 0.01f, 100.f));
                    break;
                case 'd':
                    player.setDelay(parser.argument<long>(i));
                    break;
                case 'u':
                    url = parser.argument(i);
                    gnash::log_debug (_("Setting root URL to %s"), url.c_str());
                    break;
                case 'U':    
                    // Set base URL
                    player.setBaseUrl(parser.argument(i));
                    gnash::log_debug (_("Setting base URL to %s"),
                                      parser.argument(i));
                    break;
                case 'F':
                {
		    const std::string& fds = parser.argument(i);
                    fds.find(":");
                    int hostfd = 0, controlfd = 0;
                    hostfd = strtol(fds.substr(0, fds.find(":")).c_str(), NULL, 0);
                    std::string csub = fds.substr(fds.find(":")+1, fds.size());
                    controlfd = strtol(csub.c_str(), 0, 0);
                    gnash::log_debug(_("Host FD #%d, Control FD #%d\n"), 
                              hostfd, controlfd);
                    if (hostfd < 0) {
                        cerr << boost::format(_("Invalid host communication "
						"filedescriptor %d\n"))
                            % hostfd << endl;
                        exit(EXIT_FAILURE);
                    }
                    player.setHostFD (hostfd);

                    if (controlfd < 0) {
                        cerr << boost::format(_("Invalid control communication "
                                    "filedescriptor %d\n")) % controlfd << endl;
                        exit(EXIT_FAILURE);
                    }
                    player.setControlFD (controlfd);
                }
                break;
                case 'j':
                    widthGiven = true;
                    player.setWidth(parser.argument<long>(i));
                    gnash::log_debug(_("Setting width to %d"),
                             player.getWidth());
                    break;
                case 'g':
#ifdef USE_DEBUGGER
                    gnash::log_debug(_("Setting debugger ON"));
                    debugger.enabled(true);
                    //              debugger.startServer(&debugger);
                    debugger.console();
#else
                    gnash::log_error(_("No debugger; disabled at compile "
                                "time, -g is invalid"));
                    exit(EXIT_FAILURE);
#endif
                    break;
                case 'k':
                    heightGiven = true;
                    player.setHeight(parser.argument<long>(i));
                    gnash::log_debug(_("Setting height to %d"),
                             player.getHeight());
                    break;
                case 'X':
                    xPosGiven = true;
                    player.setXPosition ( parser.argument<int>(i));
                    gnash::log_debug (_("Setting x position to %d"), 
                              player.getXPosition());
                    break;
                case 'Y':
                    yPosGiven = true;
                    player.setYPosition(parser.argument<int>(i));
                    gnash::log_debug(_("Setting x position to %d"), 
                              player.getYPosition());
                    break;
                case 'x':
                    plugin = true;
                    player.setWindowId(parser.argument<long>(i));
                    break;
                case '1':
                    player.setDoLoop(false);
                    break;
                    // See if the hardware video decoder was specified
                 case 261:
                    switch (parser.argument<char>(i)) {
                        case 'v':
                            player.setHWAccel("vaapi");
                            break;
                        case 'x':
                            player.setHWAccel("xv");
                            break;
                        case 'n':
                        default:
                            player.setHWAccel("none");
                            break;
                        }
                    break; 
                case 262:
                    cout << rcfile.getFlashVersionString() << endl;
                    exit(EXIT_SUCCESS);          
                    break;
              case 'M':
                    player.setMedia(parser.argument(i));
                    break;
              case 'R':
                    player.setRenderer(parser.argument(i));
                    break;
              case 'r':
                    renderflag = true;
                    switch (parser.argument<char>(i)) {
                        case '0':
                            // Disable both
                            player.setDoRender(false);
                            player.setDoSound(false);
                            break;
                        case '1':
                            // Enable rendering, disable sound
                            player.setDoRender(true);
                            player.setDoSound(false);
                            break;
                        case '2':
                            // Enable sound, disable rendering
                            player.setDoRender(false);
                            player.setDoSound(true);
                            break;
                        case '3':
                            // Enable render & sound
                            player.setDoRender(true);
                            player.setDoSound(true);
                            break;
                            // See if a renderer was specified
                        case 'a':
                            // Enable AGG as the rendering backend
                            player.setRenderer("agg");
                            break;
                        case 'o':
                            // Enable OpenGL as the rendering backend
                            player.setRenderer("opengl");
                            break;
                        case 'c':
                            // Enable Cairo as the rendering backend
                            player.setRenderer("cairo");
                            break;
                        default:
                            gnash::log_error(_("ERROR: -r must be followed by "
                                               "0, 1, 2 or 3 "));
                            break;
                    }
                break;
            case 't':
                player.setExitTimeout(parser.argument<float>(i));
                break;
            case 'f':
#ifdef GNASH_FPS_DEBUG
                player.setFpsPrintTime(parser.argument<float>(i));
#else
                cout << _("FPS debugging disabled at compile time, -f "
                          "is invalid") << endl;
                exit(EXIT_FAILURE);
#endif 
                break;
            case 'P':
            {
                const std::string& param = parser.argument(i);
                const size_t eq = param.find("=");
                std::string name, value;
                if (eq == std::string::npos) {
                    name = param;
                    value = "true";
                } else {
                    name = param.substr(0, eq);
                    value = param.substr(eq + 1);
                }
                player.setParam(name, value);
                break;
            }
            case 'A':
            {
                player.setAudioDumpfile(parser.argument(i));
                break;
            }
            case 259:
                // The player takes care of parsing the list.
                player.setScreenShots(parser.argument(i));
                break;
            case 260:
                player.setScreenShotFile(parser.argument(i));
                break;
            case 0:
                infiles.push_back(parser.argument(i));
                break;
            }
        }
        catch (Arg_parser::ArgParserException &e) {
            cerr << _("Error parsing command line options: ") << e.what() 
                << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

    if (!renderflag) {
        gnash::log_debug (_("No rendering flags specified, using rcfile"));
        if (plugin) {
            player.setDoSound(rcfile.usePluginSound());
        }
        else {
            player.setDoSound(rcfile.useSound());
        }
    }

    if (plugin && heightGiven && widthGiven && !player.getHeight() &&
            !player.getWidth()) {
            // We were given dimensions of 0x0 to render to (probably the plugin
            // is playing an "invisible" movie. Disable video rendering.
            player.setDoRender(false);
    }

}

int
gnash_main(int argc, char *argv[])
{
    
    std::ios::sync_with_stdio(false);

    gnash::Player player;

    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif

    try { 
        parseCommandLine(argc, argv, player);
    }
    catch (const std::exception& ex) {
        cerr << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...) {
        cerr << _("Exception thrown during parseCommandLine") << endl;
        return EXIT_FAILURE;
    }

    // No file name was supplied
    if (infiles.empty()) {
        cerr << _("Error: no input file was specified. Exiting.") << endl;
        usage();
        return EXIT_FAILURE;
    }

    // We only expect GnashExceptions here. No others should be thrown!
    try {
        player.run(argc, argv, infiles.front(), url);
    }
    catch (const gnash::GnashException& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
