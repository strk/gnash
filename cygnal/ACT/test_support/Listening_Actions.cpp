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

/// \file Listening_Actions.cpp
/// \brief Implementations of a set of simple actions with which to test the scheduler

#include "Listening_Actions.hpp"
#include "ACT/Scheduler.hpp"

namespace ACT {
	//--------------------------------------------------
	// N_to_completion_Monitor
	//--------------------------------------------------
	N_to_completion_Monitor::
	N_to_completion_Monitor()
		: listeners( 0 )
	{}

	void
	N_to_completion_Monitor::
	add( N_to_completion *, wakeup_listener * w )
	{
		// Assert this instance is registered in the scheduler and will eventually run.
		// Note: as of this writing, there's no way for an object to register itself with the scheduler.
		//		Some other object must do so.
		//		It's not yet clear whether this is good or not.

		//	We ignore the N_to_completion pointer, since in this test action,
		//		we consider any background action always ready for wakeup after one cycle of quiescence.
		listeners.push_back( w ) ;
	}

	act_state
	N_to_completion_Monitor::
	run()
	{
		while ( ! listeners.empty() ) {
			( * listeners.back() )() ;
			listeners.pop_back() ;
		}
		return Working ;
	}

	//--------------------------------------------------
	// N_to_completion
	//--------------------------------------------------
	boost::shared_ptr< N_to_completion_Monitor > N_to_completion::the_monitor ;

	//-------------------------
	N_to_completion::
	N_to_completion( unsigned int n, tracking_function * f )
		: total_number_of_activations( n ),
		number_of_activations_left( n ),
		tracker( f )
	{}

	//-------------------------
	act_state
	N_to_completion::
	run( wakeup_listener * w )
	{
		if ( tracker.get() != 0 ) ( * tracker )( "" ) ;
		-- number_of_activations_left ;
		if ( number_of_activations_left == 0 ) return set_completed() ;
		// Assert we're not done yet.
		// Register with our monitor before returning.
		register_for_wakeup( w ) ;
		return Working ;
	}

	//-------------------------
	void
	N_to_completion::
	reset()
	{
		number_of_activations_left = total_number_of_activations ;
		set_working() ;
	}

	//-------------------------
	void
	N_to_completion::
	register_for_wakeup( wakeup_listener * w )
	{
		if ( the_monitor.get() == 0 ) {
			the_monitor = shared_ptr< monitor_type >( new monitor_type() ) ;
			Basic_Scheduler::obtain_scheduler().add_service( act( the_monitor ) ) ;
		}
		// Assert monitor exists
		the_monitor -> add( this, w ) ;
	}

	//-------------------------
} // end namespace ACT
