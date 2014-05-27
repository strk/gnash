// Player.cpp:  Top level SWF player, for gnash.
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

#include "Player.h"

#include <iostream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/any.hpp>
#include <utility>
#include <memory>
#include <vector>

#include "gui.h"
#include "NullGui.h"
#include "MovieFactory.h"
#include "movie_definition.h"
#include "sound_handler.h" // for set_sound_handler and create_sound_handler_*
#include "MovieClip.h" // for setting FlashVars
#include "movie_root.h" 
#include "StreamProvider.h"
#include "swf/TagLoadersTable.h"
#include "swf/DefaultTagLoaders.h"
#include "NamingPolicy.h"
#include "StringPredicates.h"
#include "URL.h"
#include "rc.h"
#include "GnashException.h"
#include "noseek_fd_adapter.h"
#include "VM.h"
#include "SystemClock.h"
#include "ExternalInterface.h"
#include "ScreenShotter.h"
#include "GnashSystemIOHeaders.h" // for write() 
#include "log.h"
#include "HostInterface.h"
#include "RunResources.h"
#include "IOChannel.h"
#include "MediaHandler.h"
#include "GnashFactory.h"

using namespace gnash;

namespace {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

namespace {

class MessageHandler : public boost::static_visitor<boost::any>
{
public:
    explicit MessageHandler(Gui& g) : _gui(g) {}

    boost::any operator()(const HostMessage& e) {

        switch (e.event()) {

            case HostMessage::NOTIFY_ERROR:
                _gui.error(boost::any_cast<std::string>(e.arg()));
                return boost::blank();

            case HostMessage::QUERY:
                return _gui.yesno(boost::any_cast<std::string>(e.arg()));

            case HostMessage::SHOW_MOUSE:
            {
                // Must return a bool, true if the mouse was visible before.
                return _gui.showMouse(boost::any_cast<bool>(e.arg()));   
            }

            case HostMessage::SET_DISPLAYSTATE:
            {
                const movie_root::DisplayState s =
                    boost::any_cast<movie_root::DisplayState>(e.arg());
                if (s == movie_root::DISPLAYSTATE_FULLSCREEN) {
                    _gui.setFullscreen();
                }
                else if (s == movie_root::DISPLAYSTATE_NORMAL) {
                    _gui.unsetFullscreen();
                }
                return boost::blank();
            }

            case HostMessage::UPDATE_STAGE:
                _gui.updateStageMatrix();
                return boost::blank();

            case HostMessage::SHOW_MENU:
                _gui.showMenu(boost::any_cast<bool>(e.arg()));
                return boost::blank();

            case HostMessage::SET_CLIPBOARD:
                _gui.setClipboard(boost::any_cast<std::string>(e.arg()));
                return boost::blank();

            case HostMessage::RESIZE_STAGE:
            {
                if (_gui.isPlugin()) {
                    log_debug("Player doing nothing on Stage.resize as we're a plugin");
                    return boost::blank();
                }

                typedef std::pair<int, int> Dimensions;
                const Dimensions i = boost::any_cast<Dimensions>(e.arg());
                _gui.resizeWindow(i.first, i.second);
                return boost::blank();
            }
            case HostMessage::EXTERNALINTERFACE_ISPLAYING:
                return (_gui.isStopped()) ? false : true;

            case HostMessage::EXTERNALINTERFACE_PAN:
                log_unimpl(_("GUI ExternalInterface.Pan event"));
                return boost::blank();

            case HostMessage::EXTERNALINTERFACE_PLAY:
                _gui.play();
                return boost::blank();

            case HostMessage::EXTERNALINTERFACE_REWIND:
                _gui.restart();
                return boost::blank();

            case HostMessage::EXTERNALINTERFACE_SETZOOMRECT:
                log_unimpl(_("GUI ExternalInterface.SetZoomRect event"));
                return boost::blank();

            case HostMessage::EXTERNALINTERFACE_STOPPLAY:
                _gui.pause();
                return boost::blank();

            case HostMessage::EXTERNALINTERFACE_ZOOM:
                log_unimpl(_("GUI ExternalInterface.Zoom event"));
                return boost::blank();

            case HostMessage::SCREEN_RESOLUTION:
                return _gui.screenResolution();

            case HostMessage::SCREEN_DPI:
                return _gui.getScreenDPI();

            case HostMessage::SCREEN_COLOR:
                return _gui.getScreenColor();

            case HostMessage::PIXEL_ASPECT_RATIO:
                return _gui.getPixelAspectRatio();

            case HostMessage::PLAYER_TYPE:
                return std::string(_gui.isPlugin() ? "PlugIn" : "StandAlone");
        }
        log_error(_("Unhandled callback %s with arguments %s"), +e.event());
        return boost::blank();
    }

    boost::any operator()(const CustomMessage& /*e*/) {
        return boost::blank();
    }
private:
    Gui& _gui;
};

}

void
Player::setFlashVars(const std::string& varstr)
{
    typedef Gui::VariableMap maptype;

    maptype vars;
    URL::parse_querystring(varstr, vars);

    _gui->addFlashVars(vars);
}

Player::Player()
    :
    _scale(1.0f),
    _delay(0),
    _width(0),
    _height(0),
    _xPosition(-1),
    _yPosition(-1),
    _windowID(0),
    _doLoop(true),
    _doRender(true),
    _doSound(true),
    _exitTimeout(0),
    _movieDef(nullptr),
    _maxAdvances(0),
#ifdef GNASH_FPS_DEBUG
    _fpsDebugTime(0.0),
#endif
    _hostfd(-1),
    _controlfd(-1),
    _startFullscreen(false),
    _hideMenu(false),
    _screenshotQuality(100)
{
}

float
Player::setScale(float newscale)
{
    float oldscale = _scale;
    _scale = newscale;
    return oldscale;
}

void
Player::init_logfile()
{
    dbglogfile.setWriteDisk(false);

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    if (rcfile.useWriteLog()) {
        dbglogfile.setWriteDisk(true);
    }

    dbglogfile.setLogFilename(rcfile.getDebugLog());

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
    
    // If a delay was not specified yet use
    // any eventual setting for it found in 
    // the RcInitFile
    //
    // TODO: we should remove all uses of the rcfile
    //       from Player class..
    //
    if (!_delay && rcfile.getTimerDelay() > 0) {
        _delay = rcfile.getTimerDelay();
        log_debug ("Timer delay set to %d milliseconds", _delay);
    }    

}

void
Player::init_sound()
{

    if (_doSound) {
        try {
#ifdef SOUND_SDL
            _soundHandler.reset(sound::create_sound_handler_sdl(
                        _mediaHandler.get()));
#elif defined(SOUND_AHI)
            _soundHandler.reset(sound::create_sound_handler_aos4(
                        _mediaHandler.get()));
#elif defined(SOUND_MKIT)
            _soundHandler.reset(sound::create_sound_handler_mkit(
                        _mediaHandler.get()));
#else
            log_error(_("Sound requested but no sound support compiled in"));
            return;
#endif

        } catch (const SoundException& ex) {
            log_error(_("Could not create sound handler: %s."
                " Will continue without sound."), ex.what());
        }
    }
}

void
Player::init_gui()
{
    if (_doRender) {
        _gui = getGui();
    } else {
        _gui.reset(new NullGui(_doLoop, *_runResources));
    }

    _gui->setAudioDump(_audioDump);
    _gui->setMaxAdvances(_maxAdvances);

#ifdef GNASH_FPS_DEBUG
    if (_fpsDebugTime) {
        log_debug("Activating FPS debugging every %g seconds",
                _fpsDebugTime);
        _gui->setFpsTimerInterval(_fpsDebugTime);
    }
#endif
}

boost::intrusive_ptr<movie_definition>
Player::load_movie()
{
    /// The RunResources must be initialized by this point to provide resources
    /// for parsing.
    assert(_runResources.get());

    boost::intrusive_ptr<gnash::movie_definition> md;

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    URL vurl(_url);

    if (vurl.protocol() == "file") {
        const std::string& path = vurl.path();
        size_t lastSlash = path.find_last_of('/');
        std::string dir = path.substr(0, lastSlash+1);
        rcfile.addLocalSandboxPath(dir);
        log_debug("%s appended to local sandboxes", dir.c_str());
    }

    try {
        if (_infile == "-") {
            std::unique_ptr<IOChannel> in (
                noseek_fd_adapter::make_stream(fileno(stdin)));
            md = MovieFactory::makeMovie(std::move(in), _url, *_runResources, false);
        }
        else {
            URL url(_infile);
            if ( url.protocol() == "file" ) {
                std::string path = url.path();
                // We'll need to allow load of the file, no matter virtual url
                // specified...
                // This is kind of hackish, cleaner would be adding an argument
                // to createMovie to skip the security checking phase.
                // NOTE that if we fail to allow this load, the konqueror plugin
                // would not be able to load anything
                //
                rcfile.addLocalSandboxPath(path);
                log_debug("%s appended to local sandboxes", path.c_str());
            }

            // _url should be always set at this point...
            md = MovieFactory::makeMovie(url, *_runResources, _url.c_str(),
                    false);
        }
    }
    catch (const GnashException& er) {
        std::cerr << er.what() << std::endl;
        md = nullptr;
    }

    if (!md) {
        fprintf(stderr, "Could not load movie '%s'\n", _infile.c_str());
        return nullptr;
    }

    return md;
}

/// \brief Run, used to open a new flash file. Using previous initialization
void
Player::run(int argc, char* argv[], const std::string& infile,
        const std::string& url)
{
    // Call this at run() time, so the caller has
    // a cache of setting some parameter before calling us...
    // (example: setDoSound(), setWindowId() etc.. ) 
    init_logfile();
   
    // gnash.cpp should check that a filename is supplied.
    assert (!infile.empty());

    _infile = infile;

    // Work out base url
    if (_baseurl.empty()) {
        if (!url.empty()) _baseurl = url;
        else if (infile == "-") _baseurl = URL("./").str();
        else _baseurl = infile;
    }

    // Set _root._url (either explicit of from infile)
    if (!url.empty()) {
        _url = url;
    } else {
        _url = infile;
    }

    // Parse player parameters. These are not passed to the SWF, but rather
    // control stage properties etc.
    // NOTE: it is intentional to force a trailing slash to "base" argument
    //       as it was tested that the "base" argument is always considered
    //       a directory!
    Params::const_iterator it = _params.find("base");
    const URL baseURL = (it == _params.end()) ? _baseurl :
                                               URL(it->second+"/", _baseurl);
    /// The RunResources should be populated before parsing.
    _runResources.reset(new RunResources());

    std::shared_ptr<SWF::TagLoadersTable> loaders(new SWF::TagLoadersTable());
    addDefaultLoaders(*loaders);
    _runResources->setTagLoaders(loaders);

    std::unique_ptr<NamingPolicy> np(new IncrementalRename(_baseurl));

    /// The StreamProvider uses the actual URL of the loaded movie.
    std::shared_ptr<StreamProvider> sp(new StreamProvider(_url, baseURL, std::move(np)));
    _runResources->setStreamProvider(sp);

    // Set the Hardware video decoding resources. none, vaapi, omap
    _runResources->setHWAccelBackend(_hwaccel);
    
    // Set the Renderer resource, opengl, openvg, agg, or cairo
    _runResources->setRenderBackend(_renderer);

#ifdef USE_MEDIA
    _mediaHandler.reset(media::MediaFactory::instance().get(_media));
#endif
    
    if (!_media.empty() && !_mediaHandler.get()) {
        boost::format fmt =
            boost::format(_("Non-existent media handler %1% specified"))
            % _media;
        throw GnashException(fmt.str());
    }

    _runResources->setMediaHandler(_mediaHandler);
    
    init_sound();
    _runResources->setSoundHandler(_soundHandler);
    
    init_gui();

    // Initialize gui (we need argc/argv for this)
    // note that this will also initialize the renderer
    // which is *required* during movie loading
    if (!_gui->init(argc, &argv)) {
        throw GnashException("Could not initialize GUI");
    }

    // Parse querystring (before FlashVars, see
    // testsuite/misc-ming.all/FlashVarsTest*)
    setFlashVars(URL(_url).querystring());

    // Add FlashVars.
    Params::const_iterator fv = _params.find("flashvars");
    if (fv != _params.end()) {
        setFlashVars(fv->second);
    }

    // Load the actual movie.
    _movieDef = load_movie();
    if (!_movieDef) {
        throw GnashException("Could not load movie!");
    }

    // Get info about the width & height of the movie.
    const size_t movie_width = _movieDef->get_width_pixels();
    const size_t movie_height = _movieDef->get_height_pixels();

    if (! _width) {
        _width = static_cast<size_t>(movie_width * _scale);
    }
    if (! _height) {
        _height = static_cast<size_t>(movie_height * _scale);
    }

    if (!_width || !_height) {
        log_debug("Input movie has collapsed dimensions "
                  "%d/%d. Setting to 1/1 and going on.",
                  _width, _height);
        if (!_width) _width = 1;
        if (!_height) _height = 1;
    }

    // Register movie definition before creating the window
    _gui->setMovieDefinition(_movieDef.get());

    // Now that we know about movie size, create gui window.
    _gui->createWindow(_url.c_str(), _width, _height, _xPosition, _yPosition);

    { // we construct movie_root in its own scope, to be sure it's destroyed
      // (bringing down the MovieLoader) before we clear the MovieLibrary

    movie_root root(_gui->getClock(), *_runResources);
    
    _callbacksHandler.reset(new CallbacksHandler(*_gui, *this)); 
    
    // Register Player to receive events from the core (Mouse, Stage,
    // System etc)
    root.registerEventCallback(_callbacksHandler.get());
    
    // Register Player to receive FsCommand events from the core.
    root.registerFSCommandCallback(_callbacksHandler.get());

    // log_debug("Player Host FD #%d, Player Control FD #%d", _hostfd, _controlfd);
    
    // Set host requests fd (if any)
    if ( _hostfd != -1 ) {
        root.setHostFD(_hostfd);
    }
    
    if (_controlfd != -1) {
        root.setControlFD(_controlfd);        
    }

    _gui->setStage(&root);
    
    // When startStopped is true, stop here after the stage has been 
    // registered, but before the movie has started. Initial loading
    // and VM initialization have been done by this stage, but not
    // the complete parsing of the SWF. This is important because
    // the Gui accesses movie_root to get the sound_handler, but also
    // because the gui window should be properly set up by this point.
    RcInitFile& rcfile = RcInitFile::getDefaultInstance();

    if (rcfile.startStopped()) {
        _gui->stop();
    }

    // Start loader thread
    // NOTE: the loader thread might (in IMPORT tag parsing)
    //       create new movies and register them to the MovieLibrary.
    //       If MovieLibrary size exceeded, _movieDef might be
    //       destroyed prematurely. movie_root might actually be
    //       keeping it alive, as Gui might as well, but why relying
    //       on luck ? So we made sure to keep _movieDef by 
    //       intrusive_ptr...
    _movieDef->completeLoad();

    if (! _delay) {
        float fps = _movieDef->get_frame_rate();
        // log_debug("Movie Frame Rate is %g, adjusting delay", fps);
        // FIXME: this value is arbitrary, and will make any movie with
        // less than 12 frames eat up more of the cpu. It should probably
        // be a much lower value, like 2.
        if (fps > 12) {
            _delay = static_cast<int>(1000/fps);
        } else {
            // 10ms per heart beat
            _delay = 10;
        }
    }
    // This is the time between the main loop waking up and processing
    // network messages, external calls, and displaying the next frame.
    _gui->setInterval(_delay);

    if (_exitTimeout) {
      _gui->setTimeout(static_cast<unsigned int>(_exitTimeout * 1000));
    }

    if (!_windowID && _startFullscreen) {
        _gui->setFullscreen();
    }

    if (!_windowID && _hideMenu) {
        _gui->hideMenu();
    }
    
    // Now handle stage alignment and scale mode. This should be done after
    // the GUI is created, after its stage member is set, and after the
    // interface callbacks are registered.
    it = _params.find("salign");
    if (it != _params.end()) {
        log_debug("Setting align");
        const short align = stringToStageAlign(it->second);
        root.setStageAlignment(align);
    }

    it = _params.find("allowscriptaccess");
    if (it != _params.end()) {
        StringNoCaseEqual noCaseCompare;
        const std::string& str = it->second;
                
        movie_root::AllowScriptAccessMode mode = 
            movie_root::SCRIPT_ACCESS_SAME_DOMAIN;
        
        if (noCaseCompare(str, "never")) {
            mode = movie_root::SCRIPT_ACCESS_NEVER;
        } 
        else if (noCaseCompare(str, "sameDomain")) {
            mode = movie_root::SCRIPT_ACCESS_SAME_DOMAIN;
        } 
        else if (noCaseCompare(str, "always")) {
            mode = movie_root::SCRIPT_ACCESS_ALWAYS;
        }
        log_debug("Setting allowscriptaccess to %s", mode);
        root.setAllowScriptAccess(mode);
    }

    it = _params.find("scale");
    if (it != _params.end()) {                
        StringNoCaseEqual noCaseCompare;
        const std::string& str = it->second;
        movie_root::ScaleMode mode = movie_root::SCALEMODE_SHOWALL;
        
        if (noCaseCompare(str, "noScale")) {
            mode = movie_root::SCALEMODE_NOSCALE;
        } 
        else if (noCaseCompare(str, "exactFit")) {
            mode = movie_root::SCALEMODE_EXACTFIT;
        }
        else if (noCaseCompare(str, "noBorder")) {
            mode = movie_root::SCALEMODE_NOBORDER;
        }

        log_debug("Setting scale mode");
            root.setStageScaleMode(mode);
    }

    // Set up screenshots. 
    if (!_screenshots.empty()) {
        std::istringstream is(_screenshots);
        std::string arg;
        bool last = false;
        ScreenShotter::FrameList v;

        while (std::getline(is, arg, ',')) {
            if (arg == "last") last = true;
            else try {
                const size_t frame = boost::lexical_cast<size_t>(arg);
                v.push_back(frame);
            }
            catch (const boost::bad_lexical_cast&) {}
        }

        // Use default if filename is empty.
        if (_screenshotFile.empty()) {
            URL url(_runResources->streamProvider().baseURL());
            std::string::size_type p = url.path().rfind('/');
            const std::string& name = (p == std::string::npos) ? url.path() :
                url.path().substr(p + 1);
            _screenshotFile = "screenshot-" + name + "-%f";
        }
        if (!last && v.empty()) return;
        
        std::unique_ptr<ScreenShotter> ss(new ScreenShotter(_screenshotFile,
                                                          _screenshotQuality));
        if (last) ss->lastFrame();
        ss->setFrames(v);
        _gui->setScreenShotter(std::move(ss));
    }

    _gui->run();

    log_debug("Main loop ended, cleaning up");

    }

    // Clean up the MovieLibrary so left-over SWFMovieDefinitions
    // get destroyed and join any loader thread
    MovieFactory::clear();

}

void
Player::CallbacksHandler::exit()
{
    _gui.quit();
}

boost::any
Player::CallbacksHandler::call(const HostInterface::Message& e)
{
    MessageHandler v(_gui);
    try {
        return boost::apply_visitor(v, e);
    }
    catch (const boost::bad_any_cast&) {
        log_error(_("Got unexpected argument type for message %1%"), e);
        return boost::blank();
    }
}

void
Player::CallbacksHandler::notify(const std::string& command,
        const std::string& args)
{
    //log_debug(_("fs_callback(%p): %s %s"), (void*)movie, command, args);

    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    // it's _hostfd, but we're a static method...
    const int hostfd = _player.getHostFD();
    if (hostfd != -1) {
        //log_debug("user-provided host requests fd is %d", hostfd);
        std::stringstream request;
	std::vector<as_value> fnargs;
	fnargs.push_back(as_value(command));
	fnargs.push_back(as_value(args));
	request << ExternalInterface::makeInvoke("fsCommand", fnargs);

        std::string requestString = request.str();
        const char* cmd = requestString.c_str();
        size_t len = requestString.length();
        // TODO: should mutex-protect this ?
        // NOTE: we assuming the hostfd is set in blocking mode here..
        //log_debug("Attempt to write INVOKE requests fd %d", hostfd);
        int ret = write(hostfd, cmd, len);
        if ( ret == -1 ) {
            log_error(_("Could not write to user-provided host "
                        "requests fd %d: %s"), hostfd, strerror(errno));
        }
        if ( static_cast<size_t>(ret) < len ) {
            log_error(_("Could only write %d bytes over %d required to "
                        "user-provided host requests fd %d"),
                      ret, len, hostfd);
        }

        // Remove the newline for logging
        requestString.resize(requestString.size() - 1);
        log_debug("Sent FsCommand '%s' to host fd %d",
                    requestString, hostfd);
    }

    /// Fscommands can be ignored using an rcfile setting. As a 
    /// plugin they are always ignored.
    if (_gui.isPlugin()) {
        // We log the request to the fd above
        log_debug("Running as plugin: skipping internal "
                    "handling of FsCommand %s%s.");
        return;
    }
    
    // This only disables fscommands for the standalone player. In the
    // plugin or a hosting application, the fscommands are always passed
    // on; the hosting application should decide what to do with them.
    // (Or do we want to allow disabling all external communication?) 
    if (rcfile.ignoreFSCommand()) return;

    StringNoCaseEqual noCaseCompare;

    // There are six defined FsCommands handled by the standalone player:
    // quit, fullscreen, showmenu, exec, allowscale, and trapallkeys.
    
    // FSCommand quit
    if (noCaseCompare(command, "quit")) {
        _gui.quit();
        return;
    }

    // FSCommand fullscreen
    if (noCaseCompare(command, "fullscreen")) {
        if (noCaseCompare(args, "true")) _gui.setFullscreen();
        else if (noCaseCompare(args, "false")) _gui.unsetFullscreen();
        return;
    }
       
    // FSCommand showmenu
    if (noCaseCompare(command, "showmenu")) {
        if (noCaseCompare(args, "true")) _gui.showMenu(true);
        else if (noCaseCompare(args, "false")) _gui.showMenu(false);
        return;
    }

    // FSCommand exec
    // Note: the pp insists that the file to execute should be in 
    // a subdirectory 'fscommand' of the 'projector' executable's
    // location. In SWF5 there were no restrictions.
    if (noCaseCompare(command, "exec")) {
        log_unimpl(_("FsCommand exec called with argument %s"), args);
        return;
    }

    // FSCommand allowscale
    if (noCaseCompare(command, "allowscale")) {
        //log_debug("allowscale: %s", args);
        if (noCaseCompare(args, "true")) _gui.allowScale(true);
        else {
            if (strtol(args.c_str(), nullptr, 0)) _gui.allowScale(true);
            else _gui.allowScale(false);
        }
        return;
    }

    // FSCommand trapallkeys
    if (noCaseCompare(command, "trapallkeys")) {
        log_unimpl(_("FsCommand trapallkeys called with argument %s"), args);
        return;
    }
       
    // The plugin never reaches this point; anything sent to the fd has
    // been logged already.
    log_debug(_("FsCommand '%s(%s)' not handled internally"),
            command, args);

}


// private
std::unique_ptr<Gui>
Player::getGui()
{
#ifdef GUI_GTK
    return createGTKGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_KDE3
    return createKDEGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_QT4
    return createQt4Gui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_SDL
    return createSDLGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_AQUA
    return createAQUAGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_RISCOS
    return createRISCOSGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_FLTK
    return createFLTKGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_FB
    return gui::createFBGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_AOS4
    return createAOS4Gui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_HAIKU
    return createHaikuGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_DUMP
    return createDumpGui(_windowID, _scale, _doLoop, *_runResources);
#endif

    return std::unique_ptr<Gui>(new NullGui(_doLoop, *_runResources));
}

Player::~Player()
{
    if (_movieDef.get()) {
        log_debug("~Player - _movieDef refcount: %d (1 will be dropped "
                "now)", _movieDef->get_ref_count());
    }
}
