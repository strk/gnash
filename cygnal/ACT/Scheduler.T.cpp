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

/// \file Scheduler.T.cpp

#include "Scheduler.hpp"

namespace ACT {
	//--------------------------------------------------
	// wakeup_listener_allocated
	//--------------------------------------------------
	template< class S >
	wakeup_listener_allocated< S >::
	wakeup_listener_allocated( size_t x, scheduler_pointer y )
		: the_wakeup_listener( new Basic_Wakeup_Listener< S >( x, y ) ) {}

	//--------------------------------------------------
	// Basic_Wakeup_Listener
	//--------------------------------------------------
	template< class S >
	void
	Basic_Wakeup_Listener< S >::
	operator()()
	{
		queue_type & the_queue = the_scheduler -> queue() ;
		the_queue.item( permutation_index ).priority_category = Ordinary ;
		the_queue.reorder( permutation_index ) ;
	}

	//--------------------------------------------------
	// Basic_Scheduler
	//--------------------------------------------------
	template< template< class > class Aspect >
	Basic_Scheduler< Aspect >::
	Basic_Scheduler( aspect_type aspect )
		: Scheduler( this ),
		operating( true ),
		next_task_sequence_number( 1 ),
		next_service_sequence_number( 1 ),
		aspect( aspect )
	{
		aspect.set_owner( this ) ;
	}

	//-------------------------
	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
	reset()
	{
		operating = true ;
	}

	//-------------------------
	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
	operator()()
	{
		aspect.run_begin() ;
		while ( operating ) {
			if ( ! aspect.run_guard() ) {
				aspect.run_end_guard_violation() ;
				return ;
			}
			activate_one_item() ;
		}
		aspect.run_end_ordinary() ;
	}

	//-------------------------
	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
	add_task( act x )
	{
		item_pointer item = the_queue.push( Basic_Scheduled_Item( x, ++ next_task_sequence_number ), this ) ;
	}

	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
	add_service( act x )
	{
		item_pointer item = the_queue.push( Basic_Scheduled_Item( x, ++ next_service_sequence_number, Demon ), this ) ;
	}

	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
	add_critical_service( act )
	{
	}

	//-------------------------
	template< template< class > class Aspect >
	void
	Basic_Scheduler< Aspect >::
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

		typename queue_type::pointer item = the_queue.top_ptr() ;
		ACT_State result = item -> the_action( the_queue.auxiliary_top() -> get() ) ;
		if ( result.working() ) {
			switch ( item -> action_type ) 
			{
				case Task :
					// Assert action is not a service
					item -> priority_category = Waiting ;
					the_queue.reorder( item ) ;
					break ;
				case Demon :
					// Always reset the sequence number for a demon.
					// The resulting behavior is to cycle through all available demons.
					item -> sequence_number = next_service_sequence_number ++ ;
					the_queue.reorder( item ) ;
					break ;
			}
		} else {
			the_queue.pop() ;
			// Perhaps we might want to log items gone bad here.
		}
	}

	//-------------------------
	template< template< class > class Aspect >
	bool
	Basic_Scheduler< Aspect >::
	ordinary_tasks_available()
	{
		if ( the_queue.empty() ) return false ;
		return the_queue.top().priority_category <= Ordinary ;
	}

} // end namespace ACT
