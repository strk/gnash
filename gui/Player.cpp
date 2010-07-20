// Player.cpp:  Top level SWF player, for gnash.
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

#ifndef DEFAULT_GUI
# define DEFAULT_GUI "NULL"
#endif

#include "gui.h"
#include "NullGui.h"

#include "MovieFactory.h"
#include "movie_definition.h"
#include "sound_handler.h" // for set_sound_handler and create_sound_handler_*
#include "MovieClip.h" // for setting FlashVars
#include "movie_root.h" 
#include "Player.h"
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

#include "GnashSystemIOHeaders.h" // for write() 
#include "log.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <boost/lexical_cast.hpp>

using namespace gnash;

namespace {
    gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

void
Player::setFlashVars(const std::string& varstr)
{
    typedef Gui::VariableMap maptype;

    maptype vars;
    URL::parse_querystring(varstr, vars);

    _gui->addFlashVars(vars);
}

void
Player::setScriptableVar(const std::string &name, const std::string &value)
{
    _gui->addScriptableVar(name, value);
}

Player::Player()
    :
#if defined(RENDERER_CAIRO)
    _bitDepth(32),
#else
    _bitDepth(16),
#endif
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
    _movieDef(0),
    _maxAdvances(0),
#ifdef GNASH_FPS_DEBUG
    _fpsDebugTime(0.0),
#endif
    _hostfd(-1),
    _controlfd(-1),
    _startFullscreen(false),
    _hideMenu(false)
{
    init();
}

float
Player::setScale(float newscale)
{
    float oldscale = _scale;
    _scale = newscale;
    return oldscale;
}

void
Player::init()
{
    /// Initialize gnash core library
    gnashInit();

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
        log_debug (_("Timer delay set to %d milliseconds"), _delay);
    }    

}

unsigned int
Player::silentStream(void* /*udata*/, boost::int16_t* stream, unsigned int len, bool& atEOF)
{
    std::fill(stream, stream+len, 0);
    atEOF=false;
    return len;
}

void
Player::init_sound()
{

    if (_doSound) {
        try {
#ifdef SOUND_SDL
            _soundHandler.reset(sound::create_sound_handler_sdl(
                        _mediaHandler.get(), _audioDump));
#elif defined(SOUND_AHI)
            _soundHandler.reset(sound::create_sound_handler_aos4(
                        _mediaHandler.get(), _audioDump));
#elif defined(SOUND_MKIT)
            _soundHandler.reset(sound::create_sound_handler_mkit(
                        _mediaHandler.get(), _audioDump));
#else
            log_error(_("Sound requested but no sound support compiled in"));
            return;
#endif
        } catch (SoundException& ex) {
            log_error(_("Could not create sound handler: %s."
                " Will continue w/out sound."), ex.what());
        }
        if (! _audioDump.empty()) {
            // add a silent stream to the audio pool so that our output file
            // is homogenous;  we actually want silent wave data when no sounds
            // are playing on the stage
            _soundHandler->attach_aux_streamer(silentStream, (void*) this);
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

    _gui->setMaxAdvances(_maxAdvances);

#ifdef GNASH_FPS_DEBUG
    if (_fpsDebugTime) {
        log_debug(_("Activating FPS debugging every %g seconds"),
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
        log_debug(_("%s appended to local sandboxes"), dir.c_str());
    }

    try {
        if ( _infile == "-" ) {
            std::auto_ptr<IOChannel> in (
                noseek_fd_adapter::make_stream(fileno(stdin)));
            md = MovieFactory::makeMovie(in, _url, *_runResources, false);
        } else {
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
                log_debug(_("%s appended to local sandboxes"), path.c_str());
            }

            // _url should be always set at this point...
            md = MovieFactory::makeMovie(url, *_runResources, _url.c_str(),
                    false);
        }
    } catch (const GnashException& er) {
        std::cerr << er.what() << std::endl;
        md = NULL;
    }

    if ( ! md ) {
        fprintf(stderr, "Could not load movie '%s'\n", _infile.c_str());
        return NULL;
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
    Params::const_iterator it = _params.find("base");
    const URL baseURL = (it == _params.end()) ? _baseurl :
                                               URL(it->second, _baseurl);
    /// The RunResources should be populated before parsing.
    _runResources.reset(new RunResources(baseURL.str()));

    boost::shared_ptr<SWF::TagLoadersTable> loaders(new SWF::TagLoadersTable());
    addDefaultLoaders(*loaders);
    _runResources->setTagLoaders(loaders);

    std::auto_ptr<NamingPolicy> np(new IncrementalRename(_baseurl));
    boost::shared_ptr<StreamProvider> sp(new StreamProvider(np));

    _runResources->setStreamProvider(sp);

    // Set the Hardware video decoding resources. none, vaapi, xv, omap
    _runResources->setHWAccelBackend(_hwaccel);
    // Set the Renderer resource, opengl, agg, or cairo
    _runResources->setRenderBackend(_renderer);

    _mediaHandler.reset(media::MediaFactory::instance().get(_media));

    if (!_mediaHandler.get()) {
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

#if 0
    // Add Scriptable Variables. These values become the default, but
    // they can be reset from JavaScript via ExternalInterface. These
    // are passed to Gnash using the '-P' option, and have nothing to
    // to do with 'flashVars'.
    fv = _params.begin();
    for (fv=_params.begin(); fv != _params.end(); fv++) {
        if (fv->first != "flashvars") {
            setScriptableVar(fv->first, fv->second);
        }
    }
#endif
    
    // Load the actual movie.
    _movieDef = load_movie();
    if (!_movieDef) {
        throw GnashException("Could not load movie!");
    }

    // Get info about the width & height of the movie.
    int movie_width = static_cast<int>(_movieDef->get_width_pixels());
    int movie_height = static_cast<int>(_movieDef->get_height_pixels());

    if (! _width) {
        _width = static_cast<size_t>(movie_width * _scale);
    }
    if (! _height) {
        _height = static_cast<size_t>(movie_height * _scale);
    }

    if (! _width || ! _height) {
        log_debug(_("Input movie has collapsed dimensions "
                    "%d/%d. Setting to 1/1 and going on."),
                     _width, _height);
        if (!_width) _width = 1;
        if (!_height) _height = 1;
    }

    // Register movie definition before creating the window
    _gui->setMovieDefinition(_movieDef.get());

    // Now that we know about movie size, create gui window.
    _gui->createWindow(_url.c_str(), _width, _height, _xPosition, _yPosition);

    movie_root root(*_movieDef, _gui->getClock(), *_runResources);
    
    _callbacksHandler.reset(new CallbacksHandler(*_gui, *this)); 
    
    // Register Player to receive events from the core (Mouse, Stage,
    // System etc)
    root.registerEventCallback(_callbacksHandler.get());
    
    // Register Player to receive FsCommand events from the core.
    root.registerFSCommandCallback(_callbacksHandler.get());

    log_debug("Player Host FD #%d, Player Control FD #%d", 
                     _hostfd, _controlfd);
    
    // Set host requests fd (if any)
    if ( _hostfd != -1 ) {
        root.setHostFD(_hostfd);
    }
    
    if (_controlfd != -1) {
        root.setControlFD(_controlfd);        
//        _gui->setFDCallback(_controlfd, boost::bind(&Gui::quit, boost::ref(_gui)));
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
        // 10ms per heart beat
        _delay = 10; 
    }
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
        std::string access = it->second;
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
            URL url(_runResources->baseURL());
            std::string::size_type p = url.path().rfind('/');
            const std::string& name = (p == std::string::npos) ? url.path() :
                url.path().substr(p + 1);
            _screenshotFile = "screenshot-" + name + "-%f";
        }

        _gui->requestScreenShots(v, last, _screenshotFile);
    }

    _gui->run();

    log_debug("Main loop ended, cleaning up");

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

}

void
Player::CallbacksHandler::exit()
{
    _gui.quit();
}

void
Player::CallbacksHandler::error(const std::string& msg)
{
    _gui.error(msg);
}

bool
Player::CallbacksHandler::yesNo(const std::string& query)
{
    return _gui.yesno(query);
}

std::string
Player::CallbacksHandler::call(const std::string& event, const std::string& arg)
{
    StringNoCaseEqual noCaseCompare;
        
    if (event == "Mouse.hide") {
        return _gui.showMouse(false) ? "true" : "false";
    }

    if (event == "Mouse.show") {
        return _gui.showMouse(true) ? "true" : "false";
    }
    
    if (event == "Stage.displayState") {
        if (arg == "fullScreen") _gui.setFullscreen();
        else if (arg == "normal") _gui.unsetFullscreen();
        return "";
    }

    if (event == "Stage.scaleMode" || event == "Stage.align" ) {
        _gui.updateStageMatrix();
        return "";
    }

    if (event == "Stage.showMenu") {
        if (noCaseCompare(arg, "true")) _gui.showMenu(true);
        else if (noCaseCompare(arg, "false")) _gui.showMenu(false);
        return "";
    }

    if (event == "Stage.resize") {
        if ( _gui.isPlugin() ) {
            log_debug("Player doing nothing on Stage.resize as we're a plugin");
            return "";
        }

        // arg contains WIDTHxHEIGHT
        log_debug("Player got Stage.resize(%s) message", arg);
        int width, height;
        sscanf(arg.c_str(), "%dx%d", &width, &height);
        _gui.resizeWindow(width, height);

        return "";
    }

    if (event == "ExternalInterface.Play") {
        _gui.play();
        return "";
    }

    if (event == "ExternalInterface.StopPlay") {
        _gui.pause();
        return "";
    }

    if (event == "ExternalInterface.Rewind") {
        _gui.restart();
        return "";
    }

    if (event == "ExternalInterface.Pan") {
	// FIXME: the 3 args are encoded as 1:2:0
	log_unimpl("ExternalInterface.Pan");
        return "";
    }

    if (event == "ExternalInterface.IsPlaying") {
	return (_gui.isStopped()) ? "false" : "true";
    }

    if (event == "ExternalInterface.SetZoomRect") {
	// FIXME: the 4 arguments are encoded as 1:2:0:1
	log_unimpl("ExternalInterface.SetZoomRect");
        return "";
    }

    if (event == "ExternalInterface.Zoom") {
	// The 1 argument is a percentage to zoom
	int percent = strtol(arg.c_str(), NULL, 0);
	log_unimpl("ExternalInterface.Zoom(%d)", percent);
        return "";
    }

    if (event == "System.capabilities.screenResolutionX") {
        std::ostringstream ss;
        ss << _gui.getScreenResX();
        return ss.str();
    }

    if (event == "System.capabilities.screenResolutionY") {
        std::ostringstream ss;
        ss << _gui.getScreenResY();
        return ss.str();
    }

    if (event == "System.capabilities.pixelAspectRatio") {
        std::ostringstream ss;
        // Whether the pp actively limits the precision or simply
        // gets a slightly different result isn't clear.
        ss << std::setprecision(7) << _gui.getPixelAspectRatio();
        return ss.str();
    }

    if (event == "System.capabilities.screenDPI") {
        std::ostringstream ss;
        ss << _gui.getScreenDPI();
        return ss.str();
    }

    if (event == "System.capabilities.screenColor") {
        return _gui.getScreenColor();
    }

    if (event == "System.capabilities.playerType") {
        return _gui.isPlugin() ? "PlugIn" : "StandAlone";
    }

    log_error(_("Unhandled callback %s with arguments %s"), event, arg);
    return "";
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
            log_error("Could not write to user-provided host "
                      "requests fd %d: %s", hostfd, strerror(errno));
        }
        if ( static_cast<size_t>(ret) < len ) {
            log_error("Could only write %d bytes over %d required to "
                      "user-provided host requests fd %d",
                      ret, len, hostfd);
        }

        // Remove the newline for logging
        requestString.resize(requestString.size() - 1);
        log_debug(_("Sent FsCommand '%s' to host fd %d"),
                    requestString, hostfd);
    }

    /// Fscommands can be ignored using an rcfile setting. As a 
    /// plugin they are always ignored.
    if (_gui.isPlugin()) {
        // We log the request to the fd above
        log_debug(_("Running as plugin: skipping internal "
                    "handling of FsCommand %s%s."));
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
            if (strtol(args.c_str(), NULL, 0)) _gui.allowScale(true);
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
std::auto_ptr<Gui>
Player::getGui()
{
#ifdef GUI_GTK
    return createGTKGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_KDE3
    return createKDEGui(_windowID, _scale, _doLoop, *_runResources);
#endif

#ifdef GUI_KDE4
    return createKDE4Gui(_windowID, _scale, _doLoop, *_runResources);
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
    return createFBGui(_windowID, _scale, _doLoop, *_runResources);
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

    return std::auto_ptr<Gui>(new NullGui(_doLoop, *_runResources));
}

Player::~Player()
{
    if (_movieDef.get()) {
        log_debug("~Player - _movieDef refcount: %d (1 will be dropped "
                "now)", _movieDef->get_ref_count());
    }
}
