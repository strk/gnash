// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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

/// \file Scheduler.cpp

#include "Scheduler.hpp"

namespace ACT {

	//--------------------------------------------------
	// Basic_Scheduled_Item
	//--------------------------------------------------
	Basic_Scheduled_Item::
	Basic_Scheduled_Item( act x, unsigned int n, Basic_Priority p )
		: the_action( x ),
		sequence_number( n ),
		priority_category( p )
	{}

	//-------------------------
	bool
	Basic_Scheduled_Item::
	operator<( const Basic_Scheduled_Item & x ) const
	{
		return	priority_category < x.priority_category
			||	sequence_number > x.sequence_number ;
	}

	//--------------------------------------------------
	// Basic_Scheduler
	//--------------------------------------------------
	// Class member initialization
	Basic_Scheduler Basic_Scheduler::the_instance ;

	//-------------------------
	// Class factory function member
	Basic_Scheduler & 
	Basic_Scheduler::obtain_scheduler()
	{
		return the_instance ;
	}

	//-------------------------
	Basic_Scheduler::
	Basic_Scheduler()
		: operating( true )
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
		item_pointer item = the_queue.push( Basic_Scheduled_Item( x, ++ next_sequence_number ) ) ;
		item -> listener = Basic_Wakeup_Listener( item, & the_queue ) ;
	}

	void
	Basic_Scheduler::
	add_service( act )
	{
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

		Scheduling_Queue< Basic_Scheduled_Item >::reference item = the_queue.top() ;
		act_state result = item.the_action( & item.listener.get() ) ;		//.get() because .listener is optional<>
		if ( result == Working ) {
			item.priority_category = Waiting ;
			the_queue.reorder( the_queue.top_ptr() ) ;
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

	//--------------------------------------------------
	// Basic_Wakeup_Listener
	//--------------------------------------------------
	void
	Basic_Wakeup_Listener::
	operator()()
	{
		the_item -> priority_category = Ordinary ;
		the_scheduler -> reorder( the_item ) ;
	}


} // end namespace ACT
