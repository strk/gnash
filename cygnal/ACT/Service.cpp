// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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

/// \file Service.cpp

#include "Service.hpp"

namespace ACT {
	//-------------------------
	/*	\par Implementation
	 */
	Service::
	Service( Generator & x, Basic_Scheduler & z )
		: the_generator( x ),
		our_scheduler( z )
	{}

	//-------------------------
	/*	\par Implementation
	 */
	act_state
	Service::
	run( wakeup_listener * w )
	{
		// We use a loop rather than a single pass because the generator may have multiple pending actions ready for us.
		// We'd get a similar result with just a single pass.
		// The difference is that using a single pass here would cause loop overhead in the scheduler.
		// It's more efficient to do ordinary cycling here.
		//
		while ( true ) {
			the_generator( w ) ;
			if ( the_generator.working() ) return ACT::Working ;
			if ( the_generator.bad() ) return set_bad() ;
			// Assert the_generator.completed()

			act x( the_generator.result() ) ;
			our_scheduler.add_task( x ) ;

			// If, in the future, ours scheduler might be unavailable for adding something new to the queue.
			// Such unavailability would (hopefully) be temporary.
			// It might arise, for example, from a non-blocking queue operation that just wasn't ready just yet.
			// What we would do in that case is simply to return, avoiding the following reset.
			// Upon next invocation, the result won't have changed and we can just try again.

			// Reset the generator and re-run the action.
			the_generator.reset() ;
		}
	}

}
