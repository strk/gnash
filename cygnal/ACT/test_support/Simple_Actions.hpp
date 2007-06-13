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

/// \file Simple_Actions.hpp

#pragma once
#ifndef __Simple_Actions_hpp__
#define __Simple_Actions_hpp__

#include "ACT/ACT.hpp"
#include "Action_Tracing.hpp"

namespace ACT {
	//-------------------------
	/**	\class N_to_completion
	 *	\brief Action that completes after N activations.
	 */
	class N_to_completion
		: public autonomous_act
	{
		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

		///
		unsigned int number_of_activations_left ;

		///
		unsigned int total_number_of_activations ;

		///
		act_state run( wakeup_listener * ) ;

	public:
		///
		N_to_completion( unsigned int n, tracking_function * = 0 ) ;

		void reset() ;

	} ;

	//-------------------------
	/**	\class single_action
	 */
	class single_action
		: public simple_act
	{
		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

		/// Action body
		act_state run() ;
	public:
		///
		single_action( tracking_function * x = 0 )
			: tracker( x )
		{}
	} ;

	//-------------------------
	/**	\class no_action
	 *	\brief An action that starts off completed.
	 *
	 *	This isn't as unrealistic as a test as it might seem at first glance.
	 *	There are actions whose constructors may performs some operation that completes the goal of the action.
	 *	Alternately, a watcher action may find that its watch predicate is satisfied at construction.
	 */
	class no_action
		: public simple_act
	{
		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

		/// Action body does very little.
		act_state run() ;

	public:
		/// Ordinary constructor.
		no_action( tracking_function * x = 0 )
			: tracker( x )
		{
			set_completed() ;
		}
	} ;

} // end namespace ACT

#endif