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

/// \file Listening_Actions.cpp
/// \brief Implementations of a set of simple actions with which to test the scheduler

#include "Listening_Actions.hpp"
#include "ACT/Scheduler.hpp"

// Required to obtain instantiation of template base classes
#include "ACT/Listen.T.cpp"

namespace ACT {
	//--------------------------------------------------
	// N_to_completion_Monitor
	//--------------------------------------------------
	void
	N_to_completion_Monitor::
	add_wakeup_item( N_to_completion *, wakeup_listener * w )
	{
		//	We ignore the N_to_completion pointer, since, for the purposes of the present test action,
		//		we consider any background action always ready for wakeup after one cycle of quiescence.

		Parent::add_ready_listener_task( w ) ;
	}

	ACT_State
	N_to_completion_Monitor::
	run()
	{
		return Parent::wake_up_ready_tasks() ;
	}

	//--------------------------------------------------
	// N_to_completion
	//--------------------------------------------------
	N_to_completion::
	N_to_completion( unsigned int n, tracking_function * f )
		: total_number_of_activations( n ),
		number_of_activations_left( n ),
		tracker( f )
	{}

	//-------------------------
	ACT_State
	N_to_completion::
	run( wakeup_listener * w )
	{
		if ( tracker.get() != 0 ) ( * tracker )( "" ) ;
		-- number_of_activations_left ;
		if ( number_of_activations_left == 0 ) return set_completed() ;
		// Assert we're not done yet.
		// Register with our monitor before returning.
		register_for_wakeup( this, w ) ;
		return set_would_block() ; ;
	}

	//-------------------------
	void
	N_to_completion::
	reset()
	{
		number_of_activations_left = total_number_of_activations ;
		set_ready() ;
	}

	//-------------------------
} // end namespace ACT

//-------------------------
// Instantiate an instance of Handle_Registry_Follower as needed for N_to_Completion
#include "ACT/Handle.cpp"
namespace ACT {
	template class Handle_Registry_Follower< shared_ptr< N_to_completion_Monitor >, Scheduler > ;
}
