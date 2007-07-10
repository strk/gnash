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

/// \file Supplied_Service.hpp
/// \brief A service that takes an outside supply of actions and supplies them as a service.

#pragma once
#ifndef __Supplied_Service_hpp__
#define __Supplied_Service_hpp__

#include "ACT/Service.hpp"
#include "Action_Tracing.hpp"
#include <list>

namespace ACT {
	//-------------------------
	/**	\class Supplied_Generator
	 *	\brief A generator whose entries are added manually one by one.
	 */
	class Supplied_Generator
		: public Generator
	{
		/// List of pending actions.
		std::list< shared_ptr< basic_act > > the_tasks ;

		/// Remove the first pending action in our internal list and return it.
		shared_ptr< basic_act > next_action( wakeup_listener * ) ;

		/// The active-vs.-shutdown state of this generator.
		bool active ;

		///
		bool complete ;

	public:
		/// Default constructor starts off in active state.
		Supplied_Generator()
			: active( true ), complete( false ) {}

		/// This \c shutdown routine pushes out all pending actions, accepting no new ones.
		void shutdown() { active = false ; }

		///
		void add_task( shared_ptr< basic_act > ) ;

		///
		bool completed() { return complete ; }
	} ;

	//-------------------------
	// Aspect for Service base class
	template< class > class Supplied_Service_Aspect ;

	//-------------------------
	//	Supplied_Service_Aspect
	/**
	 *	\brief Aspect for class \c Service that tracks the run function
	 */
	template<>
	class Supplied_Service_Aspect< Service< Supplied_Service_Aspect > >
		: public Service_Null_Aspect,
		public aspect::Aspect_Has_Access_To_Owner< Service< Supplied_Service_Aspect > >
	{
		/// Tracking
		std::auto_ptr< tracking_function > tracker ;

	public:
		Supplied_Service_Aspect( tracking_function * t )
			: tracker( t )
		{}

		void log_run_begin() { if ( tracker.get() != 0 ) ( * tracker )( "" ) ; }
	} ;

	//-------------------------
	/**	\class Supplied_Service
	 *	\brief A service whose tasks are manually added one by one.
	 */
	class Supplied_Service
		: public Service< Supplied_Service_Aspect >
	{
		/// Aspect type for \c Service base class
		typedef Supplied_Service_Aspect< Service< Supplied_Service_Aspect > > service_base_aspect_type ;

		/// Member allocation for our generator
		Supplied_Generator the_generator ;

	public:
		/// Default constructor
		Supplied_Service( Scheduler & z, tracking_function * t = 0 )
			: Service( the_generator, z, service_base_aspect_type( t ) )
		{}

		///
		inline void shutdown() { the_generator.shutdown() ; }

		///
		inline void add_task( shared_ptr< basic_act > x ) { the_generator.add_task( x ) ; }
	} ;

	//-------------------------
} // end namespace ACT

#endif
