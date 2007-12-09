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

/* $Id: timers.cpp,v 1.41 2007/12/09 20:40:49 strk Exp $ */

#include "timers.h"
#include "as_function.h" // for class as_function
#include "as_object.h" // for inheritance
#include "log.h"
#include "sprite_instance.h"
#include "fn_call.h"
#include "xml.h"
#include "VM.h"
#include "movie_root.h"

#include <limits> // for numeric_limits

using namespace std;

namespace gnash {

  Timer::Timer() :
      _interval(0),
      _start(std::numeric_limits<unsigned long>::max()),
      _object(0)
  {
  }
  
  Timer::~Timer()
  {
    //log_msg("%s: \n", __FUNCTION__);
  }
  

  void
  Timer::setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr)
  {
    _function = &method;
    _interval = ms; // keep milliseconds
    //log_msg("_interval milliseconds: %lu", _interval);
    _object = this_ptr;
    start();
  }

  void
  Timer::setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr, 
		  std::vector<as_value>& args)
  {
    _function = &method;
    _interval = ms; // keep as milliseconds
    //log_msg("_interval milliseconds: %llu", _interval);
    _object = this_ptr;
    _args = args;
    start();
  }

  void
  Timer::clearInterval()
  {
    _interval = 0;
    _start = std::numeric_limits<unsigned long>::max();
  }
  
  void
  Timer::start()
  {
	_start = VM::get().getTime();
	//log_msg("_start at seconds %lu", _start);
  }
  

bool
Timer::expired()
{
	if ( _start != std::numeric_limits<unsigned long>::max() )
	{
		unsigned long now = VM::get().getTime();
		assert(now >= _start); // it is possible for now to be == _start 

		//cout << "Start is " << _start << " interval is " << _interval << " now is " << now << endl;
		if (now > _start + _interval)
		{
			_start = now; // reset the timer
			//cout << " Expired, reset start to " << _start << endl;
			//log_msg("Timer expired! \n");
			return true;
		}
	}
	else
	{
		log_msg("Timer not enabled!");
	}

	return false;
}

void
Timer::operator() ()
{
    //printf("FIXME: %s:\n", __FUNCTION__);
    //log_msg("INTERVAL ID is %d\n", getIntervalID());

    as_value timer_method(_function.get());

    as_environment env;

    // Push args to the as_environment stack if needed
    for ( ArgsContainer::reverse_iterator it=_args.rbegin(), itEnd=_args.rend();
		    it != itEnd; ++it )
    {
	    //log_msg("Env-pushing %s", it->to_debug_string().c_str());
	    env.push(*it);
    }

    size_t firstArgBottomIndex = env.stack_size()-1; 

    as_value val = call_method(timer_method, &env, _object.get(),
		    _args.size(), firstArgBottomIndex);

}

#ifdef GNASH_USE_GC
void
Timer::markReachableResources() const
{
	for (ArgsContainer::const_iterator i=_args.begin(), e=_args.end();
			i!=e; ++i)
	{
		i->setReachable();
	}

	if ( _function ) _function->setReachable();
	if ( _object ) _object->setReachable();
}
#endif // GNASH_USE_GC

// TODO: move to Global.cpp
as_value
timer_setinterval(const fn_call& fn)
{
	//log_msg("%s: args=%d", __FUNCTION__, fn.nargs);
	// TODO: support setInterval(object, propertyname, intervaltime, arguments...) too
    
	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- need at least 2 arguments",
				ss.str().c_str());
		);
		return as_value();
	}

	unsigned timer_arg = 1;

	boost::intrusive_ptr<as_object> obj = fn.arg(0).to_object();
	if ( ! obj )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- first argument is not an object or function",
				ss.str().c_str());
		);
		return as_value();
	}

	// Get interval function
	boost::intrusive_ptr<as_function> as_func = obj->to_function(); 
	if ( ! as_func )
	{
		as_value method;
		const std::string& method_name = fn.arg(1).to_string();
		if (!obj->get_member(VM::get().getStringTable().find(method_name), &method) )
		{
			IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Invalid call to setInterval(%s) "
					"- can't find member %s of object %s",
					ss.str().c_str(), method_name.c_str(),
					fn.arg(0).to_debug_string().c_str());
			);
			return as_value();
		}
		as_func = method.to_as_function();
		if ( ! as_func )
		{
			IF_VERBOSE_ASCODING_ERRORS(
				std::stringstream ss; fn.dump_args(ss);
				log_aserror("Invalid call to setInterval(%s) "
					"- %s.%s is not a function",
					ss.str().c_str(),
					fn.arg(0).to_debug_string().c_str(),
					method_name.c_str());
			);
			return as_value();
		}

		timer_arg = 2;
	}


	if ( fn.nargs < timer_arg+1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- missing timeout argument",
				ss.str().c_str());
		);
		return as_value();
	}

	// Get interval time
	unsigned long ms = static_cast<unsigned long>(fn.arg(timer_arg).to_number());

	// Parse arguments 
	Timer::ArgsContainer args;
	for (unsigned i=timer_arg+1; i<fn.nargs; ++i)
	{
		args.push_back(fn.arg(i));
	}

	std::auto_ptr<Timer> timer(new Timer);
	timer->setInterval(*as_func, ms, fn.this_ptr, args);
    
	movie_root& root = VM::get().getRoot();
	int id = root.add_interval_timer(timer);
	return as_value(id);
}
  
// TODO: move to Global.cpp
as_value
timer_clearinterval(const fn_call& fn)
{
	//log_msg("%s: nargs = %d", __FUNCTION__, fn.nargs);

	int id = int(fn.arg(0).to_number());

	movie_root& root = VM::get().getRoot();
	bool ret = root.clear_interval_timer(id);
	return as_value(ret);
}

} // namespace gnash
