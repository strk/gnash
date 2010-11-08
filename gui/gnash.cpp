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
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <cstdlib>
#include <sys/types.h>
#include <fcntl.h>
#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "Player.h"
#include "log.h"
#include "rc.h" // for use of rcfile
#include "arg_parser.h"
#include "GnashNumeric.h" // for clamp
#include "GnashException.h"
#include "revno.h"
#include "MediaHandler.h"

using std::cerr;
using std::endl;
using std::cout;

std::vector<std::string> infiles;
std::string url;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

static void
usage_gui_keys(std::ostream& os)
{
    os
    << _("Keys:") << std::endl
    << std::endl
    << "  CTRL-Q, CTRL-W           "
    << _("Quit")
    << std::endl
    << "  CTRL-F                   "
    << _("Toggle fullscreen")
    << std::endl 
    << "  CTRL-P                   "
    << _("Toggle pause") 
    << std::endl 
    << "  CTRL-R                   "
    << _("Restart the movie") 
    << std::endl 
    << "  CTRL-O                   "
    << _("Take a screenshot") 
    << std::endl 
    << "  CTRL-L                   "
    << _("Force immediate redraw") 
    << std::endl;
}

static void
usage()
{
    std::vector<std::string> handlers;
    gnash::media::MediaFactory::instance().listKeys(back_inserter(handlers));

    std::vector<std::string> renderers;
    boost::split(renderers, RENDERER_CONFIG,
        boost::is_any_of(" "), boost::token_compress_on);

    cout

    << _("Usage: gnash [options] movie_file.swf") << endl
    << _("Plays a SWF (Shockwave Flash) movie") << endl
    << _("Options:") << endl
    << endl 

    <<   "  -h,  --help              "
    << _("Print this help and exit") << endl

    <<   "  -V,  --version           "
    << _("Print version information and exit") << endl

    <<   "  -s,  --scale <factor>    "
    << _("Scale the movie by the specified factor") << endl

    <<   "  -d,  --delay <num>       "
    << _("Number of milliseconds to delay in main loop") << endl

    <<   "  -v,  --verbose           "
    << _("Produce verbose output") << endl

#if VERBOSE_ACTION
    <<   "  -va                      "
    << _("Be (very) verbose about action execution") << endl
#endif

#if VERBOSE_PARSE
    <<   "  -vp                      "
    << _("Be (very) verbose about parsing") << endl
#endif

    <<   "  -A <file>                "
    << _("Audio dump file (wave format)") << endl

    <<   "  --hwaccel <none|vaapi>   "
    << _("Hardware Video Accelerator to use") << endl
    <<   "                           none|vaapi|omap (default: none)" 
    << endl

    <<   "  -x,  --xid <ID>          "
    << _("X11 Window ID for display") << endl

    <<   "  -w,  --writelog          "
    << _("Produce the disk based debug log") << endl

    <<   "  -j,  --width <width>     "
    << _("Set window width") << endl

    <<   "  -k,  --height <height>   "
    << _("Set window height") << endl

    <<   "  -X,  --x-pos <x-pos>     "
    << _("Set window x position") << endl

    <<   "  -Y,  --y-pos <y-pos>     "
    << _("Set window y position") << endl

    <<   "  -1,  --once              "
    << _("Exit when/if movie reaches the last frame") << endl

    <<   "  -r,  --render-mode <0|1|2|3>" << endl 
    <<   "                           0 "
    << _("disable rendering and sound") << endl
    <<   "                           1 "
    << _("enable rendering, disable sound") << endl
    <<   "                           2 "
    << _("enable sound, disable rendering") << endl
    <<   "                           3 "
    << _("enable rendering and sound (default)") << endl

    <<   "  -M,  --media <" << boost::join(handlers, "|") << ">" << endl
    <<   "                           "
    << _("The media handler to use")
    << " (default: " << handlers.front() << ")" << endl

    <<   "  -R,  --renderer <" << boost::join(renderers, "|") << ">" << endl
    <<   "                           "
    << _("The renderer to use")
    << " (default: agg)" << endl

    <<   "  -t,  --timeout <sec>     "
    << _("Exit after the specified number of seconds") << endl

    <<   "  -u,  --real-url <url>    "
    << _("Set \"real\" URL of the movie")  << endl

    <<   "  -U,  --base-url <url>    "
    << _("Set \"base\" URL for resolving relative URLs") << endl

    <<   "  -P,  --param <param>     "
    << _("Set parameter (e.g. \"FlashVars=A=1&b=2\")") << endl

    <<   "  -F,  --fd <fd>:<fd>      "
    << _("Filedescriptor to use for external communications") << endl

#ifdef GNASH_FPS_DEBUG
    <<   "  -f,  --debug-fps <num>   "
    << _("Print FPS every num seconds (float)") << endl
#endif // def GNASH_FPS_DEBUG
    
    <<   "  --max-advances <num>     "
    << _("Exit after specified number of frame advances") << endl

    <<   "  --fullscreen             "
    << _("Start in fullscreen mode") << endl

    <<   "  --hide-menubar           "
    << _("Start without displaying the menu bar") << endl

    <<   "  --screenshot <list>      "
    << _("List of frames to save as screenshots") << endl

    <<   "  --screenshot-file <file> "
    << _("Filename pattern for screenshot images.") << endl

    << endl;

    // Add gui keys
    usage_gui_keys(cout);

    cout << std::endl;

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
	 << _("   Version: ")  << BRANCH_NICK << ":" << BRANCH_REVNO << endl;
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
        { 'V', "version",           Arg_parser::no  },        
        { 'f', "debug-fps",         Arg_parser::yes },        
        { 'F', "fifo",              Arg_parser::yes },
        { 'A', "dump",              Arg_parser::yes },
        { 259, "screenshot",        Arg_parser::yes },
        { 260, "screenshot-file",   Arg_parser::yes },
        { 261, "hwaccel",           Arg_parser::yes },
        { 262, "flash-version",     Arg_parser::no },
        { 'D', 0,                   Arg_parser::yes }, // Handled in dump gui
        { 'S', 0,                   Arg_parser::yes }, // Handled in dump gui
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
main(int argc, char *argv[])
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
