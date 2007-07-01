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

/// \file Listening_Actions.hpp

#pragma once
#ifndef __Listening_Actions_hpp__
#define __Listening_Actions_hpp__

#include "ACT/Scheduler.hpp"
#include "Action_Tracing.hpp"
#include <boost/optional.hpp>
#include <vector>

namespace ACT {
	// Forward declaration
	class N_to_completion ;

	//-------------------------
	/**	\class N_to_completion_Monitor
	 *	\brief A monitor for N_to_completion.
	 */
	class N_to_completion_Monitor
		: public simple_act
	{
		/// A vector of schedulers
		static std::vector< wakeup_listener::scheduler_pointer > schedulers ;

		std::vector< wakeup_listener * > listeners ;

		act_state run() ;

	public:
		/// Default constructor.
		N_to_completion_Monitor() ;

		/// Query whether there are any actions registered for wakeup.
		inline bool empty() { return listeners.empty() ; }

		void add_wakeup_item( N_to_completion *, wakeup_listener * ) ;
	} ;

	//-------------------------
	/**	\class N_to_completion
	 *	\brief Action that completes after N activations.
	 */
	class N_to_completion
		: public autonomous_act
	{
		/// The monitor type for these actions to register with.
		typedef N_to_completion_Monitor monitor_type ;

		///
		static shared_ptr< monitor_type > the_monitor ;

		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

		///
		unsigned int number_of_activations_left ;

		///
		unsigned int total_number_of_activations ;

		/// Action body, proxied by operator()
		act_state run( wakeup_listener * ) ;

		/// Register an action for wakeup
		void register_for_wakeup( wakeup_listener * ) ;

	public:
		///
		N_to_completion( unsigned int n, tracking_function * = 0 ) ;

		void reset() ;

	} ;

} // end namespace ACT

#endif
