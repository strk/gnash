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

/* $Id: timers.cpp,v 1.22 2007/02/09 13:38:50 strk Exp $ */

#include "timers.h"
#include "as_function.h" // for class as_function
#include "as_object.h" // for inheritance
#include "log.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "xml.h"
#include "VM.h"
#include "movie_root.h"

using namespace std;

namespace gnash {

  Timer::Timer() :
      _interval(0),
      _start(0),
      _object(0),
      _env(0)
  {
  }
  
  Timer::~Timer()
  {
    //log_msg("%s: \n", __FUNCTION__);
  }
  

  void
  Timer::setInterval(as_function& method, unsigned ms, as_object* this_ptr, as_environment *env)
  {
    _function = &method;
    _interval = ms * 1000; // transform to microseconds 
    //log_msg("_interval microseconds: %lu", _interval);
    _env = env;
    _object = this_ptr;
    start();
  }

  void
  Timer::clearInterval()
  {
    _interval = 0;
    _start = 0;
  }
  
  void
  Timer::start()
  {
	_start = tu_timer::get_profile_ticks();
	//log_msg("_start at seconds %lu", _start);
  }
  

bool
Timer::expired()
{
	if (_start)
	{
		uint64 now = tu_timer::get_profile_ticks();
		//log_msg("now: %lu", now);
		assert(now > _start);

		//printf("FIXME: %s: now is %f, start time is %f, interval is %f\n", __FUNCTION__, now, _start, _interval);
		if (now > _start + _interval)
		{
			_start = now; // reset the timer
			//log_msg("Timer expired! \n");
			return true;
		}
	}
	else
	{
		//log_msg("Timer not enabled!");
	}
	return false;
}

void
Timer::operator() ()
{
    //printf("FIXME: %s:\n", __FUNCTION__);
    //log_msg("INTERVAL ID is %d\n", getIntervalID());

    as_value timer_method(_function.get());
    as_value val = call_method(timer_method, _env, _object.get(), 0, 0);

}

// TODO: move to Global.cpp
void
timer_setinterval(const fn_call& fn)
{
	//log_msg("%s: args=%d", __FUNCTION__, fn.nargs);
    
	// Get interval function
	boost::intrusive_ptr<as_function> as_func = fn.arg(0).to_as_function();
	if ( ! as_func )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- first argument is not a function",
				ss.str().c_str());
		);
		return;
	}


	// Get interval time
	int ms = int(fn.arg(1).to_number());

	Timer timer;
	timer.setInterval(*as_func, ms, fn.this_ptr, fn.env);
    
	movie_root& root = VM::get().getRoot();
	int id = root.add_interval_timer(timer);
	fn.result->set_int(id);
}
  
// TODO: move to Global.cpp
void
timer_clearinterval(const fn_call& fn)
{
	//log_msg("%s: nargs = %d", __FUNCTION__, fn.nargs);

	int id = int(fn.arg(0).to_number());

	movie_root& root = VM::get().getRoot();
	bool ret = root.clear_interval_timer(id);
	fn.result->set_bool(ret);
}

} // namespace gnash
