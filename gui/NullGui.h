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

#ifndef NULLGUI_H
#define NULLGUI_H

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "gui.h"

namespace gnash
{

/// Null GUI, used when rendering is disabled
class NullGui : public Gui
{
public: 

	NullGui(bool do_loop, RunResources& r)
		:
		Gui(0, 0, do_loop, r),
		_timeout(0),
		_quit(false)
	{}

	~NullGui() {}

    void setInterval(unsigned int interval) {
		_interval=interval;
	}

	void setTimeout(unsigned int to) {
		_timeout=to;
	}

	bool init(int, char ***) { return true; }
	
    bool createWindow(const char* /*title*/, int /*width*/, int /*height*/,
	                  int /*yPosition*/, int /*xPosition*/) {
		return true;
	}

	bool run();
	bool createMenu() { return true; }
	bool setupEvents() { return true; }
	void renderBuffer() { }

	void quitUI() { _quit = true; }

private:

	/// timeout value, in milliseconds
	unsigned int _timeout;

	/// True when we quit
	bool _quit;
};

} // namespace gnash

#endif
