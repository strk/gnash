// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 
//

#ifndef _PLAYER_H_
#define _PLAYER_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#include "tu_config.h" // do we need this ??

#include "gnash.h"

#include <string>
#include <map>

// Forward declarations
namespace gnash
{
	class movie_interface;
}


namespace gnash
{

/// This class is an attempt at simplifying the code required
/// to simply start the flash player. The idea was to use it
/// from the plugin so we can set callback for getUrl and fs_commands
/// w/out the need of using FIFOs or sockets or whatever else.
///
class Player
{
public:

	Player();

	~Player() {}

	int run(int argc, char* argv[],
		const char* infile, const char* url=NULL);

	float setScale(float s);

	// milliseconds per frame
	void setDelay(unsigned int d) { delay=d; }

	void setWidth(size_t w) { width=w; }
	size_t getWidth() { return width; }

	void setHeight(size_t h) { height=h; }
	size_t getHeight() { return height; }

	void setWindowId(unsigned long x) { windowid=x; }

	void setDoLoop(bool b) { do_loop=b; }

	void setDoRender(bool b) { do_render=b; }

	void setDoSound(bool b) { do_sound=b; }

	float setExitTimeout(float n) {
		float old_timeout = exit_timeout;
		exit_timeout = n;
		return old_timeout;
	}

	int setBitDepth(int depth) {
		int old=bit_depth;
		bit_depth=depth;
		return old;
	}

	void setParam(std::string& name, std::string& value) {
		params[name] = value;
	}
	
private:
	static void setFlashVars(movie_interface& m, const std::string& varstr);

	static void fs_callback(movie_interface* movie,
			const char* command, const char* args);

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
};

 
} // end of gnash namespace

// end of _PLAYER_H_
#endif
