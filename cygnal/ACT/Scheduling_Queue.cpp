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

/// \file Scheduling_Queue.cpp
/// \brief A scheduling queue, a data structure used by an actual scheduler, provides basic storage and manipulation.

#include "Scheduling_Queue.hpp"

// All we need is Scheduled_Item.  Pare down this inclusion should it become possible to do so.
#include "Scheduler.hpp"

namespace ACT {
	//-------------------------
	template< class T >
	Scheduling_Queue< T >::
	Scheduling_Queue()
		: n_queue_items( 0 )
	{}

	//-------------------------
	/*
The problem here is that the free list isn't ordered as it should be.
Or else the problem here is that something.
Blargh.
	 */
	template< class T >
	typename Scheduling_Queue< T >::pointer
	Scheduling_Queue< T >::
	push( const_reference item )
	{
		size_t permutation_index ;
		// The new queue-index in both cases is n_queue_items, which is incremented afterwards.
		if ( n_queue_items == the_queue.size() ) {
			// Assert the_queue has no deleted elements
			// In this case the permutation index is the mutual size of the queue and the permutation.
			permutation_index = n_queue_items ;
			the_queue.push_back( Scheduled_Item< T >( item, permutation_index ) ) ;	// operator[] would throw here
			permutation.push_back( n_queue_items ) ;								// ditto
		} else {
			// Assert n_queue_items < the_queue.size()
			// Assert the_queue has at least one deleted element
			// In this case the permutation index to use it that of the first deleted element
			permutation_index = the_queue[ n_queue_items ].permutation_index ;
			the_queue[ n_queue_items ] = Scheduled_Item< T >( item, permutation_index ) ;	// note that the Scheduled_Item is the same as above
			permutation[ permutation_index ] = n_queue_items ;
		}
		++ n_queue_items ;

		size_t n = up_heap( n_queue_items - 1 ) ;
		return pointer( n, this ) ;
	}

	//-------------------------
	template< class T >
	void
	Scheduling_Queue< T >::
	pop()
	{
		-- n_queue_items ;
		swap( 0, n_queue_items ) ;
		down_heap( 0 ) ;
	}

	//-------------------------
	template< class T >
	void
	Scheduling_Queue< T >::
	reorder( pointer p )
	{
		size_t z = permutation[ p.index ] ;
		size_t x = down_heap( z ) ;
		if ( z != x ) return ;
		(void) up_heap( z ) ;
	}

	//-------------------------
	template< class T >
	void
	Scheduling_Queue< T >::
	swap( size_t x, size_t y )
	{
		// Swap the items
		Scheduled_Item< T > tmp = the_queue[ x ] ;
		the_queue[ x ] = the_queue[ y ] ;
		the_queue[ y ] = tmp ;

		// Update the permutation
		permutation[ the_queue[ x ].permutation_index ] = x ;
		permutation[ the_queue[ y ].permutation_index ] = y ;
	}

	//-------------------------
	/** \par Implementation
	 *	This algorithm treat the_queue as an implicit binary heap.
	 */
	template< class T >
	size_t
	Scheduling_Queue< T >::
	up_heap( size_t item )
	{
		//	Guards:
		//	- item > 0
		//	- parent( item ) < item
 		while ( item > 0 ) {
			size_t parent = ( item - 1 ) / 2 ;
			if ( the_queue[ parent ] < the_queue[ item ] ) {
				swap( parent, item ) ;
			} else {
				// heap invariant is reestablished
				break ;
			}
			item = parent ;
		}
		return item ;
	}

	//-------------------------
	/** \par Implementation
	 *	This algorithm treat the_queue as an implicit binary heap.
	 */
	template< class T >
	size_t
	Scheduling_Queue< T >::
	down_heap( size_t item )
	{
		//
		while ( true ) {
			size_t child = 2 * item + 1 ;
			// Assert child is the index of the first child of item
			if ( child >= n_queue_items ) {
				// Assert item has no children
				break ;
			} else if ( child + 1 >= n_queue_items ) {
				// Assert item has exactly one child
			} else {
				// Assert item has two children
				if ( the_queue[ child ] < the_queue[ child + 1 ] ) {
					++ child ;
				}
			}
			// Assert child is not less than the other child, if it exists.
			if ( the_queue[ item ] < the_queue[ child ] ) {
				swap( item, child ) ;
			} else {
				// Assert item is not less than either of its children
				break ;
			}
			item = child ;
		}
		return item ;
	}


} // end namespace ACT
