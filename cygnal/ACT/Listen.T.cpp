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

/// \file Listen.T.cpp
/// \brief Implementations of a set of simple actions with which to test the scheduler

#include "Listen.hpp"

namespace ACT {
	//--------------------------------------------------
	// Basic_Listen_Monitor
	//--------------------------------------------------
	template< class Listener >
	void
	Basic_Listen_Monitor< Listener >::
	add_ready_listener_task( wakeup_listener * w )
	{
		ready_list.push_back( w ) ;
	}

	template< class Listener >
	ACT_State
	Basic_Listen_Monitor< Listener >::
	wake_up_ready_tasks()
	{
		if ( ready_list.empty() ) {
			// This monitor is no longer necessary, since there's nothing to wake up.
			// Therefore we return complete and let the scheduler remove this instance.
			// We first have to remove ourselves from the follower registry, in order to be re-created when needed.

			// Note that we only return Completed when the ready list is empty at the start of execution.
			// This satisfies the service requirement 
			//		that a service action may not both reschedule a task and complete itself in the same invocation.
			return set_completed() ;
		}
		do {
			( * ready_list.back() )() ;
			ready_list.pop_back() ;
		} while ( ! ready_list.empty() ) ;
		return set_ready() ;
	}

	//--------------------------------------------------
	// Basic_Listening_Task
	//--------------------------------------------------
	// Class registry definition
	template< class Listener, class Monitor >
	typename Basic_Listening_Task< Listener, Monitor >::follower_registry_type
	Basic_Listening_Task< Listener, Monitor >::schedulers ;

	//-------------------------
	template< class Listener, class Monitor >
	void
	Basic_Listening_Task< Listener, Monitor >::
	register_for_wakeup( Listener * that, wakeup_listener * w )
	{
		if ( w == 0 ) return ;
		// Assert wakeup listener is non-trivial.

		handle_type k = w -> scheduler() -> handle() ;
		// Assert k is the handle to the scheduler of origin for our wakeup_listener
		shared_ptr< monitor_type > p = schedulers[ k ].lock() ;
		// Assert p is the monitor associated with our scheduler, if one exists.

		// Ensure monitor exists
		if ( ! p ) {
			// Assert monitor does not yet exist
			p = shared_ptr< monitor_type >( new monitor_type() ) ;
			schedulers[ k ] = boost::weak_ptr< monitor_type >( p ) ;
			// Monitor exists but it's not yet scheduled (invariant violation)
			w -> scheduler() -> add_service( act( p ) ) ;
			// Assert monitor exists in registry and it's scheduled (invariant satisfied)
		}
		// Assert listening monitor for this class exists
		// Assert p == schedulers[ k ].lock() && p != 0

		// Add the current instance to the monitor associated with our wakeup listener
		p -> add_wakeup_item( that, w ) ;
	}

	//-------------------------
} // end namespace ACT
