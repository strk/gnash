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

/// \file Listen.hpp

#pragma once
#ifndef __Listen_hpp__
#define __Listen_hpp__

#include "ACT/Scheduler.hpp"
#include "ACT/Handle.hpp"

#include <boost/weak_ptr.hpp>
#include <vector>

namespace ACT {
	//-------------------------
	/**	\class Basic_Listen_Monitor
	 *	\brief Base class for monitor service actions.
	 *
	 *	Collaborators:
	 *		- Basic_Listening_Task. Tasks register themselves here when they need to wake up.
	 *		- Scheduler. This action runs as a service within the scheduler.  
	 *			While this class uses no specific knowledge of its scheduler, it satisfies the requirements for a service action.
	 *		- wakeup_listener.  Calls this is a function object to effect a wake-up.
	 *	Responsibilities:
	 *		- Hold a record of each task that is waiting and won't again run until this class wakes it up.
	 *		- Run as a service in the same scheduler that each task is also in.
	 *		- When activated, wake up waiting tasks that have become ready.
	 *			Note: this is not a postcondition for monitors generally.
	 *			A monitor might only wake up a subset of its ready tasks at each activation.
	 */
	template< class Listener >
	class Basic_Listen_Monitor
		: public simple_act
	{
		/// Container for actions ready to run.
		///
		/// \par Future
		///		The vector used currently will be replaced by one provided by the scheduling queue.
		///		Since each task is present in at most one monitor list,
		///			we may eliminate an entire dynamic allocation issue by allocating them within the scheduling queue.
		std::vector< wakeup_listener * > ready_list ;

	protected:
		/// Run through the ready list and wake up all the tasks on it, removing them from the ready list.
		///
		/// This is typically the last statement in the action body of a derived monitor.
		/// \post
		///	- return value is either Completed or Working
		ACT_State wake_up_ready_tasks() ;

		/// Add an item directly to the ready list, skipping readiness checking
		///
		/// Since the item is known to be ready, it needs no further examination.
		/// It need not appear in the parameter list, in contradistinction to \c add_waiting_listener_task.
		void add_ready_listener_task( wakeup_listener * ) ;

		/// Add an item to the waiting list
		void add_waiting_listener_task( Listener *, wakeup_listener * ) ;
	} ;

	//-------------------------
	/**	\class Basic_Listening_Task
	 *	\brief Base class for tasks that may suspend themselves and listen for wakeup.
	 *
	 *	Collaborators:
	 *		- Basic_Listen_Monitor. An instance registers itself with its monitor, upon which it relies for wake-up.
	 *		- Scheduler. This action runs as a task within the scheduler.  
	 *	Responsibilities.
	 *		- Register self for later wake-up if it suspends computation.
	 */
	template< class Listener, class Monitor >
	class Basic_Listening_Task
		: public autonomous_act
	{
		/// The monitor type for these actions to register with.
		typedef Monitor monitor_type ;

		/// Type of handles to Scheduler
		typedef Scheduler::handle_type handle_type ;

		/// Type of the class registry of monitors
		typedef typename Handle_Registry_Follower< boost::weak_ptr< Monitor >, Scheduler > follower_registry_type ;

		/**	A vector of monitors, indexed by Scheduler handles.
		 *
		 *	The cardinality relationship is either zero-or-one monitor instance for each scheduler.
		 *
		 *	\invariant
		 *	- Each monitor within this registry is present in the queue of its corresponding scheduler.
		 *
		 *	Occurrences when this invariant might be violated:
		 *	- When adding a monitor as a service, the add_service call in the registry doesn't know about the registry.
		 *		Therefore, we add it to the registry when adding a listening monitor service.
		 *	- When a monitor completes (say, having nothing left to listen for), the scheduler remove it from the queue.
		 *		Therefore, when a monitor completes, it's shared_ptr in the Scheduler goes away,
		 *			so the weak_ptr in the follower registry expires.
		 *	- When a registry terminates, it might do so with pending actions in its queue.
		 *		While this is abnormal, it may also be recoverable with a single execution context.
		 *		The entire scheduling queue in terminated scheduler goes out of scope,
		 *			so, as with ordinary monitor completion, the weak_ptr expires.
		 *		Unlike the case of ordinary monitor completion, the destructor for the scheduler
		 *			runs before that of the monitor.
		 *		Since the monitor is ignorant of its scheduler, this makes no difference.
		 */
		static follower_registry_type schedulers ;

	protected:
		/**	\brief Register an action for wakeup
		 *	\post
		 *	- Monitor is registered in the scheduler and will eventually run.
		 */
		void register_for_wakeup( Listener * that, wakeup_listener * w ) ;

	} ;

	//-------------------------
} // end namespace ACT

#endif
