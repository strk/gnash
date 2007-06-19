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

#include <sys/time.h> // for gettimeofday
#include <time.h> // for gettimeofday
#include <errno.h> // for reporting gettimeofday errors

namespace gnash
{

bool
NullGui::run()
{
	struct timeval tv;


	if (gettimeofday(&tv, NULL))
	{
		cerr << "Could not get time of day: " << strerror(errno) << endl;
		return false;
	}
	unsigned long int start_timer = tv.tv_sec*1000 + tv.tv_usec / 1000;

	while (true)
	{
		if (gettimeofday(&tv, NULL))
		{
			cerr << "Could not get time of day: " << strerror(errno) << endl;
			return false;
		}
		unsigned long int timer = tv.tv_sec*1000 + tv.tv_usec / 1000;
		if ( timer - start_timer > _timeout)
		{
			break;
		}

		// sleep for _interval milliseconds
		// TODO: use the timer value to trigger advances
		usleep(_interval*1000);
		Gui::advance_movie(this);
	}
	return false;
}

} // end of gnash namespace
