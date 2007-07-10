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
	template< template< class > class Aspect >
	Service< Aspect >::
	Service( Generator & x, Scheduler & z, aspect_type aspect )
		: the_generator( x ),
		our_scheduler( z ),
		aspect( aspect )
	{
		aspect.set_owner( this ) ;
	}

	//-------------------------
	/*	\par Implementation
	 */
	template< template< class > class Aspect >
	ACT_State
	Service< Aspect >::
	run( wakeup_listener * w )
	{
		aspect.log_run_begin() ;
		// We use a loop rather than a single pass because the generator may have multiple pending actions ready for us.
		// We'd get a similar result with just a single pass.
		// The difference is that using a single pass here would cause loop overhead in the scheduler.
		// It's more efficient to do ordinary cycling here.
		//
		bool task_added( false ) ;
		while ( true ) {
			try {
				shared_ptr< basic_act > x( the_generator.next_action( w ) ) ;
				if ( ! x ) {
					if ( the_generator.completed() && ! task_added ) {
						aspect.log_run_end() ;
						return set_completed() ;
					} else {
						aspect.log_run_end() ;
						return set_ready() ; 
					}
				}
				our_scheduler.add_task( x ) ;
				task_added = true ;
			} catch ( ... ) {
				aspect.log_run_end() ;
				return set_bad() ;
			}
			// If, in the future, ours scheduler might be unavailable for adding something new to the queue.
			// Such unavailability would (hopefully) be temporary.
			// It might arise, for example, from a non-blocking queue operation that just wasn't ready just yet.
			// What we would do in that case is to save our action-pending state and introduce at the next invocation.
		}
	}
}
