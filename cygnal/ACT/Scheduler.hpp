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

/// \file Scheduler.hpp

#pragma once
#ifndef __Scheduler_hpp__
#define __Scheduler_hpp__

#include "ACT.hpp"
#include "Scheduling_Queue.hpp"
#include <boost/optional/optional.hpp>

//#include "boost/date_time/posix_time/posix_time.hpp"

namespace ACT {

	//using boost::posix_time::ptime ;

	//-------------------------
	/* Forward declarations for Basic_Wakeup_Listener
	 */
	struct Basic_Scheduled_Item ;

	//-------------------------
	class Basic_Wakeup_Listener
		: public wakeup_listener
	{
	public:
		///
		typedef Scheduling_Queue< Basic_Scheduled_Item >::pointer item_pointer ;

		///
		typedef Scheduling_Queue< Basic_Scheduled_Item > * scheduler_pointer ;

	private:
		/// A pointer to the item to reschedule.
		item_pointer the_item ;

		/// We use the scheduler to reorder an item after we change its priority.
		scheduler_pointer the_scheduler ;

	public:
		/// The listener body.
		void operator()() ;

		/// Ordinary constructor used after item is within a scheduling queue and its storage location is known.
		Basic_Wakeup_Listener( item_pointer x, scheduler_pointer y )
			: the_item( x ), the_scheduler( y ) {}
	} ;

	//-------------------------
	/// Task Priorities.
	///
	/// We don't have issues of persistence with these values at the present.
	/// Should we ever want to do so, persisting string representations rather than binary values would be preferable.
	enum Basic_Priority {
		/// Urgent tasks are activated as soon as they're entered.
		/// Shutdown notification, for example, is an urgent task.
		Urgent = 0,

		/// The first activation of an action operates at higher priority than subsequent ones.
		Initial,

		/// A task ready for its next activation is ordinary.
		Ordinary,

		///
		Background,

		///
		Waiting
	} ;

	//-------------------------
	///
	enum Action_Category
	{
		/// An ordinary task, executed until completed, then discarded.
		Task = 0,

		/**	A service task, reset if completed, discarded if reset does not succeed.
		 *
		 *	A service executes with background priority in the scheduling queue.
		 *	It may also have its own autonomous processing.
		 *	The select() call behind the wakeup listener for network I/O, for example, is a service.
		 */
		Service,

		/**	A critical service is one that must be run in order for the scheduler to run.
		 */
		Critical_Service
	} ;

	//-------------------------
	struct Basic_Scheduled_Item
	{
		/// 
		Basic_Priority priority_category ;

		///
		unsigned int sequence_number ;

		///
		act the_action ;

		///
		Basic_Scheduled_Item( act, unsigned int, Basic_Priority = Initial ) ;

		///
		boost::optional< Basic_Wakeup_Listener > listener ;

		/// Comparison operator for the priority queue
		bool operator<( const Basic_Scheduled_Item & ) const ;
	} ;

	//-------------------------
	class Basic_Scheduler
	{
		///
		typedef Scheduling_Queue< Basic_Scheduled_Item >::pointer item_pointer ;

		/// Activate the next action once.
		void activate_one_item() ;

		/// Flag indicating whether the main execution loop should continue.
		bool operating ;

		/// The live activation queue.
		Scheduling_Queue< Basic_Scheduled_Item > the_queue ;

		/// Increasing sequence numbers upon scheduling implements a kind of LRU activation policy.
		unsigned int next_sequence_number ;

		/// Default constructor is private to enforce singleton
		Basic_Scheduler() ;

		///
		static Basic_Scheduler the_instance ;

	public:
		/// Add an ordinary task into the scheduling queue.
		void add_task( act ) ;

		///
		void add_service( act ) ;

		///
		void add_critical_service( act ) ;

		/// The main execution loop.
		void operator()() ;

		/// The main execution loop with an activation bound.
		void operator()( unsigned int ) ;

		///
		bool ordinary_tasks_available() ;

		///
		inline bool empty() { return the_queue.empty() ; }

		///
		void reset() ;

		/// Factory method for singleton pattern
		static Basic_Scheduler & obtain_scheduler() ;
	} ;

} // end namespace ACT

#endif
