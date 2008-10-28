// Player.cpp:  Top level SWF player, for gnash.
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

#ifndef DEFAULT_GUI
# define DEFAULT_GUI "NULL"
#endif

#include "gui.h"
#include "NullGui.h"

#include "gnash.h" // still needed ?
#include "movie_definition.h"
#include "sound_handler.h" // for set_sound_handler and create_sound_handler_*
#include "MovieClip.h" // for setting FlashVars
#include "movie_root.h" 
#include "Player.h"

#include "StringPredicates.h"
#include "URL.h"
#include "rc.h"
#include "GnashException.h"
#include "noseek_fd_adapter.h"
#include "VM.h"
#include "SystemClock.h"

#ifdef USE_FFMPEG
# include "MediaHandlerFfmpeg.h"
#elif defined(USE_GST)
# include "MediaHandlerGst.h"
#endif


#include "log.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <unistd.h> // for write() on BSD

using namespace gnash;

namespace {
gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
}

/// @todo Shouldn't Player be in 'gnash' namespace ?

/*static private*/
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
#if defined(RENDERER_CAIRO)
    _bitDepth(32),
#else
    _bitDepth(16),
#endif
    _scale(1.0f),
    _delay(0),
    _width(0),
    _height(0),
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
    _startFullscreen(false)
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

bool
Player::silentStream(void* /*udata*/, boost::uint8_t* stream, int len)
{
    memset((void*)stream, 0, len);
    return true;
}

void
Player::init_sound()
{

    if (_doSound) {
#ifdef SOUND_SDL
        _soundHandler.reset(sound::create_sound_handler_sdl(_audioDump));
        if (! _audioDump.empty()) {
            // add a silent stream to the audio pool so that our output file
            // is homogenous;  we actually want silent wave data when no sounds
            // are playing on the stage
            _soundHandler->attach_aux_streamer(silentStream, (void*) this);
        }
#elif defined(SOUND_GST)
        _soundHandler.reset(media::create_sound_handler_gst());
#else
        log_error(_("Sound requested but no sound support compiled in"));
        return;
#endif
    }
}

void
Player::init_media()
{
#ifdef USE_FFMPEG
        _mediaHandler.reset( new gnash::media::ffmpeg::MediaHandlerFfmpeg() );
#elif defined(USE_GST)
        _mediaHandler.reset( new gnash::media::gst::MediaHandlerGst() );
#else
        log_error(_("No media support compiled in"));
        return;
#endif
        
        gnash::media::MediaHandler::set(_mediaHandler);
}


void
Player::init_gui()
{
    if ( _doRender )
    {
        _gui = getGui(); 

        RcInitFile& rcfile = RcInitFile::getDefaultInstance();
        if ( rcfile.startStopped() )
        {
            _gui->stop();
        }

    }
    else
    {
        _gui.reset(new NullGui(_doLoop));
    }

    _gui->setMaxAdvances(_maxAdvances);

#ifdef GNASH_FPS_DEBUG
    if ( _fpsDebugTime )
    {
        log_debug(_("Activating FPS debugging every %g seconds"), _fpsDebugTime);
        _gui->setFpsTimerInterval(_fpsDebugTime);
    }
#endif // def GNASH_FPS_DEBUG
}

boost::intrusive_ptr<movie_definition>
Player::load_movie()
{
    /// The RunInfo must be initialized by this point to provide resources
    /// for parsing.
    assert(_runInfo.get());

    boost::intrusive_ptr<gnash::movie_definition> md;

    RcInitFile& rcfile = RcInitFile::getDefaultInstance();
    URL vurl(_url);

    if ( vurl.protocol() == "file" )
    {
        const std::string& path = vurl.path();
        size_t lastSlash = path.find_last_of('/');
        std::string dir = path.substr(0, lastSlash+1);
        rcfile.addLocalSandboxPath(dir);
        log_debug(_("%s appended to local sandboxes"), dir.c_str());
    }

    try {
        if ( _infile == "-" )
        {
            std::auto_ptr<IOChannel> in (
                    noseek_fd_adapter::make_stream(fileno(stdin)));
            md = gnash::create_movie(in, _url, *_runInfo, false);
        }
        else
        {
            URL url(_infile);
            if ( url.protocol() == "file" )
            {
                std::string path = url.path();
                // We'll need to allow load of the file, no matter virtual url
                // specified...
                // This is kind of hackish, cleaner would be adding an argument
                // to create_library_movie to skip the security checking phase.
                // NOTE that if we fail to allow this load, the konqueror plugin
                // would not be able to load anything
                //
                rcfile.addLocalSandboxPath(path);
                log_debug(_("%s appended to local sandboxes"), path.c_str());
            }

            // _url should be always set at this point...
            md = gnash::create_library_movie(url, *_runInfo, _url.c_str(),
                    false);
        }
    } catch (const GnashException& er) {
        std::cerr << er.what() << std::endl;
        md = NULL;
    }

    if ( ! md )
    {
        fprintf(stderr, "Could not load movie '%s'\n", _infile.c_str());
        return NULL;
    }

    return md;
}

/* \brief Run, used to open a new flash file. Using previous initialization */
int
Player::run(int argc, char* argv[], const std::string& infile, const std::string& url)
{
    
    // Call this at run() time, so the caller has
    // a cache of setting some parameter before calling us...
    // (example: setDoSound(), setWindowId() etc.. ) 
    init_logfile();
    init_media();
    init_sound();
    init_gui();
   
    // gnash.cpp should check that a filename is supplied.
    assert (!infile.empty());

    _infile = infile;

    // Work out base url
    if ( _baseurl.empty() )
    {
        if (! url.empty() ) _baseurl = url;
        else if ( infile == "-" ) _baseurl = URL("./").str();
        else _baseurl = infile;
    }

    // Set _root._url (either explicit of from infile)
    if (! url.empty() ) {
        _url = url;
    }  else {
        _url = infile;
    }


    // Initialize gui (we need argc/argv for this)
    // note that this will also initialize the renderer
    // which is *required* during movie loading
    if ( ! _gui->init(argc, &argv) )
    {
        std::cerr << "Could not initialize gui." << std::endl;
        return EXIT_FAILURE;
    }

    // Parse querystring (before FlashVars, see testsuite/misc-ming.all/FlashVarsTest*)
    setFlashVars(URL(_url).querystring());

    // These flags are here so we can construct
    // the correct URL for base url later.
    // If the URL class was not immutable we could do something smarter...
    bool hasOverriddenBaseUrl=false;
    std::string overriddenBaseUrl;

    // Parse parameters
    StringNoCaseEqual noCaseCompare;
    for ( std::map<std::string,std::string>::const_iterator it=params.begin(),
        itEnd=params.end(); it != itEnd; ++it)
    {
        if ( noCaseCompare(it->first, "flashvars") )
        {
            setFlashVars(it->second);
            continue;
        }

        if ( noCaseCompare(it->first, "base") )
        {
            hasOverriddenBaseUrl=true;
            overriddenBaseUrl=it->second;
            continue;
        }
    }

    URL baseURL = hasOverriddenBaseUrl ? URL(overriddenBaseUrl, URL(_baseurl))
                                       : URL(_baseurl);

    /// The RunInfo should be populated before parsing.
    _runInfo.reset(new RunInfo(baseURL.str()));
    _runInfo->setSoundHandler(_soundHandler.get());

    // Load the actual movie.
    _movieDef = load_movie();
    if ( ! _movieDef )
    {
        return EXIT_FAILURE;
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

    if ( ! _width || ! _height )
    {
        log_debug(_("Input movie has collapsed dimensions "
                    "%d/%d. Setting to 1/1 and going on."),
                     _width, _height);
        if ( ! _width ) _width = 1;
        if ( ! _height ) _height = 1;
    }

    // Now that we know about movie size, create gui window.
    _gui->createWindow(_url.c_str(), _width, _height);

    SystemClock clock; // use system clock here...
    movie_root root(*_movieDef, clock, *_runInfo);

    _callbacksHandler.reset(new CallbacksHandler(_gui.get())); 
    
    // Register Player to receive events from the core (Mouse, Stage,
    // System etc)
    root.registerEventCallback(_callbacksHandler.get());
    
    // Register Player to receive FsCommand events from the core.
    root.registerFSCommandCallback(_callbacksHandler.get());

    // Set host requests fd (if any)
    if ( _hostfd != -1 ) root.setHostFD(_hostfd);

    _gui->setStage(&root);

    // Start loader thread
    // NOTE: the loader thread might (in IMPORT tag parsing)
    //       create new movies and register them to the MovieLibrary.
    //       If MovieLibrary size exceeded, _movieDef might be
    //       destroyed prematurely. movie_root might actually be
    //       keeping it alive, as Gui might as well, but why relying
    //       on luck ? So we made sure to keep _movieDef by 
    //       intrusive_ptr...
    _movieDef->completeLoad(*_runInfo);

    _gui->setMovieDefinition(_movieDef.get());

    if (! _delay) {
      //float movie_fps = _movieDef->get_frame_rate();
      //_delay = static_cast<unsigned int>(1000 / movie_fps) ; // milliseconds per frame
      _delay = 10; // 10ms per heart beat
    }
    _gui->setInterval(_delay);

    if (_exitTimeout) {
      _gui->setTimeout(static_cast<unsigned int>(_exitTimeout * 1000));
    }

    if (!_windowID && _startFullscreen) {
        _gui->setFullscreen();
    }
    _gui->run();

    log_debug("Main loop ended, cleaning up");

    // Clean up as much as possible, so valgrind will help find actual leaks.
    gnash::clear();

    return EXIT_SUCCESS;
}

bool
Player::CallbacksHandler::yesNo(const std::string& query)
{
    return _gui->yesno(query);
}

std::string
Player::CallbacksHandler::call(const std::string& event, const std::string& arg)
{
    if (event == "Mouse.hide")
    {
        return _gui->showMouse(false) ? "true" : "false";
    }

    if (event == "Mouse.show")
    {
        return _gui->showMouse(true) ? "true" : "false";
    }
    
    if (event == "Stage.displayState")
    {
        if (arg == "fullScreen") _gui->setFullscreen();
        else if (arg == "normal") _gui->unsetFullscreen();
        return "";
    }

    if (event == "Stage.scaleMode" || event == "Stage.align" )
    {
        _gui->updateStageMatrix();
        return "";
    }
    
    if (event == "System.capabilities.screenResolutionX")
    {
        std::ostringstream ss;
        ss << _gui->getScreenResX();
        return ss.str();
    }

    if (event == "System.capabilities.screenResolutionY")
    {
        std::ostringstream ss;
        ss << _gui->getScreenResY();
        return ss.str();
    }

    if (event == "System.capabilities.pixelAspectRatio")
    {
        std::ostringstream ss;
        // Whether the pp actively limits the precision or simply
        // gets a slightly different result isn't clear.
        ss << std::setprecision(7) << _gui->getPixelAspectRatio();
        return ss.str();
    }

    if (event == "System.capabilities.screenDPI")
    {
        std::ostringstream ss;
        ss << _gui->getScreenDPI();
        return ss.str();
    }

    if (event == "System.capabilities.screenColor")
    {
        return _gui->getScreenColor();
    }

    if (event == "System.capabilities.playerType")
    {
        return _gui->isPlugin() ? "PlugIn" : "StandAlone";
    }

    log_error(_("Unhandled callback %s with arguments %s"), event, arg);
    return "";
}

void
Player::CallbacksHandler::notify(const std::string& command, const std::string& args)
{
    //log_debug(_("fs_callback(%p): %s %s"), (void*)movie, command, args);

    gnash::RcInitFile& rcfile = gnash::RcInitFile::getDefaultInstance();

    // it's _hostfd, but we're a static method...
    int hostfd = VM::get().getRoot().getHostFD();
    if ( hostfd != -1 )
    {
        //log_debug("user-provided host requests fd is %d", hostfd);
        std::stringstream request;
        request << "INVOKE " << command << ":" << args << std::endl;

        std::string requestString = request.str();
        const char* cmd = requestString.c_str();
        size_t len = requestString.length();
        // TODO: should mutex-protect this ?
        // NOTE: we assuming the hostfd is set in blocking mode here..
        //log_debug("Attempt to write INVOKE requests fd %d", hostfd);
        int ret = write(hostfd, cmd, len);
        if ( ret == -1 )
        {
            log_error("Could not write to user-provided host "
                      "requests fd %d: %s", hostfd, strerror(errno));
        }
        if ( static_cast<size_t>(ret) < len )
        {
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
    if (_gui->isPlugin())
    {
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
    if (noCaseCompare(command, "quit"))
    {
        _gui->quit();
        return;
    }

    // FSCommand fullscreen
    if (noCaseCompare(command, "fullscreen"))
    {
        if (noCaseCompare(args, "true")) _gui->setFullscreen();
        else if (noCaseCompare(args, "false")) _gui->unsetFullscreen();
        return;
    }
       
    // FSCommand showmenu
    if (noCaseCompare(command, "showmenu"))
    {
        if (noCaseCompare(args, "true")) _gui->showMenu(true);
        else if (noCaseCompare(args, "false")) _gui->showMenu(false);
        return;
    }

    // FSCommand exec
    // Note: the pp insists that the file to execute should be in 
    // a subdirectory 'fscommand' of the 'projector' executable's
    // location. In SWF5 there were no restrictions.
    if (noCaseCompare(command, "exec"))
    {
        log_unimpl(_("FsCommand exec called with argument %s"), args);
        return;
    }

    // FSCommand allowscale
    if (noCaseCompare(command, "allowscale"))
    {
        //log_debug("allowscale: %s", args);
        if (noCaseCompare(args, "true")) _gui->allowScale(true);
        else
        {
                if (strtol(args.c_str(), NULL, 0)) _gui->allowScale(true);
                else _gui->allowScale(false);
        }
        return;
    }

    // FSCommand trapallkeys
    if (noCaseCompare(command, "trapallkeys"))
    {
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
    return createGTKGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_KDE
    return createKDEGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_SDL
    return createSDLGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_AQUA
    return createAQUAGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_RISCOS
    return createRISCOSGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_FLTK
    return createFLTKGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_FB
    return createFBGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

#ifdef GUI_DUMP
    return createDumpGui(_windowID, _scale, _doLoop, _bitDepth);
#endif

    return std::auto_ptr<Gui>(new NullGui(_doLoop));
}

Player::~Player()
{
    if (_movieDef.get())
    {
            log_debug("~Player - _movieDef refcount: %d (1 will be dropped now)", _movieDef->get_ref_count());
    }
}
