// 
//   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

namespace gnash {

  Timer::Timer() :
      _interval(0),
      _start(std::numeric_limits<unsigned long>::max()),
      _object(0),
      _runOnce(false)
  {
  }
  
  Timer::~Timer()
  {
    //log_debug("%s: \n", __FUNCTION__);
  }

  void
  Timer::setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr, 
		  ArgsContainer& args, bool runOnce)
  {
    _function = &method;
    _interval = ms; // keep as milliseconds
    //log_debug("_interval milliseconds: %llu", _interval);
    _object = this_ptr;
    _args = args;
    _runOnce = runOnce;
    start();
  }

  void
  Timer::setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr, bool runOnce)
  {
    _function = &method;
    _interval = ms; // keep as milliseconds
    //log_debug("_interval milliseconds: %llu", _interval);
    _object = this_ptr;
    _runOnce = runOnce;
    start();
  }

  void
  Timer::setInterval(boost::intrusive_ptr<as_object> this_ptr, const std::string& methodName, unsigned long ms, 
		  std::vector<as_value>& args, bool runOnce)
  {
    _object = this_ptr;
    _methodName = methodName;
    _interval = ms; // keep as milliseconds
    //log_debug("_interval milliseconds: %llu", _interval);
    _args = args;
    _runOnce = runOnce;
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
	//log_debug("_start at seconds %lu", _start);
  }
  

bool
Timer::expired(unsigned long now, unsigned long& elapsed)
{
	if ( cleared() ) return false;
	long unsigned expTime = _start + _interval;
	if ( now < expTime ) return false;
	elapsed = expTime-now;
	return true;
}

void
Timer::executeAndReset()
{
	if ( cleared() ) return;
	execute();
	if ( _runOnce ) clearInterval();
	else _start += _interval; // reset the timer
}

void
Timer::execute()
{
    //printf("FIXME: %s:\n", __FUNCTION__);
    //log_debug("INTERVAL ID is %d\n", getIntervalID());

    as_value timer_method;

    as_object* super = _object->get_super(_function ? 0 : _methodName.c_str());

    if ( _function.get() )
    {
        timer_method.set_as_function(_function.get());
    }
    else
    {
        VM& vm = _object->getVM();
        string_table::key k = vm.getStringTable().find(_methodName);
        as_value tmp;
        if ( ! _object->get_member(k, &tmp) )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("object %p has no member named %s (interval method)",
                 _object, _methodName);
            );
            return;
        }
        as_function* f = tmp.to_as_function();
        if ( ! f )
        {
            IF_VERBOSE_ASCODING_ERRORS(
            log_aserror("member %s of object %p (interval method) is not a function (%s)",
                 _methodName, (void*)_object.get(), tmp);
            );
            return;
        }
        timer_method.set_as_function(f);
    }

    as_environment env;

    // Prepare args 
    std::auto_ptr< std::vector<as_value> > args ( new std::vector<as_value> );
    for ( ArgsContainer::iterator it=_args.begin(), itEnd=_args.end();
		    it != itEnd; ++it )
    {
	    //log_debug("Env-pushing %s", *it);
	    args->push_back(*it);
    }

    /* as_value val = */
    call_method(timer_method, &env, _object.get(), args, super);

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
	//std::stringstream ss; fn.dump_args(ss);
	//log_debug("setInterval(%s)", ss.str());

	//log_debug("%s: args=%d", __FUNCTION__, fn.nargs);
    
	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- need at least 2 arguments",
				ss.str());
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
				ss.str());
		);
		return as_value();
	}

	std::string methodName;

	// Get interval function
	boost::intrusive_ptr<as_function> as_func = obj->to_function(); 
	if ( ! as_func )
	{
		methodName = fn.arg(1).to_string();
		timer_arg = 2;
	}


	if ( fn.nargs < timer_arg+1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setInterval(%s) "
				"- missing timeout argument",
				ss.str());
		);
		return as_value();
	}

	// Get interval time
	unsigned long ms = static_cast<unsigned long>(fn.arg(timer_arg).to_number());
	// TODO: check validity of interval time number ?

	// Parse arguments 
	Timer::ArgsContainer args;
	for (unsigned i=timer_arg+1; i<fn.nargs; ++i)
	{
		args.push_back(fn.arg(i));
	}

	std::auto_ptr<Timer> timer(new Timer);
	if ( as_func )
	{
		// TODO: 'this_ptr' should be NULL/undefined in this case
		timer->setInterval(*as_func, ms, fn.this_ptr, args);
	}
	else
	{
		timer->setInterval(obj, methodName, ms, args);
	}
    
    
	movie_root& root = VM::get().getRoot();
	int id = root.add_interval_timer(timer);
	return as_value(id);
}

// TODO: move to Global.cpp
as_value
timer_settimeout(const fn_call& fn)
{
	//std::stringstream ss; fn.dump_args(ss);
	//log_debug("setTimeout(%s)", ss.str());

	//log_debug("%s: args=%d", __FUNCTION__, fn.nargs);
    
	if ( fn.nargs < 2 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setTimeout(%s) "
				"- need at least 2 arguments",
				ss.str());
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
				ss.str());
		);
		return as_value();
	}

	std::string methodName;

	// Get interval function
	boost::intrusive_ptr<as_function> as_func = obj->to_function(); 
	if ( ! as_func )
	{
		methodName = fn.arg(1).to_string();
		timer_arg = 2;
	}


	if ( fn.nargs < timer_arg+1 )
	{
		IF_VERBOSE_ASCODING_ERRORS(
			std::stringstream ss; fn.dump_args(ss);
			log_aserror("Invalid call to setTimeout(%s) "
				"- missing timeout argument",
				ss.str());
		);
		return as_value();
	}

	// Get interval time
	unsigned long ms = static_cast<unsigned long>(fn.arg(timer_arg).to_number());
	// TODO: check validity of interval time number ?

	// Parse arguments 
	Timer::ArgsContainer args;
	for (unsigned i=timer_arg+1; i<fn.nargs; ++i)
	{
		args.push_back(fn.arg(i));
	}

	std::auto_ptr<Timer> timer(new Timer);
	if ( as_func )
	{
		// TODO: 'this_ptr' should be NULL/undefined in this case
		timer->setInterval(*as_func, ms, fn.this_ptr, args, true);
	}
	else
	{
		timer->setInterval(obj, methodName, ms, args, true);
	}
    
    
	movie_root& root = VM::get().getRoot();

	int id = root.add_interval_timer(timer);
	return as_value(id);
}
  
// TODO: move to Global.cpp
as_value
timer_clearinterval(const fn_call& fn)
{
	//log_debug("%s: nargs = %d", __FUNCTION__, fn.nargs);

	int id = int(fn.arg(0).to_number());

	movie_root& root = VM::get().getRoot();
	bool ret = root.clear_interval_timer(id);
	return as_value(ret);
}

} // namespace gnash
