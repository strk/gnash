// gnash.cpp:  Main routine for top-level SWF player, for Gnash.
// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
//   2011 Free Software Foundation, Inc
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
#include <boost/program_options.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <functional>
#ifdef ENABLE_NLS
# include <clocale>
#endif

#include "Player.h"
#include "log.h"
#include "rc.h" // for use of rcfile
#include "GnashNumeric.h" // for clamp
#include "GnashException.h"
#include "revno.h"
#include "MediaHandler.h"
#include "utility.h"
#include "accumulator.h"

using std::endl;
using std::cout;

std::vector<std::string> infiles;
std::string url;

namespace gnash {
    class Player;
}

namespace {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();
}

// Forward declarations
namespace {
    namespace po = boost::program_options;
    po::options_description getSupportedOptions(gnash::Player& p);

    void setupSoundAndRendering(gnash::Player& p, int i);
    void setupFlashVars(gnash::Player& p,
        const std::vector<std::string>& params);
    void setupFDs(gnash::Player& p, const std::string& fds);

    void usage_gui_keys(std::ostream& os);
    void usage(std::ostream& os, const po::options_description& opts);
    void build_options(std::ostream& os);
    void version_and_copyright(std::ostream& os);
}

void
playFile(gnash::Player& player, int argc, char *argv[],
                              const std::string& filename)
{
    gnash::Player newPlayer(player);
    newPlayer.run(argc, argv, filename, url);
}

int
main(int argc, char *argv[])
{
    
    std::ios::sync_with_stdio(false);

    // Enable native language support, i.e. internationalization
#ifdef ENABLE_NLS
    std::setlocale (LC_ALL, "");
    bindtextdomain (PACKAGE, LOCALEDIR);
    textdomain (PACKAGE);
#endif

    gnash::Player player;

    po::options_description opts = getSupportedOptions(player);

    // Add all positional arguments as input files.
    po::positional_options_description files;
    files.add("input-file", -1);

    namespace cls = po::command_line_style;

    po::variables_map vm;
    try {
        po::store(po::command_line_parser(argc, argv)
                .options(opts)
                .positional(files)
                .style(cls::default_style ^ cls::allow_guessing)
                .run(), vm);
    }
    catch (const po::error& e) {
        std::cerr << boost::format(_("Error parsing options: %s\n"))
            % e.what();
        return EXIT_FAILURE;
    }

    po::notify(vm);

    if (vm.count("help")) {
        version_and_copyright(std::cout);
        usage(std::cout, opts);
        return EXIT_SUCCESS;
    }

    if (vm.count("version")) {
        version_and_copyright(std::cout);
        build_options(std::cout);
        return EXIT_SUCCESS;
    }

    // Do some extra sanity checks on the options.
    const bool plugin = vm.count("xid");

    if (plugin && vm.count("height") && vm.count("width") &&
            !player.getHeight() && !player.getWidth()) {
            // We were given dimensions of 0x0 to render to (probably the plugin
            // is playing an "invisible" movie. Disable video rendering.
            player.setDoRender(false);
    }

    if (!vm.count("render-mode")) {
        std::cerr << "Using rcfile\n";
        if (plugin) {
            player.setDoSound(rcfile.usePluginSound());
        }
        else {
            player.setDoSound(rcfile.useSound());
        }
    }

    // No file name was supplied
    if (infiles.empty()) {
        std::cerr << _("Error: no input file was specified. Exiting.\n");
        usage(std::cerr, opts);
        return EXIT_FAILURE;
    }

    // We only expect GnashExceptions here. No others should be thrown!
    try {
        std::for_each(infiles.begin(), infiles.end(),
                boost::bind(&playFile, boost::ref(player), argc, argv, _1));
    }
    catch (const gnash::GnashException& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

namespace {

void
setupFlashVars(gnash::Player& p, const std::vector<std::string>& params)
{
    for (std::vector<std::string>::const_iterator i = params.begin(), 
        e = params.end(); i != e; ++i) {
        const std::string& param = *i;
        const size_t eq = param.find("=");
        if (eq == std::string::npos) {
            p.setParam(param, "true");
            return;
        }
        const std::string name = param.substr(0, eq);
        const std::string value = param.substr(eq + 1);
        p.setParam(name, value);
    }
}

void
setupFDs(gnash::Player& p, const std::string& fds)
{
    int hostfd = 0, controlfd = 0;
    hostfd = std::strtol(fds.substr(0, fds.find(":")).c_str(), NULL, 0);
    std::string csub = fds.substr(fds.find(":")+1, fds.size());
    controlfd = strtol(csub.c_str(), 0, 0);
    // gnash::log_debug("Host FD #%d, Control FD #%d\n", hostfd, controlfd);

    if (hostfd < 0) {
        std::cerr << boost::format(_("Invalid host communication "
                    "filedescriptor %1%\n")) % hostfd;
        std::exit(EXIT_FAILURE);
    }
    p.setHostFD(hostfd);

    if (controlfd < 0) {
        std::cerr << boost::format(_("Invalid control communication "
                    "filedescriptor %1%\n")) % controlfd;
        std::exit(EXIT_FAILURE);
    }
    p.setControlFD(controlfd);
}

void
setupSoundAndRendering(gnash::Player& p, int i)
{
    switch (i) {
        case 0:
            // Disable both
            p.setDoRender(false);
            p.setDoSound(false);
            return;
        case 1:
            // Enable rendering, disable sound
            p.setDoRender(true);
            p.setDoSound(false);
            return;
        case 2:
            // Enable sound, disable rendering
            p.setDoRender(false);
            p.setDoSound(true);
            return;
        case 3:
            // Enable render & sound
            p.setDoRender(true);
            p.setDoSound(true);
            return;
        default:
            gnash::log_error(_("ERROR: -r must be followed by "
                               "0, 1, 2 or 3 "));
    }
}

po::options_description
getDebuggingOptions(gnash::Player& p)
{
#ifndef GNASH_FPS_DEBUG
    UNUSED(p);
#endif

    using gnash::Player;
    using gnash::LogFile;
    using gnash::RcInitFile;

    po::options_description desc(_("Debugging options"));

    desc.add_options()

    ("verbose,v", accumulator<int>()
        ->notifier(boost::bind(&LogFile::setVerbosity, &dbglogfile, _1)),
        _("Produce verbose output"))

    // NB: if we use a bool_switch(), the default will be false. TODO:
    // make a sensible process for handling command-line and rcfile options.
    ("writelog,w", po::value<bool>()
        ->zero_tokens()
        ->notifier(boost::bind(&RcInitFile::useWriteLog, &rcfile, _1)),
        _("Produce the disk based debug log"))

#if VERBOSE_ACTION
    ("verbose-actions,a", po::bool_switch()
        ->notifier(boost::bind(&LogFile::setActionDump, &dbglogfile, _1)),
        _("Be (very) verbose about action execution"))
#endif

#if VERBOSE_PARSE
    ("verbose-parsing,p", po::bool_switch()
        ->notifier(boost::bind(&LogFile::setParserDump, &dbglogfile, _1)),
        _("Be (very) verbose about parsing"))
#endif

#ifdef GNASH_FPS_DEBUG
    ("debug-fps,f", po::value<float>()
        ->notifier(boost::bind(&Player::setFpsPrintTime, &p, _1)),
        _("Print FPS every num seconds"))
#endif 

    ;

    return desc;
}

po::options_description
getSupportedOptions(gnash::Player& p)
{
    using std::string;
    using gnash::Player;
    using gnash::LogFile;
    using gnash::RcInitFile;

    std::vector<std::string> handlers;
    gnash::media::MediaFactory::instance().listKeys(back_inserter(handlers));

    std::vector<std::string> renderers;
    boost::split(renderers, RENDERER_CONFIG,
        boost::is_any_of(" "), boost::token_compress_on);

    po::options_description desc(_("Options"));

    desc.add_options()

    ("help,h",
        _("Print this help and exit"))

    ("version,V", 
        _("Print version information and exit"))

    ("scale,s", po::value<float>()
        ->notifier(boost::bind(&Player::setScale, &p,
                boost::bind(gnash::clamp<float>, _1, 0.01f, 100.f))),
        _("Scale the movie by the specified factor"))

    ("delay,d", po::value<int>()
        ->notifier(boost::bind(&Player::setDelay, &p, _1)),
        _("Number of milliseconds to delay in main loop"))

    ("audio-dump,A", po::value<string>()
        ->notifier(boost::bind(&Player::setAudioDumpfile, &p, _1)),
        _("Audio dump file (wave format)"))

    ("hwaccel", po::value<string>()
        ->default_value("none")
        ->notifier(boost::bind(&Player::setHWAccel, &p, _1)),
        (string(_("Hardware Video Accelerator to use"))
        + string("\nnone|vaapi")). c_str()) 

    ("xid,x", po::value<long>()
        ->notifier(boost::bind(&Player::setWindowId, &p, _1)),
        _("X11 Window ID for display"))

    ("width,j", po::value<int>()
        ->notifier(boost::bind(&Player::setWidth, &p, _1)),
        _("Set window width"))

    ("height,k", po::value<int>()
        ->notifier(boost::bind(&Player::setHeight, &p, _1)),
        _("Set window height"))

    ("x-pos,X", po::value<int>()
        ->notifier(boost::bind(&Player::setXPosition, &p, _1)),
        _("Set window x position"))

    ("y-pos,Y", po::value<int>()
        ->notifier(boost::bind(&Player::setYPosition, &p, _1)),
        _("Set window y position"))

    ("once,1", po::bool_switch()
        ->notifier(boost::bind(&Player::setDoLoop, &p,
                boost::bind(std::logical_not<bool>(), _1))),
        _("Exit when/if movie reaches the last frame"))

    ("render-mode,r", po::value<int>()
        ->default_value(3)
        ->notifier(boost::bind(&setupSoundAndRendering, boost::ref(p), _1)),
        (string("0 ")
        + string(_("disable rendering and sound")) 
        + string("\n1 ")
        + string(_("enable rendering, disable sound"))
        + string("\n2 ")
        + string(_("enable sound, disable rendering"))
        + string("\n3 ")
        + string(_("enable rendering and sound"))
        ).c_str())

    ("media,M", po::value<string>()
        ->default_value(rcfile.getMediaHandler().empty() ?
            ( handlers.empty() ? "" : handlers.front() )
                                                 : rcfile.getMediaHandler() )
        ->notifier(boost::bind(&Player::setMedia, &p, _1)),
        (string(_("The media handler to use"))
         + string("\n") + boost::join(handlers, "|")
        ).c_str())

    ("renderer,R", po::value<string>()
        ->default_value(rcfile.getRenderer().empty() ? renderers.front()
                                                     : rcfile.getRenderer())
        ->notifier(boost::bind(&Player::setRenderer, &p, _1)),
        (string(_("The renderer to use"))
        + string("\n") + boost::join(renderers, "|")
        ).c_str())

    ("timeout,t", po::value<float>()
        ->notifier(boost::bind(&Player::setExitTimeout, &p, _1)),
        _("Exit after the specified number of seconds"))

    ("real-url,u", po::value<string>(&url),
        _("Set \"real\" URL of the movie"))

    ("base-url,U", po::value<string>()
        ->notifier(boost::bind(&Player::setBaseUrl, &p, _1)),
        _("Set \"base\" URL for resolving relative URLs"))

    ("param,P", po::value<std::vector<std::string> >()
        ->composing()
        ->notifier(boost::bind(&setupFlashVars, boost::ref(p), _1)),
        _("Set parameter (e.g. \"FlashVars=A=1&b=2\")"))

    ("fd,F", po::value<string>()
        ->notifier(boost::bind(&setupFDs, boost::ref(p), _1)),
        (string(_("Filedescriptor to use for external communications"))
        + string(" <fd>:<fd>")
        ).c_str())

    ("max-advances", po::value<size_t>()
        ->notifier(boost::bind(&Player::setMaxAdvances, &p, _1)),
        _("Exit after specified number of frame advances"))

    ("fullscreen", po::bool_switch()
        ->notifier(boost::bind(&Player::setStartFullscreen, &p, _1)),
        _("Start in fullscreen mode"))

    // TODO: move to GUIs actually implementing this
    ("hide-menubar", po::bool_switch()
        ->notifier(boost::bind(&Player::hideMenu, &p, _1)),
        _("Start without displaying the menu bar"))

    // TODO: do this in ScreenShotter class.
    ("screenshot", po::value<string>()
        ->notifier(boost::bind(&Player::setScreenShots, &p, _1)),
        _("List of frames to save as screenshots"))

    ("screenshot-file", po::value<string>()
        ->notifier(boost::bind(&Player::setScreenShotFile, &p, _1)),
        _("Filename pattern for screenshot images"))

    ("screenshot-quality", po::value<size_t>()
        ->notifier(boost::bind(&Player::setScreenShotQuality, &p, _1)),
        _("Quality for screenshot output (not all formats)"))

    ("input-file", po::value<std::vector<std::string> >(&infiles),
        _("Input files"))
    ;

    desc.add(getDebuggingOptions(p));


// Add gui-specific options
// TODO make this somehow cleaner, maybe add a signature
//      for such function in each of the gui's mains
//      like getGuiOptions()
#ifdef GUI_DUMP
    po::options_description dumpOpts (_("Dump options"));

    dumpOpts.add_options()

    (",D", po::value<string>(),
        _("Video dump file (raw format) and optional video FPS (@<num>)"))
    (",S", po::value<string>(),
        _("Number of milliseconds to sleep between advances"))
    (",T", po::value<string>(),
        _("Trigger expression to start dumping"))
    ;

    desc.add(dumpOpts);
#endif

    return desc;
}

void
usage_gui_keys(std::ostream& os)
{
    os << _("Keys:\n")
       << "  CTRL-Q, CTRL-W           "
       << _("Quit\n")
       << "  CTRL-F                   "
       << _("Toggle fullscreen\n")
       << "  CTRL-P                   "
       << _("Toggle pause\n") 
       << "  CTRL-R                   "
       << _("Restart the movie\n") 
       << "  CTRL-O                   "
       << _("Take a screenshot\n") 
       << "  CTRL-L                   "
       << _("Force immediate redraw\n");
}

void
usage(std::ostream& os, const po::options_description& opts)
{
    os << _("Usage: gnash [options] movie_file.swf\n")
       << _("Plays a SWF (Shockwave Flash) movie\n")
       << opts << "\n";

    // Add gui keys
    // TODO: stop printing these in here ?
    usage_gui_keys(os);

    os << std::endl;
}

void
version_and_copyright(std::ostream& os)
{
    os << "Gnash "
       << VERSION " ("
       << BRANCH_NICK << "-" << BRANCH_REVNO << "-" << COMMIT_ID
       << ")" << endl << endl
       << _("Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011 "
            "Free Software Foundation, Inc.\n"
            "Gnash comes with NO WARRANTY, to the extent permitted "
            "by law.\nYou may redistribute copies of Gnash under the "
            "terms of the GNU General\nPublic License.  For more "
            "information, see the file named COPYING.\n\n");
}

void
build_options(std::ostream& os)
{
    os << _("Build options ") << endl
	   << _("   Renderers: ") << RENDERER_CONFIG << endl
	   << _("   Hardware Acceleration: ") << HWACCEL_CONFIG << endl
	   << _("   GUI: ") << GUI_CONFIG << endl
	   << _("   Media handlers: ") << MEDIA_CONFIG << endl
        
	   << _("   Configured with: ") << CONFIG_CONFIG << endl
	   << _("   CXXFLAGS: ") << CXXFLAGS << endl;
}


} // unnamed namespace

