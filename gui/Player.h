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

#ifndef GNASH_PLAYER_H
#define GNASH_PLAYER_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "HostInterface.h"              // for HostInterface, FsCallback, etc
#include "StringPredicates.h"           // for StringNoCaseLessThan
#include "movie_definition.h"
#include "NetworkAdapter.h"             // for setCookiesIn

#include <boost/intrusive_ptr.hpp>
#include <string>
#include <boost/shared_ptr.hpp>
#include <map>
#include <memory>

// Forward declarations
namespace gnash {
    class MovieClip;
    class RunResources;
    class Gui;
    namespace media {
        class MediaHandler;
    }
    namespace sound {
        class sound_handler;
    }
}

namespace gnash {

/// This class is an attempt at simplifying the code required
/// to simply start the SWF player. The idea was to use it
/// from the plugin so we can set callback for getUrl and fs_commands
/// w/out the need of using FIFOs or sockets or whatever else.
///
class Player
{
public:
    
    Player();
    
    ~Player();
    
    /// Play the movie at the given url/path.
    //
    /// @param argc
    ///        number of argument strings in argv
    ///
    /// @param argv
    ///        argument strings 
    ///
    /// @param url
    ///        an optional url to assign to the given movie.
    ///        if unspecified the url will be set to the 
    ///        movie path/url.
    ///           
    ///
    void run(int argc, char* argv[],
            const std::string& infile, const std::string& url = "");
    
    float setScale(float s);
    
    // milliseconds per frame
    void setDelay(unsigned int d) { _delay=d; }
    
#ifdef GNASH_FPS_DEBUG
    /// Set the number of seconds between FPS debugging prints
    //
    /// @param time
    ///        Number of seconds between FPS debugging prints.
    ///        A value of 0 disables FPS printing.
    ///        A negative value results in an assertion failure.
    ///
    void setFpsPrintTime(float time)
    {
        assert(time >= 0.0);
        _fpsDebugTime = time;
    }
#endif // def GNASH_FPS_DEBUG
    
    void setWidth(size_t w) { _width = w; }
    size_t getWidth() const { return _width; }
    
    void setHeight(size_t h) { _height = h; }
    size_t getHeight() const { return _height; }
    
    void setXPosition(int xPos) { _xPosition = xPos; }
    size_t getXPosition() const { return _xPosition; }
    
    void setYPosition(int yPos) { _yPosition = yPos; }
    size_t getYPosition() const { return _yPosition; }
    
    void setWindowId(unsigned long x) { _windowID = x; }
    
    void setDoLoop(bool b) { _doLoop = b; }
    
    void setDoRender(bool b) { _doRender = b; }
    
    void setDoSound(bool b) { _doSound = b; }
    
    void setMaxAdvances(unsigned long ul) { if (ul > 0) _maxAdvances = ul; }
    
    /// Set the base url for this run.
    //
    /// The base url will be used to resolve relative
    /// urls on load requests.
    ///
    void setBaseUrl(const std::string& baseurl) {
        _baseurl = baseurl;
    }
    
    float setExitTimeout(float n) {
        float oldtimeout = _exitTimeout;
        _exitTimeout = n;
        return oldtimeout;
    }
    
    void setParam(const std::string& name, const std::string& value) {
        _params[name] = value;
    }
    
    void setHostFD(int fd) {
        _hostfd = fd;
    }
    
    int getHostFD() const {
        return _hostfd;
    }

    void setMedia(const std::string& media) {
        _media = media;
    }

    void setControlFD(int fd) {
        _controlfd = fd;
    }
    
    int getControlFD() const {
        return _controlfd;
    }

    void setCookiesIn(const std::string& filename) {
        NetworkAdapter::setCookiesIn(filename);
    }

    void setStartFullscreen(bool x) {
        _startFullscreen = x;
    }
    
    void hideMenu(bool x) {
        _hideMenu = x;
    }
    
    void setAudioDumpfile(const std::string& filespec) {
        _audioDump = filespec;
    }
    
    /// Set the renderer backend, agg, opengl, or cairo. This is set
    /// in the users gnashrc file, or can be overridden with the
    /// --renderer option to gnash.
    void setRenderer(const std::string& x) { _renderer = x; }
    
    /// Set the hardware video accleration backend, none, vaapi.
    /// This is set in the users gnashrc file, or can be
    /// overridden with the --hwaccel option to gnash.
    void setHWAccel(const std::string& x) { _hwaccel = x; }

    /// This should be a comma-separated list of frames.
    //
    /// Only numbers and the word "last" are valid.
    //
    /// We parse the string here rather than in gnash.cpp to avoid making
    /// the interface to Player more complicated than it is already.
    void setScreenShots(const std::string& screenshots) {
        _screenshots = screenshots;
    }

    /// Set the filename for screenshot output.
    //
    /// A %f in the filename will be replaced with the frame number.
    void setScreenShotFile(const std::string& file) {
        _screenshotFile = file;
    }

    /// Set the quality for screenshot output.
    //
    /// A %f in the filename will be replaced with the frame number.
    void setScreenShotQuality(int quality) {
        _screenshotQuality = quality;
    }

private:

    /// Whether to ue HW video decoding support, no value means disabled.
    /// The only currently supported values are: none or vaapi.
    /// The default is none,
    std::string _hwaccel;
    
    /// Which renderer backend to use, no value means use the default.
    /// The currently supported values are agg, opengl, or cairo. AGG
    /// being the default.
    std::string _renderer;

    class CallbacksHandler : public HostInterface, public FsCallback
    {
    public:
        CallbacksHandler(Gui& gui, const Player& player)
            :
            _gui(gui),
            _player(player)
        {
        }
        
        boost::any call(const HostInterface::Message& e);

        void exit();
        
        // For handling notification callbacks from ActionScript.
        // The callback is always sent to a hosting application
        // (i.e. if a file descriptor is supplied). It is never
        // acted on by Gnash when running as a plugin.
        void notify(const std::string& event, const std::string& arg);
        
    private:
        
        Gui& _gui;
        
        const Player& _player;
    };
    
    std::auto_ptr<CallbacksHandler> _callbacksHandler;
    
    void init();
    
    /// This aux streamer returns a silent audio stream
    ///
    /// @param udata
    ///     Pointer to user-specific data
    /// @param stream
    ///     Buffer into which method will put samples
    /// @param len
    ///     Requested amount of samples to put
    /// @param atEOF
    ///     Will always set to false, silent stream never ends ..
    /// 
    /// @return always the len parameter value (silent stream never ends 
    ///         and is always available)
    ///
    static unsigned int silentStream(void* udata, boost::int16_t* stream,
                     unsigned int len, bool& atEOF);
    
    void init_sound();
    
    void init_logfile();
    
    void init_gui();
    
    /// Create the gui instance
    //
    /// Uses the USE_<guiname> macros to find out which one
    ///
    std::auto_ptr<Gui> getGui();
    
    void setFlashVars(const std::string& varstr);

    typedef std::map<std::string, std::string, StringNoCaseLessThan> Params;
    
    // Movie parameters (for -P)
    Params      _params;
    
    // the scale at which to play 
    float       _scale;
    unsigned int _delay;
    size_t      _width;
    size_t      _height;
    int         _xPosition;
    int         _yPosition;
    unsigned long _windowID;
    bool        _doLoop;
    bool        _doRender;
    bool        _doSound;
    float       _exitTimeout;
    std::string _baseurl;
    
    /// Initialization / destruction order is important here.
    //
    /// some sound_samples are destroyed in the dtor of SWFMovieDefinition,
    /// which is called by the Gui's dtor. This means that the RunResources
    /// and sound::sound_handler must still be alive. Initializing them
    /// later ensures that this is the case. It is still a good idea to
    /// initialize _gui after _runResources.
    //
    /// Moreover, _movieDef (the SWFMovieDefinition) would also prevent
    /// destruction of a SWFMovieDefinition if it is not initialized after
    /// _gui, and probably result in a segfault.
    //
    /// @todo   This is hairy, and the core should be sorted out so that
    ///         sound_sample knows about its sound::sound_handler without
    ///         needing a RunResources.
    boost::shared_ptr<sound::sound_handler> _soundHandler;
    
    boost::shared_ptr<media::MediaHandler> _mediaHandler;
    
    /// Handlers (for sound etc) for a libcore run.
    //
    /// This must be kept alive for the entire lifetime of the movie_root
    /// (currently: of the Gui).
    std::auto_ptr<RunResources> _runResources;
    
    /// This must be initialized after _runResources
    std::auto_ptr<Gui> _gui;
    
    std::string         _url;
    
    std::string         _infile;
    
    boost::intrusive_ptr<movie_definition> _movieDef;
    
    unsigned long       _maxAdvances;
    
    /// Load the "_infile" movie setting its url to "_url"
    // 
    /// This function takes care of interpreting _infile as
    /// stdin when it equals "-". May throw a GnashException
    /// on failure.
    ///
    boost::intrusive_ptr<movie_definition> load_movie();
    
#ifdef GNASH_FPS_DEBUG
    float       _fpsDebugTime;
#endif
    
    // Filedescriptor to use for host application requests, -1 if none
    int         _hostfd;
    
    int         _controlfd;

    // Whether to start Gnash in fullscreen mode.
    // (Or what did you think it meant?)
    bool        _startFullscreen;
    bool        _hideMenu;
    
    /// The filename to use for dumping audio.
    std::string _audioDump;
    
    /// A comma-separated list of frames to output as screenshots.
    //
    /// If empty, no screenshots are required.
    std::string _screenshots;
    
    /// The filename to save screenshots to.
    //
    /// If empty, a default is used.
    std::string _screenshotFile;
    
    /// The filename to save screenshots to.
    //
    /// By default 100.
    int _screenshotQuality;

    /// The identifier for the media handler.
    //
    /// If empty, a default is used.
    std::string _media;

};
 
} // end of gnash namespace

// end of _PLAYER_H_
#endif

// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
