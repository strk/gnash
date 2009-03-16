// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#include "gnash.h" // needed for interface and fscommand callbacks
#include "sound_handler.h" // for visibility of sound_handler destructor
#include "MediaHandler.h" // for visibility of MediaHandler destructor
#include "gui.h"
#include "movie_definition.h" // for visibility of movie_definition destructor
#include "smart_ptr.h" // for intrusive_ptr holding of top-level movie
#include "movie_root.h" // for Abstract callbacks
#include "RunInfo.h" // for passing handlers and other data to the core.

#include <string>
#include <boost/shared_ptr.hpp>
#include <map>

// Forward declarations
namespace gnash
{
	class MovieClip;
}


namespace gnash
{

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
	///	number of argument strings in argv
	///
	/// @param argv
	///	argument strings 
	///
	/// @param url
	///	an optional url to assign to the given movie.
	///	if unspecified the url will be set to the 
	///	movie path/url.
	///           
	///
	int run(int argc, char* argv[],
		const std::string& infile, const std::string& url = "");

	float setScale(float s);

	// milliseconds per frame
	void setDelay(unsigned int d) { _delay=d; }

#ifdef GNASH_FPS_DEBUG
	/// Set the number of seconds between FPS debugging prints
	//
	/// @param time
	///	Number of seconds between FPS debugging prints.
	///	A value of 0 disables FPS printing.
	///	A negative value results in an assertion failure.
	///
	void setFpsPrintTime(float time)
	{
		assert(time >= 0.0);
		_fpsDebugTime = time;
	}
#endif // def GNASH_FPS_DEBUG

	void setWidth(size_t w) { _width = w; }
	size_t getWidth() { return _width; }

	void setHeight(size_t h) { _height = h; }
	size_t getHeight() { return _height; }

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

	void setParam(std::string& name, std::string& value) {
		params[name] = value;
	}

	void setHostFD(int fd) {
		_hostfd = fd;
	}
	
    int getHostFD() const {
        return _hostfd;
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
	
private:

	class CallbacksHandler : public movie_root::AbstractIfaceCallback,
		                     public movie_root::AbstractFsCallback
	{
	public:
		CallbacksHandler(Gui& gui, const Player& player)
			:
			_gui(gui),
            _player(player)
		{}

		std::string call(const std::string& event,
				const std::string& arg);

		bool yesNo(const std::string& query);

        void error(const std::string& msg);

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

	void init_media();

	void init_logfile();

	void init_gui();

	/// Create the gui instance
	//
	/// Uses the USE_<guiname> macros to find out which one
	///
	std::auto_ptr<Gui> getGui();

	void setFlashVars(const std::string& varstr);

	// Movie parameters (for -P)
	std::map<std::string, std::string> params;

	unsigned int _bitDepth;

	// the scale at which to play 
	float _scale;

	unsigned int _delay;

	size_t _width;

	size_t _height;

	unsigned long _windowID;

	bool _doLoop;

	bool _doRender;

	bool _doSound;

	float _exitTimeout;

	std::string _baseurl;


    /// Initialization / destruction order is important here.
    //
    /// some sound_samples are destroyed in the dtor of SWFMovieDefinition,
    /// which is called by the Gui's dtor. This means that the RunInfo
    /// and sound::sound_handler must still be alive. Initializing them
    /// later ensures that this is the case. It is still a good idea to
    /// initialize _gui after _runInfo.
    //
    /// Moreover, _movieDef (the SWFMovieDefinition) would also prevent
    /// destruction of a SWFMovieDefinition if it is not initialized after
    /// _gui, and probably result in a segfault.
    //
    /// @todo   This is hairy, and the core should be sorted out so that
    ///         sound_sample knows about its sound::sound_handler without
    ///         needing a RunInfo.
    boost::shared_ptr<sound::sound_handler> _soundHandler;

	std::auto_ptr<media::MediaHandler> _mediaHandler;

    /// Handlers (for sound etc) for a libcore run.
    //
    /// This must be kept alive for the entire lifetime of the movie_root
    /// (currently: of the Gui).
    std::auto_ptr<RunInfo> _runInfo;

    /// This must be initialized after _runInfo
	std::auto_ptr<Gui> _gui;

    std::string _url;

	std::string _infile;

	boost::intrusive_ptr<movie_definition> _movieDef;
	
	unsigned long _maxAdvances;

	/// Load the "_infile" movie setting its url to "_url"
	// 
	/// This function takes care of interpreting _infile as
	/// stdin when it equals "-". May throw a GnashException
	/// on failure.
	///
	boost::intrusive_ptr<movie_definition> load_movie();

#ifdef GNASH_FPS_DEBUG
	float _fpsDebugTime;
#endif

	// Filedescriptor to use for host application requests, -1 if none
	int _hostfd;
	
	// Whether to start Gnash in fullscreen mode.
	// (Or what did you think it meant?)
	bool _startFullscreen;
	bool _hideMenu;

    // The filename to use for dumping audio.
    std::string _audioDump;

};

 
} // end of gnash namespace

// end of _PLAYER_H_
#endif
