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

/* $Id: timers.cpp,v 1.21 2007/02/09 00:19:07 strk Exp $ */

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
      _which(0),
      _interval(0.0),
      _start(0.0),
      _object(0),
      _env(0)
  {
  }
  
  Timer::Timer(as_value *obj, int ms)
  {
    setInterval(*obj, ms);
    start();
  }
  
  Timer::~Timer()
  {
    log_msg("%s: \n", __FUNCTION__);
  }
  
  int
  Timer::setInterval(as_value obj, int ms)
  {
    _function = obj;
    _interval = ms * 0.01;
    // _interval = ms * 0.000001;
    start();

    return 0;
  }

  int
  Timer::setInterval(as_value obj, int ms, as_environment *en)
  {
    _function = obj;
    _interval = ms * 0.01;
    _env = en;
    // _interval = ms * 0.000001;
    start();

    return 0;
  }
  int
  Timer::setInterval(as_value obj, int ms, std::vector<variable *> *locals)
  {
    _function = obj;
    _interval = ms * 0.01;
    _locals = locals;
    // _interval = ms * 0.000001;
    start();

    return 0;
  }

  int
  Timer::setInterval(as_value obj, int ms, as_object *this_ptr, as_environment *en)
  {
    _function = obj;
    _interval = ms * 0.01;
    _env = en;
    _object = this_ptr;
    // _interval = ms * 0.000001;
    start();

    return 0;
  }

  void
  Timer::clearInterval()
  {
    _interval = 0.0;
    _start = 0.0;
  }
  
  void
  Timer::start()
  {
    uint64 ticks = tu_timer::get_profile_ticks();
    _start = tu_timer::profile_ticks_to_seconds(ticks);
  }
  

  bool
  Timer::expired()
  {
    if (_start > 0.0) {
      uint64 ticks = tu_timer::get_profile_ticks();
      double now = tu_timer::profile_ticks_to_seconds(ticks);
      //printf("FIXME: %s: now is %f, start time is %f, interval is %f\n", __FUNCTION__, now, _start, _interval);
      if (now > _start + _interval) {
        _start = now;               // reset the timer
        //log_msg("Timer expired! \n");
        return true;
      }
      // FIXME: Sometimes, "now" and "_start" have bad values.
      // I don't know why, but this works around the problem..
      else if (now < _start) {
        log_msg( "Timer::expired - now (%f) is before start (%f)!\n"
                 "     Expiring right now.\n",
                 now, _start);
        _start = now;
        return true;
      }
    }
      
    // log_msg("Timer not enabled! \n");
    return false;
  }

void
Timer::operator() ()
{
    //printf("FIXME: %s:\n", __FUNCTION__);
    //log_msg("INTERVAL ID is %d\n", getIntervalID());

    const as_value& timer_method = getASFunction();
    as_environment* as_env = getASEnvironment();
		
    as_object* obj = getObject();
    as_value val = call_method(timer_method, as_env, obj, 0, 0);

    //as_object* this_ptr = getASObject();
    //as_value val = call_method(timer_method, as_env, this_ptr, 0, 0);

}

void
timer_setinterval(const fn_call& fn)
{
	log_msg("%s: args=%d\n", __FUNCTION__, fn.nargs);
    
	timer_as_object *ptr = new timer_as_object;
    
	// Get interval function
	as_function *as_func = fn.arg(0).to_as_function();
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

	fn.env->add_frame_barrier();
	//method = env->get_variable("loopvar");

#if 0
    // FIXME: This is pretty gross, but something is broke elsewhere and it doesn't
    // seem to effect anything else. When a function is called from a executing
    // function, like calling setInterval() from within the callback to
    // XMLSocket::onConnect(), the local variables of the parent function need to
    // be propogated to the local stack as regular variables (not locals) or
    // they can't be found in the scope of the executing chld function. There is
    // probably a better way to do this... but at least this works.
    for (i=0; i< fn.env->get_local_frame_top(); i++) {
      if (fn.env->m_local_frames[i].m_name.size()) {
        //method = env->get_variable(env->m_local_frames[i].m_name);
        //if (method.get_type() != as_value::UNDEFINED)
        {
          string local_name  = fn.env->m_local_frames[i].m_name;
          as_value local_val = fn.env->m_local_frames[i].m_value;
          fn.env->set_variable(local_name, local_val);
        }
      }
    }
#endif

	as_value val(as_func);

	//Ptr->obj.setInterval(val, ms);
	ptr->obj.setInterval(val, ms, ptr, fn.env);
    
	movie_root& root = VM::get().getRoot();
	int id = root.add_interval_timer(ptr->obj);
	fn.result->set_int(id);
}
  
void
timer_clearinterval(const fn_call& fn)
{
	log_msg("%s: nargs = %d\n", __FUNCTION__, fn.nargs);

	int id = int(fn.arg(0).to_number());

	movie_root& root = VM::get().getRoot();
	bool ret = root.clear_interval_timer(id);
	fn.result->set_bool(ret);
}
}
