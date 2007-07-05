// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// 
//


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "NullGui.h"

#if defined(_WIN32) || defined(WIN32)
# include <windows.h>
# define usleep(x) Sleep(x/1000)
#else
# include <unistd.h> // for usleep
#endif

#include <boost/date_time/posix_time/posix_time_types.hpp>

using namespace boost::posix_time;

namespace gnash
{

bool
NullGui::run()
{
  uint64_t prevtimer=0;
  uint64_t start_timer = time_duration::ticks_per_second();  // returns milliseconds, maybe even ns...

  prevtimer = start_timer;

  while (true)
  {
  
    uint64_t timer=0;

    // synchronize to frame time 
    if (_timeout || (_interval>1))  // avoid timing completely for interval==1
    while (1) 
    {
        
      timer = time_duration::ticks_per_second();
            
      if (timer - prevtimer >= _interval)
        break; // next frame, please!
    
      if (timer < prevtimer) // time glitch protection
        prevtimer = timer;
        
      usleep(1);
      
    }
    
    if ( _timeout )
    {
      if ( timer - start_timer > _timeout)
      {
        break;
      }
    }
    
    
    prevtimer = timer;
          

    Gui::advance_movie(this);

    // when runnign gnash with -1 switch ::advance_movie() will call ::quit()
    // at last frame
    if ( _quit ) break;
  }
  return false;
}

} // end of gnash namespace
