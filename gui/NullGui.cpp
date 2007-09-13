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

#include <boost/timer.hpp>
using boost::timer;

namespace gnash
{

bool
NullGui::run()
{
  timer prevtimer;
  timer start_timer;  // returns milliseconds

  prevtimer = start_timer;

  while (true)
  {
  
    timer timer;

    // synchronize to frame time 
    if (_timeout || (_interval>1))  // avoid timing completely for interval==1
    while (1) 
    {
        
      timer.restart();
            
      if (timer.elapsed_min() - prevtimer.elapsed() >= _interval)
        break; // next frame, please!
    
      if (timer.elapsed() < prevtimer.elapsed()) // time glitch protection
        prevtimer = timer;
        
      usleep(1);
      
    }
    
    if ( _timeout )
    {
      if ( timer.elapsed() - start_timer.elapsed() > _timeout)
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
