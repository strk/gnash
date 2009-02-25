// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
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

#ifndef HAVE_TIMERS_H
#define HAVE_TIMERS_H

#include "dsodefs.h"

#include "as_value.h" // for struct variable composition
#include "as_object.h" // for inheritance
#include "as_function.h" // for visibility of destructor by intrusive_ptr
#include "smart_ptr.h" // GNASH_USE_GC

#include <string>
#include <vector> 
#include <limits>

// Forward declarations
namespace gnash {
	class fn_call;
	class as_function;
}

namespace gnash {
  
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
class DSOEXPORT Timer
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
      /// @param runOnce
      /// 	If true the interval will run only once. False if omitted.
      ///
      void setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr, bool runOnce=false);

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
      /// @param runOnce
      /// 	If true the interval will run only once. False if omitted.
      ///
      void setInterval(as_function& method, unsigned long ms, boost::intrusive_ptr<as_object> this_ptr, 
		      std::vector<as_value>& args, bool runOnce=false);

      /// Setup the Timer to call a late-evaluated object method, enabling it.
      //
      /// @param this_ptr
      ///	The object to be used as 'this' pointer when calling the
      ///	associated function. Will be stored in an intrusive_ptr.
      ///	It is allowed to be NULL as long as fn_call is allowed
      ///	a NULL as 'this_ptr' (we might want to change this).
      ///
      /// @param methodName
      ///	The method name to call from execution operator.
      ///
      /// @param ms
      ///	The number of milliseconds between expires.
      ///
      /// @param args
      /// 	The list of arguments to pass to the function being invoked.
      ///
      /// @param runOnce
      /// 	If true the interval will run only once. False if omitted.
      ///
      void setInterval(boost::intrusive_ptr<as_object> obj, const std::string& methodName, unsigned long ms, 
		      std::vector<as_value>& args, bool runOnce=false);

      /// Clear the timer, ready for reuse
      //
      /// When a Timer is cleared, the expired() function
      /// will always return false.
      ///
      /// Use setInterval() to reset it.
      ///
      void clearInterval();

      /// Get expiration state
      //
      /// @param now
      ///    Current time, in milliseconds.
      ///
      /// @param elapsed
      ///    Output parameter, will be set to the amount of milliseconds
      ///    elapsed since actual expiration, if expired.
      ///
      /// @return true if the timer expired, false otherwise.
      ///
      bool expired(unsigned long now, unsigned long& elapsed); 

      /// Return true if interval has been cleared.
      //
      /// Note that the timer is constructed as cleared and you
      /// need to call setInterval() to make it not-cleared.
      bool cleared() const
      {
            return _start == std::numeric_limits<unsigned long>::max();
      }

      /// Execute associated function and reset state
      //
      /// After execution either the timer is cleared
      /// (if runOnce) or start time is incremented
      /// by the interval.
      ///
      /// NOTE: if the timer is cleared this call
      ///       results in a no-op.
      ///
      void executeAndReset();

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

      /// Execute associated function properly setting up context
      void execute();

      /// Execute associated function properly setting up context
      void operator() () { execute(); }
      
      /// Return number of milliseconds between expirations 
      unsigned long getInterval() const { return _interval; }

      /// Return number of milliseconds after VM start this timer was last reset
      unsigned long getStart() const { return _start; }


      /// Set timer start
      //
      /// Called by every function setting the interval.
      ///
      void start();

      /// Number of milliseconds between expirations 
      unsigned int _interval;

      /// Number of milliseconds since epoch at Timer start 
      //
      /// This will be numeric_limits<unsigned long>::max()
      /// if the timer is not active (or cleared)
      ///
      unsigned long _start;

      /// The associated function (if statically-bound) stored in an intrusive pointer
      boost::intrusive_ptr<as_function> _function;

      /// The associated method name, stored in an intrusive pointer
      std::string _methodName;

      /// Context for the function call. Will be used as 'this' pointer.
      boost::intrusive_ptr<as_object> _object;

      /// List of arguments
      ArgsContainer _args;

      /// True if the timer should execute only once (for setTimeout)
      bool _runOnce;
};
  
  as_value timer_setinterval(const fn_call& fn);
  as_value timer_settimeout(const fn_call& fn);
  as_value timer_clearinterval(const fn_call& fn);
  
} // end of namespace gnash

  // HAVE_TIMERS_H
#endif
