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

/** \file Scheduling_Queue.hpp
 *	\brief A data structure on top of which to implement specific scheduling policies.
 *
 *	The Scheduling_Queue class provide the basic operations required for an asynchronous scheduler:
 *	- push: Insert a new item into the queue.
 *	- pop: Remove the highest-priority element
 *	- top: Access to the highest priority element
 *	- reorder: Change the order of an element when its priority changes.
 *	- pointer: Provide a stable way of accessing a particular element, even as its order is changing.
 *	The basic data structure within a Scheduling_Queue is a priority queue.
 *	A Scheduling_Queue adds a means of tracking items within the priority queue in order to provide pointers.
 *	Reordering is possible after pointers are implemented, since then its possible to access items other than the top one.
 *
 *	This class sits between a Scheduler and a priority queue as a stable interface.
 *	A Scheduling_Queue may use any kind of priority queue implementation.
 *	An implicit binary tree is a baseline; other structure may be more efficient in practice.
 *	Different behavior distributions will affect performance in practice.
 *	This class, then, acts as a maintenance boundary to be able to experiment with various algorithms and data structures.
 */

#pragma once
#ifndef __Scheduling_Queue_hpp__
#define __Scheduling_Queue_hpp__

#include "ACT.hpp"

#include <vector>
#include <boost/pending/mutable_queue.hpp>

namespace ACT {
	using boost::mutable_queue ;

	//-------------------------
	/** \class Scheduled_Item
	 *	\brief The full data that goes into the scheduling queue.
	 *		It includes the underlying item plus extra housekeeping information.
	 */
	template< class T >
	struct Scheduled_Item
	{
		/// This structure wraps an item that is of interest to the user of this class.
		T the_underlying_item ;

		/// An index into the permutation array.
		/// The permutation item at this location points back to the location of this item within the item queue.
		size_t permutation_index ;

		/// Ordinary constructor
		Scheduled_Item( T x, size_t y )
			: the_underlying_item( x ), permutation_index( y ) {} ;

		inline bool operator<( const Scheduled_Item & x ) const {
			return the_underlying_item.operator<( x.the_underlying_item ) ;
		}
	} ;


	//-------------------------
	/** \class Auxiliary_Item
	 *	\brief The full data that goes into the scheduling queue.
	 *		It includes the underlying item plus extra housekeeping information.
	 */
	template< class Aux >
	struct Auxiliary_Item
	{
		/// Index into the priority queue.
		size_t priority_queue_index ;

		/// The auxiliary item held at fixed location.
		/// Designed for wakeup_listener.
		Aux the_auxiliary ;

		/// Ordinary constructor
		Auxiliary_Item( size_t x, size_t permutation_index, typename Aux::constructor_parameter z )
			: priority_queue_index( x ), 
			the_auxiliary( permutation_index, z ) 
		{} ;
	} ;

	//-------------------------
	// forward declaration for Scheduled_Item_Pointer
	template< class T, class Aux > class Scheduling_Queue ;
	
	//-------------------------
	/** \class Scheduled_Item_Pointer
	 *	\brief A pointer into an item within a scheduling queue.
	 *	Since items within the underlying container of a queue move around,
	 *		a pointer is necessary to track such motion and provide a stable reference for outsiders.
	 */
	template< class T, class Aux >
	class Scheduled_Item_Pointer
	{
		template< class, class > friend class Scheduling_Queue ;

		size_t index ;

		Scheduling_Queue< T, Aux > * the_scheduling_queue ;

		Scheduled_Item_Pointer( size_t x, Scheduling_Queue< T, Aux > * y )
			: index( x ), the_scheduling_queue( y ) {}
	public:
		inline T & operator*() {
			return the_scheduling_queue -> item( index ) ;
		}

		inline T * operator->() {
			return the_scheduling_queue -> item_ptr( index ) ;
		}
	} ;


	//-------------------------
	/** \class Scheduling_Queue
	 *	\brief A queue responsible for storage allocation, item reference, and heap maintenance.
	 *		This class is a basis on top of which to implement particular scheduling policies.
	 *
	 *	The item queue contains both active items and deleted items.
	 *	The permutation index of a deleted item is still valid.
	 *	The underlying item within a deleted item is meaningless and ignored.
	 *	These extra permutation indices constitute a "free list" of available permutation slots for the creation of pointers.
	 *
	 *	\invariant
	 *	- (bijective permutation) permutation[ the_queue[ x ].permutation_index ].priority_queue_index == x
	 *	- (bijective permutation) the_queue[ permutation[ y ].priority_queue_index ].permutation_index == y
	 *	- n_queue_items <= the_queue.size
	 *	- permutation.size() == the_queue.size()
	 *
	 *	\par Future Directions
	 *	Right now, the storage for the underlying queue and the permutation are separate, 
	 *		even though they are of the same size.
	 *	It might be more efficient later to put them in the same container 
	 *		and provide adapters to simulate separate arrays.
	 */
	template< class T, class Aux >
	class Scheduling_Queue
	{
		// friends
		template< class T1, class Aux1 > friend class Scheduled_Item_Pointer ;
		template< class S > friend class Basic_Wakeup_Listener ;

		/// The main queue for scheduled items.
		std::vector< Scheduled_Item< T > > the_queue ;

		/// The number of active elements in the queue.
		/// This is not the same
		size_t n_queue_items ;

		/// The heap functions maintain a permutation that provide a constant pointer into the item queue.
		std::vector< Auxiliary_Item< Aux > > permutation ;

		/// Swap two elements within the queue.
		/// Adjust the permutation accordingly.
		void swap( size_t, size_t ) ;

		/// Move a new item up the heap as far as it goes.
		size_t up_heap( size_t ) ;

		/// Move a new item down in the heap as far as it goes.
		size_t down_heap( size_t ) ;

		/// Update the order of an item in the heap according to a new priority
		size_t update_heap( size_t x ) {
			size_t y = up_heap( x ) ;
			if ( x == y ) y = down_heap( x ) ;
			return y ;
		}

		/// Item accessor for implementation of indirection in Scheduled_Item_Pointer
		T & item( size_t x ) {
			return the_queue[ permutation[ x ].priority_queue_index ].the_underlying_item ;
		}

		/// Item accessor for implementation of indirection in Scheduled_Item_Pointer
		T * item_ptr( size_t x ) {
			return & the_queue[ permutation[ x ].priority_queue_index ].the_underlying_item ;
		}

	public:
		/// The type of the values contained within the queue.
		typedef T value_type ;

		/// We have a pointer type rather than an iterator type, since we cannot increment through the queue.
		typedef Scheduled_Item_Pointer< T, Aux > pointer ;

		/// A reference to a member of the queue
		typedef value_type & reference ;

		/// A constant reference to a member of the queue.
		typedef const value_type & const_reference ;

		/// Default constructor
		Scheduling_Queue() ;

		/// Add a new item to the queue
		///
		/// Note that the signature of this function differs from that of std::priority_queue, which returns void.
		/// We require a reference to be able to asynchronously wake up an action.
		pointer push( const_reference, const typename Aux::constructor_parameter auxiliary ) ;

		/// Constant access to the top-priority item in the queue.
		reference top() { return the_queue[ 0 ].the_underlying_item ; }

		/// Constant access to the top-priority item in the queue.
		pointer top_ptr() { return pointer( the_queue[ 0 ].permutation_index, this ) ; }

		/// Remove the top item from the queue
		void pop() ;

		/// Reorder the position of the indexed item.  Use this after its priority changes.
		void reorder( size_t ) ;

		/// Reorder the position of the pointed-to item.  Use this after its priority changes.
		void reorder( pointer ) ;

		/// Return is true if queue has no items of any kind in it, waiting or not.
		bool empty() { return n_queue_items == 0 ; }

		///
		inline Aux & auxiliary( size_t x ) { return permutation[ x ].the_auxiliary ; }

		///
		inline Aux * auxiliary_top() { return & permutation[ the_queue[ 0 ].permutation_index ].the_auxiliary ; }
	} ;

} // end namespace ACT

#endif
