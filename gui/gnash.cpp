// gnash.cpp:  Main routine for top-level SWF player, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include "Player.h"
#include "log.h"
#include "rc.h" // for use of rcfile
#include "debugger.h"
#include "arg_parser.h"
#include "utility.h" // for clamp

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

#include <string>
#include <iostream>

#ifdef ENABLE_NLS
# include <locale>
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

#include <boost/format.hpp> // For i18n-friendly cerr

using namespace gnash; // for log_*

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

//extern bool g_debug;

static void
usage()
{

cout << _("Usage: gnash [options] movie_file.swf") << endl
    << endl
    << _("Plays a SWF (Shockwave Flash) movie") << endl
    << _("Options:") << endl
    << endl
    << _("  -h,  --help              Print this help and exit") << endl
    << _("  -V,  --version           Print version information and exit") << endl
    << _("  -s,  --scale <factor>    Scale the movie by the specified factor") << endl
    << _("  -c                       Produce a core file instead of letting SDL trap it") << endl
    << _("  -d,  --delay num         Number of milliseconds to delay in main loop") << endl
    << _("  -v,  --verbose           Produce verbose output") << endl
#if VERBOSE_ACTION
    << _("  -va                      Be (very) verbose about action execution") << endl
#endif
#if VERBOSE_PARSE
    << _("  -vp                      Be (very) verbose about parsing") << endl
#endif
    << _("  -A <file>                Audio dump file (wave format)") << endl
    << _("  -D <file>                Video dump file (only valid with dump-gnash)") << endl
    << _("  -x,  --xid <ID>          X11 Window ID for display") << endl
    << _("  -w,  --writelog          Produce the disk based debug log") << endl
    << _("  -j,  --width <width>     Set window width") << endl
    << _("  -k,  --height <height>   Set window height") << endl
    << _("  -1,  --once              Exit when/if movie reaches the last frame") << endl
    << _("  -g,  --debugger          Turn on the SWF debugger") << endl
    << _("  -r,  --render-mode <0|1|2|3>") << endl
    << _("                           0 disable rendering and sound") << endl
    << _("                           1 enable rendering, disable sound") << endl
    << _("                           2 enable sound, disable rendering") << endl
    << _("                           3 enable rendering and sound (default)") << endl
    << _("  -t,  --timeout <sec>     Exit after the specified number of seconds") << endl
    << _("  -u,  --real-url <url>    Set \"real\" URL of the movie") << endl
    << _("  -U,  --base-url <url>    Set \"base\" URL for resolving relative URLs") << endl
    << _("  -P,  --param <param>     Set parameter (e.g. \"FlashVars=A=1&b=2\")") << endl
    << _("  -F,  --fd <fd>           Filedescriptor to use for external communications") << endl
#ifdef GNASH_FPS_DEBUG
    << _("  -f,  --debug-fps num     Print FPS every num seconds (float).") << endl
#endif // def GNASH_FPS_DEBUG
    << endl
    << _("  --max-advances num       Exit after specified number of advances") << endl
    << _("  --fullscreen             Start in fullscreen mode") << endl
    << endl
    << _("Keys:") << endl
    << endl
    << _("  CTRL-Q, CTRL-W           Quit/Exit") << endl
    << _("  CTRL-P                   Toggle Pause") << endl
    << _("  CTRL-R                   Restart the movie") << endl

#if 0 // Currently disabled
    << _("  CTRL-[ or kp-   Step back one frame") << endl
    << _("  CTRL-] or kp+   Step forward one frame") << endl
#endif

    << _("  CTRL-L                   Force immediate redraw") << endl

//    << _("  CTRL-A          Toggle antialiasing (doesn't work)") << endl
//    << _("  CTRL-T          Debug.  Test the set_variable() function") << endl
//    << _("  CTRL-G          Debug.  Test the get_variable() function") << endl
//    << _("  CTRL-M          Debug.  Test the call_method() function") << endl

    << endl;
}

static void version_and_copyright()
{
    cout << "Gnash " << VERSION << endl
        << endl
        << _("Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.\n"
        "Gnash comes with NO WARRANTY, to the extent permitted by law.\n"
        "You may redistribute copies of Gnash under the terms of the GNU General\n"
        "Public License.  For more information, see the file named COPYING.\n")
    << endl;
}

static void build_options()
{
    cout << _("Build options ") << VERSION << endl
        << _("   Target: ") << TARGET_CONFIG << endl

        << _("   Renderer: ") << RENDERER_CONFIG
        << _(" - GUI: ") << GUI_CONFIG
        << _(" - Media handler: ") << MEDIA_CONFIG << endl
        
        << _("   Configured with: ") << CONFIG_CONFIG << endl
        << _("   CXXFLAGS: ") << CXXFLAGS << endl;

#ifdef HAVE_FFMPEG_AVCODEC_H
    cout << _("Built against ffmpeg version: ") << LIBAVCODEC_IDENT << endl;
#endif
#ifdef HAVE_GST_GST_H
    cout << _("Built against gstreamer version: ") << GST_VERSION_MAJOR << "."
        << GST_VERSION_MINOR << "." << GST_VERSION_MICRO << endl;
    guint major, minor, micro, nano;
    gst_version(&major, &minor, &micro, &nano);
    cout << _("Linked against gstreamer version: ") << major << "."
        << minor << "." << micro << endl;
#endif
}

static void
parseCommandLine(int argc, char* argv[], gnash::Player& player)
{

    const Arg_parser::Option opts[] =
        {
        { 'h', "help",          Arg_parser::no  },
        { 'v', "verbose",       Arg_parser::no  },
        { 'a', 0,               Arg_parser::no  },
        { 'p', 0,               Arg_parser::no  },
        { 's', "scale",         Arg_parser::yes },
        { 256, "max-advances",  Arg_parser::yes },
        { 257, "fullscreen",    Arg_parser::no  },        
        { 'c', 0,               Arg_parser::no  },
        { 'm', 0,               Arg_parser::yes }, // What is this for?
        { 'd', "delay",         Arg_parser::yes },
        { 'x', "xid",           Arg_parser::yes },
        { 'r', "render-mode",   Arg_parser::yes },
        { 't', "timeout",       Arg_parser::yes },        
        { '1', "once",          Arg_parser::no  },        
        { 'w', "writelog",      Arg_parser::no  },
        { 'j', "width",         Arg_parser::yes },
        { 'k', "height",        Arg_parser::yes },
        { 'u', "real-url",      Arg_parser::yes },
        { 'P', "param",         Arg_parser::yes },
        { 'U', "base-url",      Arg_parser::yes },  
        { 'g', "debugger",      Arg_parser::no  },
        { 'V', "version",       Arg_parser::no  },        
        { 'f', "debug-fps",     Arg_parser::yes },        
        { 'F', "fd",            Arg_parser::yes },
        { 'A', "dump",          Arg_parser::yes },
        { 'D', 0,               Arg_parser::yes }, // Handled in dump gui
        {   0, 0,               Arg_parser::no  }
    };

    Arg_parser parser(argc, argv, opts);
    if( ! parser.error().empty() )    
    {
        cout << parser.error() << endl;
        exit(EXIT_FAILURE);
    }


    bool renderflag = false;
    bool plugin = false;
    bool widthGiven = false, heightGiven = false;


    for( int i = 0; i < parser.arguments(); ++i )
    {
        const int code = parser.code(i);
        try
        {
            switch( code )
            {
                case 'h':
                    version_and_copyright();
                    usage ();
                    exit(EXIT_SUCCESS);
                case 'v':
                    dbglogfile.setVerbosity();
                    // This happens once per 'v' flag 
                    log_debug(_("Verbose output turned on"));
                    break;
                case 'V':
                    version_and_copyright();
                    build_options();
                    exit(EXIT_SUCCESS);          
                case 'w':
                    rcfile.useWriteLog(true); // dbglogfile.setWriteDisk(true);
                    log_debug(_("Logging to disk enabled"));
                    break;
                case 'a':
#if VERBOSE_ACTION
                    dbglogfile.setActionDump(true); //gnash::set_verbose_action(true);
#else
                    log_error(_("No verbose actions; disabled at compile time"));
#endif
                    break;
                case 'p':
#if VERBOSE_PARSE
                    dbglogfile.setParserDump(true); // gnash::set_verbose_parse(true);
#else
                    log_error (_("No verbose parsing; disabled at compile time"));
#endif
                    break;
                case 256:
                    player.setMaxAdvances( parser.argument<unsigned long>(i));
                    break;
                case 257:
                    player.setStartFullscreen(true);
                    break;                    
                case 's':
                    player.setScale(utility::clamp<float>(
                                    parser.argument<float>(i),
                                    0.01f, 100.f));
                    break;
                case 'd':
                    player.setDelay( parser.argument<long>(i) );
                    break;
                case 'u':
                    url = parser.argument(i);
                    log_debug (_("Setting root URL to %s"), url.c_str());
                    break;
                case 'U':    // Set base URL
                    player.setBaseUrl(parser.argument(i));
                    log_debug (_("Setting base URL to %s"), parser.argument(i));
                    break;
                case 'F':
                {
                    int fd = parser.argument<long>(i);
                    if ( fd < 1 )
                    {
                        cerr << boost::format(_("Invalid host communication filedescriptor %d\n")) % fd << endl;
                        exit(EXIT_FAILURE);
                    }
                    player.setHostFD ( fd );
                    break;
                }
                case 'j':
                    widthGiven = true;
                    player.setWidth ( parser.argument<long>(i));
                    log_debug (_("Setting width to %d"), player.getWidth());
                    break;
                case 'g':
#ifdef USE_DEBUGGER
                    log_debug (_("Setting debugger ON"));
                    debugger.enabled(true);
    //              debugger.startServer(&debugger);
                    debugger.console();
#else
                    log_error (_("No debugger; disabled at compile time, -g is invalid"));
                    exit(EXIT_FAILURE);
#endif
                    break;
                case 'k':
                    heightGiven = true;
                    player.setHeight ( parser.argument<long>(i));
                    log_debug (_("Setting height to %d"), player.getHeight());
                    break;
                case 'x':
                    plugin = true;
                    player.setWindowId(parser.argument<long>(i));
                    break;
                case '1':
                      player.setDoLoop(false);
                      break;
                case 'r':
                    renderflag = true;
                    switch (parser.argument<char>(i))
                    {
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
                        default:
                            log_error(_("ERROR: -r must be followed by 0, 1, 2 or 3 "));
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
                    cout << _("FPS debugging disabled at compile time, -f is invalid") << endl;
                    exit(EXIT_FAILURE);
#endif // ndef GNASH_FPS_DEBUG
                    break;
                case 'P':
                {
                    std::string param = parser.argument(i);
                    size_t eq = param.find("=");
                    std::string name, value;
                    if ( eq == std::string::npos )
                    {
                        name = param;
                        value = "true";
                    }
                    else
                    {
                        name = param.substr(0, eq);
                        value = param.substr(eq + 1);
                    }
                      player.setParam(name, value);
                    break;
                }
                case 'A':
                {
                    std::string fn = parser.argument(i);
                    player.setAudioDumpfile(fn);
                    break;
                }
                case 0:
                    infiles.push_back(parser.argument(i));
                    break;
            }
        }
        catch (Arg_parser::ArgParserException &e)
        {
            cerr << _("Error parsing command line options: ") << e.what() << endl;
            cerr << _("This is a Gnash bug.") << endl;
        }
    }

    if ( ! renderflag ) {
        log_debug (_("No rendering flags specified, using rcfile"));
        if ( plugin ) {
            player.setDoSound( rcfile.usePluginSound() );
        } else {
            player.setDoSound( rcfile.useSound() );
        }
    }

    if (plugin && heightGiven && widthGiven
    && !player.getHeight() && !player.getWidth()) {
            // We were given dimensions of 0x0 to render to (probably the plugin
            // is playing an "invisible" movie. Disable video rendering.
            player.setDoRender(false);
    }

}

int
gnash_main(int argc, char *argv[])
{
    gnash::Player player;

    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif
    //rcfile.loadFiles();

    try { parseCommandLine(argc, argv, player); }
    catch (const std::exception& ex)
    {
        cerr << ex.what() << endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        cerr << _("Exception thrown during parseCommandLine") << endl;
        return EXIT_FAILURE;
    }

    // No file name was supplied
    if (infiles.empty()) {
        cerr << _("Error: no input file was specified. Exiting.") << endl;
        usage();
        return EXIT_FAILURE;
    }

    return player.run(argc, argv, infiles.front().c_str(), url.c_str());
}
