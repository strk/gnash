// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// 
//

#ifndef _NULLGUI_H_
#define _NULLGUI_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tu_config.h"
#include "gui.h"

namespace gnash
{

/// Null GUI, used when rendering is disabled
class DSOEXPORT NullGui : public Gui {

public: 

	NullGui(bool do_loop)
		:
		Gui(0,0,do_loop,0),
		_timeout(0)
	{}

	~NullGui() {}
	void setInterval(unsigned int interval)
	{
		_interval=interval;
	}
	void setTimeout(unsigned int to)
	{
		_timeout=to;
	}
	bool init(int, char ***) { return true; }
	bool createWindow(const char* /*title*/, int /*width*/, int /*height*/)
	{
		return true;
	}
	bool run();
	bool createMenu()  { return true; }
	bool setupEvents()  { return true; }
	void renderBuffer()  { }

private:

	/// timeout value, in milliseconds
	unsigned int _timeout;
};

} // end of gnash namespace

// end of _NULLGUI_H_
#endif
