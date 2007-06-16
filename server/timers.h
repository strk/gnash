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

#ifndef __TIMERS_H__
#define __TIMERS_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "as_value.h" // for struct variable composition
#include "as_object.h" // for inheritance
#include "as_function.h" // for visibility of destructor by intrusive_ptr
#include "smart_ptr.h"

#include "tu_timer.h"

#include <string>

// Forward declarations
namespace gnash {
	class fn_call;
	class as_function;
}

namespace gnash {
  
  struct variable {
    std::string name;
    as_value value;
  };

/// An interval timer.
//
/// This is constructed when _global.setInterval() is called.
/// Instances of this class will be stored in the movie_root singleton.
///
/// A timer has a function to call, a context in which to call it, a
/// list of arguments and an interval specifying how often the function must be
/// called.
///
/// It is *not* a "smart" timer, which is
/// it will *not* automatically execute at given intervals. Rather, it
/// will be movie_root responsibility to execute the timer-associated
/// function at regular intervals. As a facility, the Timer class provides
/// an execution operator, proxying the execution to the associated function
/// with properly set-up context.
///
///
class Timer
{

public:

      /// Construct a disabled (cleared) timer.
      Timer();

      ~Timer();

      /// Setup the Timer, enabling it.
      //
      /// @param method
      ///	The function to call from execution operator.
      ///	Will be stored in an intrusive_ptr.
      ///
      /// @param ms
      ///	The number of milliseconds between expires.
      ///
      /// @param this_ptr
      ///	The object to be used as 'this' pointer when calling the
      ///	associated function. Will be stored in an intrusive_ptr.
      ///	It is allowed to be NULL as long as fn_call is allowed
      ///	a NULL as 'this_ptr' (we might want to change this).
      ///
      void setInterval(as_function& method, unsigned ms, boost::intrusive_ptr<as_object> this_ptr);

      /// Setup the Timer, enabling it.
      //
      /// @param method
      ///	The function to call from execution operator.
      ///	Will be stored in an intrusive_ptr.
      ///
      /// @param ms
      ///	The number of milliseconds between expires.
      ///
      /// @param this_ptr
      ///	The object to be used as 'this' pointer when calling the
      ///	associated function. Will be stored in an intrusive_ptr.
      ///	It is allowed to be NULL as long as fn_call is allowed
      ///	a NULL as 'this_ptr' (we might want to change this).
      ///
      /// @param args
      /// 	The list of arguments to pass to the function being invoked.
      ///
      void setInterval(as_function& method, unsigned ms, boost::intrusive_ptr<as_object> this_ptr, 
		      std::vector<as_value>& args);

      /// Clear the timer, ready for reuse
      //
      /// When a Timer is cleared, the expired() function
      /// will always return false.
      ///
      /// Use setInterval() to reset it.
      ///
      void clearInterval();

      /// Return true if interval ticks are passed since last call to start()
      //
      /// Always returns false if the timer is cleared.
      //
      bool expired();

      /// Execute associated function properly setting up context
      void operator() ();
      
      /// Arguments list type
      typedef std::vector<as_value> ArgsContainer;
      
#ifdef GNASH_USE_GC
	/// Mark all reachable resources (for GC)
	//
	/// Resources reachable from Timer are:
	///
	///	- Arguments list (_args)
	///	- Associated function (_function)
	///	- Target object (_object)
	///
	void markReachableResources() const;
#endif // GNASH_USE_GC

private:

      /// Set timer start
      //
      /// Called by every function setting the interval.
      ///
      void start();

      /// Number of microseconds between expirations 
      uint64_t _interval;

      /// Number of microseconds since epoch at Timer start
      uint64_t _start;

      /// The associated function, stored in an intrusive pointer
      boost::intrusive_ptr<as_function> _function;

      /// Context for the function call. Will be used as 'this' pointer.
      boost::intrusive_ptr<as_object> _object;

      /// List of arguments
      ArgsContainer _args;
};
  
  as_value timer_setinterval(const fn_call& fn);
  as_value timer_clearinterval(const fn_call& fn);
  as_value timer_expire(const fn_call& fn);
  
} // end of namespace gnash

  // __TIMERS_H_
#endif
