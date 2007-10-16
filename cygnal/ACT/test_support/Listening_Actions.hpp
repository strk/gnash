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

#include "ACT/Listen.hpp"
#include "Action_Tracing.hpp"

namespace ACT {
	// Forward
	class N_to_completion ;

	//-------------------------
	/**	\class N_to_completion_Monitor
	 *	\brief A monitor for N_to_completion.
	 *
	 *	Note that this class derives from \c simple_act, rather than \c autonomous_act.
	 *	The monitor that wakes up other actions does not itself sleep.
	 */
	class N_to_completion_Monitor
		: public Basic_Listen_Monitor< N_to_completion >
	{
		/// Type of parent monitor class, called often in implementation
		typedef Basic_Listen_Monitor< N_to_completion > Parent ;

		/// Action body
		ACT_State run() ;

	public:
		/// Trivial default constructor.
		N_to_completion_Monitor() {}
		
		/// Trivial virtual destructor
		~N_to_completion_Monitor() {}

		/// Implementation of specific wake-up preparation
		void add_wakeup_item( N_to_completion *, wakeup_listener * ) ;
	} ;

	//-------------------------
	/**	\class N_to_completion
	 *	\brief Action that completes after N activations.
	 */
	class N_to_completion
		: public Basic_Listening_Task< N_to_completion, N_to_completion_Monitor >
	{
		///
		unsigned int total_number_of_activations ;

		///
		unsigned int number_of_activations_left ;

		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

		/// Action body, proxied by operator()
		ACT_State run( wakeup_listener * ) ;

	public:
		///
		N_to_completion( unsigned int n, tracking_function * = 0 ) ;

		/// Trivial virtual destructor
		virtual ~N_to_completion() {}
		
		void reset() ;
	} ;

	//-------------------------
} // end namespace ACT

#endif
