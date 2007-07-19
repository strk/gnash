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

/// \file Service.hpp

#pragma once
#ifndef __Service_hpp__
#define __Service_hpp__

#include "ACT.hpp"
#include "Scheduler.hpp"
#include "Aspect.hpp"

namespace ACT {
	//-------------------------
	/**	\class Generator
	 *	\brief A generator of actions, in bursts, used to feed a Service.
	 *		This class is a base for service implementations.
	 *
	 */
	class Generator
	{
	public:
		/**	The parameter is necessary for when a Service must suspend itself waiting for external events.
		 *	The central example is a socket listener, which generates new connections, each encapsulated in a protocol behavior.
		 *
		 *	\post
		 *	- return is zero implies no more action is available at the present time.  Ask again later.
		 */
		virtual shared_ptr< basic_act > next_action( wakeup_listener * ) =0 ;

		/**
		 *	\post
		 *	- return is true 
		 *			implies return value of next_action will always be zero
		 *		and	implies return value of completed will always be true
		 */
		virtual bool completed() { return false ; }

		/** The meaning of this shut-down routine are weak and lazy, rather than strong and immediate.
		 *	The only requirement is that the number of ready actions monotonically decrease with each call to \c next_action.
		 *	Conceptually, there must be a counter of ready actions.
		 *	It must decrease this count by at least one with each call to \c next_action.
		 *
		 *	These ready actions may be implicit or explicit.
		 *	Explicit action are those, say, waiting in a queue, already constructed and ready to go.
		 *	Generically, all generators will contain these kind of explict actions.
		 *
		 *	A central example of an implicit action is the set of pending connections on a listening socket.
		 *	The OS has established a connection, so there's some potential action on that connection.
		 *	Whatever that action is matter nothing to this base class.
		 *	What does matter is that after shutdown, that the listening socket be closed, 
		 *		or at least put in a state to reject connections, thereby not generating any new actions.
		 *
		 *	The weak requirement of this shutdown does not preclude immediate termination.
		 *	The requirement is that the count of available actions decrease by at least one, not by exactly one.
		 *	A service that wishes to hang up immediately, simply ending, may do so.
		 *	Indeed, a particular implementation class might well have configurable shutdown behavior,
		 *		depending on whether the desired shutdown is graceful or abrupt.
		 *
		 *	Implementing classes should document their approach to shutdown.
		 *	This will avoid incorrect expectations about behavior,
		 *		which expectations might develop out of habit or idiom, rather than out of requirement.
		 */
		virtual void shutdown() =0 ;
	} ;

	//-------------------------
	/**	\class Service
	 *	\brief A service is a persistent action whose result to create new actions and to schedule them for execution.
	 *
	 *	This generic base class has no result accessor, since services run primarily under the scheduler.
	 *	The scheduler, which deals with a multiplicity of action types, has no way of dealing with specific results.
	 *
	 *	A service, in addition, is "always working".
	 *	It only completes when it needs to shut down, either from an internal failure or by external command.
	 *
	 *	WARNING: A service must not both schedule a new action and return \c Completed in the same activation.
	 *	This restriction simplifies implementation of the scheduler, which then doesn't need to removed
	 *		a completed service action from within the middle of the scheduling queue.
	 *	When a service schedules no new actions, it remains at the top of the priority queue,
	 *		from where it can be removed like any other task.
	 */
	template< template< class > class Aspect = aspect::Null_Aspect_0 >
	class Service 
		: public autonomous_act
	{
	public:
		typedef Aspect< Service > aspect_type ;

	private:
		/// Aspect instance.
		aspect_type aspect ;

		Scheduler & our_scheduler ;

		Generator & the_generator ;
	public:
		/// 
		Service( Generator & x, Scheduler & z, aspect_type aspect = aspect_type() ) ;

		///
		ACT_State run( wakeup_listener * ) ;

		///
		inline void shutdown() { the_generator.shutdown() ; }
	} ;

	//-------------------------
	/**	\class Service_Null_Aspect
	 *	\brief Null aspect for \c Service
	 */
	class Service_Null_Aspect
	{
	public:
		void log_run_begin() {} ;
		void log_run_end() {} ;
	} ;

	//-------------------------
} // end namespace ACT

namespace aspect {
	//-------------------------
	/**	\brief Default null aspect for \c Service.
	 */
	template<>
	class Null_Aspect_0< ACT::Service< Null_Aspect_0 > >
		: public ACT::Service_Null_Aspect,
		public Null_Aspect_Base< ACT::Service< Null_Aspect_0 > >
	{} ;
}

#endif