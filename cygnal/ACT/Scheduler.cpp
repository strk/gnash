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

/// \file Scheduler.cpp

#include "Scheduler.hpp"

namespace ACT {

	//--------------------------------------------------
	// Basic_Scheduled_Item
	//--------------------------------------------------
	Basic_Scheduled_Item::
	Basic_Scheduled_Item( act x, unsigned int n, Action_Category action_type )
		: the_action( x ),
		sequence_number( n ),
		action_type( action_type )
	{
		switch ( action_type ) {
			case Task :
				priority_category = Initial ;
				break ;
			case Service :
				priority_category = Background ;
				break ;
			default :
				priority_category = Background ;
				break ;
		}
	}

	//-------------------------
	bool
	Basic_Scheduled_Item::
	operator<( const Basic_Scheduled_Item & x ) const
	{
		return	priority_category > x.priority_category
			||	(		priority_category == x.priority_category 
					&&	sequence_number > x.sequence_number 
				) ;
	}

	//--------------------------------------------------
	// wakeup_listener_allocated
	//--------------------------------------------------
	wakeup_listener_allocated::
	wakeup_listener_allocated( size_t x, scheduler_pointer y )
		: the_wakeup_listener( new wakeup_listener( x, y ) ) {}

	//--------------------------------------------------
	// wakeup_listener
	//--------------------------------------------------
	void
	wakeup_listener::
	operator()()
	{
		queue_type & the_queue = the_scheduler -> queue() ;
		the_queue.item( permutation_index ).priority_category = Ordinary ;
		the_queue.reorder( permutation_index ) ;
	}

	//--------------------------------------------------
	// Basic_Scheduler
	//--------------------------------------------------
	Basic_Scheduler::
	Basic_Scheduler()
		: operating( true ),
		next_sequence_number( 1 )
	{}

	//-------------------------
	void
	Basic_Scheduler::
	reset()
	{
		operating = true ;
	}

	//-------------------------
	void
	Basic_Scheduler::
	operator()()
	{
		while ( operating ) {
			activate_one_item() ;
		}
	}

	//-------------------------
	void
	Basic_Scheduler::
	operator()( unsigned int activation_bound )
	{
		while ( operating && activation_bound > 0 ) {
			-- activation_bound ;
			activate_one_item() ;
		}
	}

	//-------------------------
	void
	Basic_Scheduler::
	add_task( act x )
	{
		item_pointer item = the_queue.push( Basic_Scheduled_Item( x, ++ next_sequence_number ), this ) ;
	}

	void
	Basic_Scheduler::
	add_service( act x )
	{
		item_pointer item = the_queue.push( Basic_Scheduled_Item( x, ++ next_sequence_number, Service ), this ) ;
	}

	void
	Basic_Scheduler::
	add_critical_service( act )
	{
	}

	//-------------------------
	void
	Basic_Scheduler::
	activate_one_item()
	{
		// Note that this operation is not locked.
		// As of this version, this is a single-threaded server.
		if ( the_queue.empty() ) {
			// If the queue is finally empty, we're done.
			operating = false ;
			return ;
			// An alternate behavior would be to wait indefinitely for another scheduled item.
			// This is an asynchronous scheduler; it doesn't wait.
			// To understand how this scheduler wakes up from quiescence, look into service actions.
		}
		// Assert the_queue is not empty

		queue_type::pointer item = the_queue.top_ptr() ;
		act_state result = item -> the_action( the_queue.auxiliary_top() -> get() ) ;
		if ( result == Working ) {
			if ( item -> action_type == Task ) {
				// Assert action is not a service
				item -> priority_category = Waiting ;
				the_queue.reorder( item ) ;
			}
			// Note service actions do not re-prioritize right now
		} else {
			the_queue.pop() ;
			// Perhaps we might want to log items gone bad here.
		}
	}

	//-------------------------
	bool
	Basic_Scheduler::
	ordinary_tasks_available()
	{
		if ( the_queue.empty() ) return false ;
		return the_queue.top().priority_category <= Ordinary ;
	}

} // end namespace ACT
