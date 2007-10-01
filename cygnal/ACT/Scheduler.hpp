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

/// \file Scheduler.hpp

#pragma once
#ifndef __Scheduler_hpp__
#define __Scheduler_hpp__

#include "ACT.hpp"
#include "Handle.hpp"
#include "Scheduling_Queue.hpp"
#include <boost/optional/optional.hpp>
#include "../Aspect.hpp"
//#include "boost/date_time/posix_time/posix_time.hpp"

// Declarations within this namespace block intended for a new header file defining only interfaces
namespace ACT {
	//-------------------------
	/**	\class Scheduler
	 *	\brief Abstract interface to scheduler.
	 */
	class Scheduler
		: public Handled< Scheduler >
	{
	public:
		/// Add a task with bounded lifetime.
		virtual void add_task( act ) =0 ;

		/// Add a service with indefinite lifetime.
		virtual void add_service( act ) =0 ;

		///
		virtual bool ordinary_tasks_available() =0 ;

		///
		Scheduler( Scheduler * that )
			: Handled< Scheduler >( that )
		{} ;
	} ;

	/** \class wakeup_listener
	 *	\brief Abstract interface to a function object that wakes up a listener task that's waiting.
	 */
	class wakeup_listener
	{
	public:
		/// The listener body.
		virtual void operator()() =0 ;

		/// Accessor for the scheduler
		virtual Scheduler * scheduler() const =0 ;
	} ;
}

namespace ACT {
	//-------------------------
	/* Forward declarations for wakeup_listener
	 */
	struct Basic_Scheduled_Item ;
	template< template< class > class > class Basic_Scheduler ;

	//-------------------------
	/**	\class wakeup_listener_allocated
	 *	\brief A managed-storage wrapper for wakeup listener as it's used with the Scheduling_Queue
	 *
	 *	\par Plans
	 *	This class uses the ordinary heap for allocation on an item-by-item basis.
	 *	Since we know that we must have one object of this class allocated for each task,
	 *		we can optimize this memory use by allocating in bulk and providing a custom allocator.
	 *	On the other hand, since these objects are allocated only during the growth phase,
	 *		this allocation strategy has no performance penalty in the steady state,
	 *		except to the extent that excess memory usage causes unnecessary swapping.
	 */
	template< class S >
	class wakeup_listener_allocated
	{
	public:
		///
		typedef S * scheduler_pointer ;
		// typedef wakeup_listener::scheduler_pointer scheduler_pointer ;

		///
		typedef scheduler_pointer constructor_parameter ;

	private:
		/// The wrapped pointer
		///
		/// I'm using a shared_ptr here because this class must have a copy constructor for std::vector resizing.
		/// An auto_ptr won't work with the default copy constructor.
		boost::shared_ptr< wakeup_listener > the_wakeup_listener ;

	public:
		/// Ordinary constructor used after item is within a scheduling queue and its storage location is known.
		wakeup_listener_allocated( size_t x, scheduler_pointer y ) ;

		///
		wakeup_listener * get() const { return the_wakeup_listener.get() ; }

		///
		inline void reconstruct( scheduler_pointer ) {} ;
	} ;

	//-------------------------
	/** \class Basic_Wakeup_Listener
	 *	\brief The 'A' in ACT means 'asynchronous', so in general there must be a way of notifying
	 *		a scheduler that an inactive ACT, one that has a pending sub-action, is ready to proceed.
	 *	This class is the interface between a scheduler and such an ACT.
	 *
	 *	The archetype of a notification is that from an asynchronous system I/O call.
	 *	These notifications arrive various by event, queue item, callback, signal, and others.
	 *	Notification is a mechanism crying out for implementation hiding by means of an abstraction.
	 *	This type provides a standard way to do so.
	 *
	 *	The implementation pattern is an ACT call a wakeup_listener and that a scheduler
	 *		provide the function so called.
	 *	The scheduler should encapsulate its own notification receiver, however structured,
	 *		into a function object of this class, say, by binding a member function adapter.
	 */
	template< class S >
	class Basic_Wakeup_Listener
		: public wakeup_listener
	{
	public:
		///
		typedef S * scheduler_pointer ;

	private:
		///
		typedef Scheduling_Queue< Basic_Scheduled_Item, wakeup_listener_allocated< S > > queue_type ;

		/// A pointer to the item to reschedule.
		size_t permutation_index ;

		/// We use the scheduler to reorder an item after we change its priority.
		scheduler_pointer the_scheduler ;

	public:
		/// The listener body.
		void operator()() ;

		/// Accessor for the scheduler
		inline scheduler_pointer scheduler() const { return the_scheduler ; }

		/// Ordinary constructor used after item is within a scheduling queue and its storage location is known.
		Basic_Wakeup_Listener( size_t x, scheduler_pointer y )
			: permutation_index( x ), the_scheduler( y ) {}
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
	/** \brief Category of actions within Basic_Scheduler.
	 *		Distinguishes ordinary tasks from services, which have different scheduling policies.
	 */
	enum Action_Category
	{
		/// An ordinary task, executed until completed, then discarded.
		Task = 0,

		/**	A demon task, never expected to complete, but which may complete.
		 *
		 *	A demon executes with background priority in the scheduling queue.
		 *	It may also have its own autonomous processing.
		 *	The select() call behind the wakeup listener for network I/O, for example, is somewhere within a certain demon.
		 */
		Demon,

		/**	A critical demon is one that must be run in order for the scheduler to run.
		 */
		Critical_Demon
	} ;

	//-------------------------
	/**	\class Basic_Scheduled_Item
	 *	\brief Data within the priority queue proper.
	 *		This data is swapped during priority queue operations, so should remain as small as possible.
	 *
	 *	\par Future
	 *		It seems that \c the_action, which isn't needed for ordering, might be able to move to the auxiliary vector.
	 */
	struct Basic_Scheduled_Item
	{
		///
		Action_Category action_type ;

		/// 
		Basic_Priority priority_category ;

		///
		unsigned int sequence_number ;

		///
		act the_action ;

		///
		Basic_Scheduled_Item( act x, unsigned int n, Action_Category action_type = Task ) ;

		/// Comparison operator for the priority queue
		bool operator<( const Basic_Scheduled_Item & ) const ;
	} ;

	//-------------------------
	/**	\class Basic_Scheduler
	 *	\brief A basic implementation of a scheduler.
	 *
	 *	This may not be the final production version of a scheduler.
	 *	Nevertheless, a scheduler which is as simple as possible, but still works as a scheduler,
	 *		should be retained for testing.
	 *	In particular, this scheduler does not have a timeout mechanism to detect stalled actions.
	 *	This would be a defect in a production environment.
	 *	Such an absence should be retained for the test environment, to be able to isolate defective actions that stall.
	 */
	template< template< class > class Aspect = aspect::Null_Aspect_0 >
	class Basic_Scheduler
		: public Scheduler
	{
		///
		friend class Basic_Wakeup_Listener< Basic_Scheduler > ;

		/// The type of our internal queue.
		typedef Scheduling_Queue< Basic_Scheduled_Item, wakeup_listener_allocated< Basic_Scheduler > > queue_type ;

		///
		typedef typename queue_type::pointer item_pointer ;

		/// Activate the next action once.
		void activate_one_item() ;

		/// Flag indicating whether the main execution loop should continue.
		bool operating ;

		/// The live activation queue.
		queue_type the_queue ;

		/// Getter method for wakeup_listener
		inline queue_type & queue() { return the_queue ; }

		/// Increasing sequence numbers upon scheduling implements a kind of LRU activation policy.
		unsigned int next_task_sequence_number ;

		/// A separate sequence for services implements round-robin activation.
		unsigned int next_service_sequence_number ;

		///
		typedef Aspect< Basic_Scheduler > aspect_type ;

		///
		aspect_type aspect ;

	public:
		/// Default constructor is private to enforce singleton
		Basic_Scheduler( aspect_type aspect = aspect_type() ) ;

		/// Add an ordinary task into the scheduling queue.
		void add_task( act ) ;

		///
		void add_service( act ) ;

		///
		void add_critical_service( act ) ;

		/// The main execution loop.
		void operator()() ;

		///
		bool ordinary_tasks_available() ;

		///
		inline bool empty() { return the_queue.empty() ; }

		///
		void reset() ;
	} ;

	//-------------------------
	/** \class Basic_Scheduler_Null_Aspect
	 *	\brief Base class for aspects of \c Basic_Scheduler.
	 */
	class Basic_Scheduler_Null_Aspect
	{
	public:
		/// Guard must be true in order for execution loop to continue.
		inline bool run_guard() { return true ; }

		/// Called before any execution starts.
		inline void run_begin() {}

		/// Called after execution stops through internal means.
		inline void run_end_ordinary() {}

		/// Called if execution stop because of guard failure.
		inline void run_end_guard_violation() {}
	} ;

	//-------------------------
} // end namespace ACT

namespace aspect {
	//-------------------------
	/** \brief Default null aspect for Basic_Scheduler.
	 */
	template<>
	class Null_Aspect_0< ACT::Basic_Scheduler< Null_Aspect_0 > >
		: public Null_Aspect_Base< ACT::Basic_Scheduler< Null_Aspect_0 > >,
		public ACT::Basic_Scheduler_Null_Aspect
	{} ;

}

#endif
