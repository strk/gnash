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
//
// 
//


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "NullGui.h"

#if defined(_WIN32) || defined(WIN32)
#	include <windows.h>
# define usleep(x) Sleep(x/1000)
#else
# include <unistd.h> // for usleep
#endif

namespace gnash
{

bool
NullGui::run()
{
		while (true)
		{
			// sleep for _interval milliseconds
			usleep(_interval*1000);
			Gui::advance_movie(this);
		}
		return false;
}

} // end of gnash namespace
