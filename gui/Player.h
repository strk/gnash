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

#ifndef _PLAYER_H_
#define _PLAYER_H_

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "tu_config.h"

#include "gnash.h" // still needed ?
#include "sound_handler.h" // for visibility of sound_handler destructor
#include "gui.h"

#include <string>
#include <map>
#include <iostream> // debugging...

// Forward declarations
namespace gnash
{
	class sprite_instance;
}


namespace gnash
{

/// This class is an attempt at simplifying the code required
/// to simply start the flash player. The idea was to use it
/// from the plugin so we can set callback for getUrl and fs_commands
/// w/out the need of using FIFOs or sockets or whatever else.
///
class DSOEXPORT Player
{
public:

	Player();

	~Player() {}

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
		const char* infile, const char* url=NULL);
#if 0
        int run(const char* infile);
#endif
	float setScale(float s);

	// milliseconds per frame
	void setDelay(unsigned int d) { delay=d; }

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
		_fpsDebugTime=time;
	}
#endif // def GNASH_FPS_DEBUG

	void setWidth(size_t w) { width=w; }
	size_t getWidth() { return width; }

	void setHeight(size_t h) { height=h; }
	size_t getHeight() { return height; }

	void setWindowId(unsigned long x) { windowid=x; }

	void setDoLoop(bool b) { do_loop=b; }

	void setDoRender(bool b) { do_render=b; }

	void setDoSound(bool b) { do_sound=b; }

	/// Set the base url for this run.
	//
	/// The base url will be used to resolve relative
	/// urls on load requests.
	///
	void setBaseUrl(const std::string& baseurl) {
		_baseurl = baseurl;
	}

	float setExitTimeout(float n) {
		float old_timeout = exit_timeout;
		exit_timeout = n;
		return old_timeout;
	}

//	int setBitDepth(int depth) {
//		int old=bit_depth;
//		bit_depth=depth;
//		return old;
//	}

	void setParam(std::string& name, std::string& value) {
		params[name] = value;
	}

	void setHostFD(int fd)
	{
		_hostfd = fd;
	}
	
private:

	void init();

	void init_sound();

	void init_logfile();

	void init_gui();

	/// Create the gui instance
	//
	/// Uses the USE_<guiname> macros to find out which one
	///
	std::auto_ptr<Gui> getGui();

	void setFlashVars(const std::string& varstr);

	static void fs_callback(sprite_instance* movie,
			const std::string& command, const std::string& args);

	static bool interfaceEventCallback(const std::string& event,
							const std::string& arg);

	// Movie parameters (for -P)
	std::map<std::string, std::string> params;

	unsigned int bit_depth;

	// the scale at which to play 
	float scale;

	unsigned int delay;

	size_t width;

	size_t height;

	unsigned long windowid;

	bool do_loop;

	bool do_render;

	bool do_sound;

	float exit_timeout;

	std::string _baseurl;

	static std::auto_ptr<Gui> _gui;

	std::auto_ptr<media::sound_handler> _sound_handler;

	std::string _url;

	std::string _infile;

	movie_definition* _movie_def;

	/// Load the "_infile" movie setting it's url to "_url"
	// 
	/// This function takes care of interpreting _infile as
	/// stdin when it equals "-". May throw a GnashException
	/// on failure.
	///
	movie_definition* load_movie();

#ifdef GNASH_FPS_DEBUG
	float _fpsDebugTime;
#endif

	// Filedescriptor to use for host application requests, -1 if none
	int _hostfd;

};

 
} // end of gnash namespace

// end of _PLAYER_H_
#endif
