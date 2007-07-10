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

/**	\file Handle.cpp
 */

#include "Handle.hpp"

namespace ACT {
	//---------------
	template< class T, template< class, class > class Aspect >
	typename Handled< T, Aspect >::registry_type
	Handled< T, Aspect >::our_registry ;

	//---------------
	template< class T, template< class, class > class Aspect >
	typename Handle_Registry_Leader< T, Aspect >::handle_type
	Handle_Registry_Leader< T, Aspect >::
	add( T * that )
	{
		handle_type x( 0 ) ;
		if ( free_handles.empty() ) {
			// Assert no existing space in registry vector
			x = handle_type( the_registry.size() ) ;
			the_registry.push_back( that ) ;
			aspect.add_in_new_place() ;
		} else {
			// Assert there's an existing slot in the registry vector
			x = free_handles.back() ;
			free_handles.pop_back() ;
			the_registry[ x ] = that ;
			aspect.add_in_old_place() ;
		}
		return x ;
	}

	template< class T, template< class, class > class Aspect >
	void
	Handle_Registry_Leader< T, Aspect >::
	remove( handle_type x )
	{
		try {
			// Zero out the entry so that a bad index (however that might happen) doesn't get reused.
			the_registry.at( x ) = 0 ;
			free_handles.push_back( x ) ;
		} catch ( ... ) {
		}
	}

	//---------------
	template< class T, class Leader, template< class, class, class > class Aspect >
	typename Handle_Registry_Follower< T, Leader, Aspect >::reference 
	Handle_Registry_Follower< T, Leader, Aspect >::
	operator[]( handle_type x )
	{
		if ( x >= the_registry.size() ) {
			if ( x >= the_registry.capacity() ) {
				// Assert the handle is outside of the allocated bounds of the registry vector.
				// Force a reservation, not knowing how resize() works.
				the_registry.reserve( x + 1 ) ;
				aspect.expand_capacity_of_vector() ;
			}
			// Assert the handle is within the allocated size of the registry vector.
			the_registry.resize( x + 1 ) ;
			aspect.enlarge_size_of_vector() ;
		} else {
			// Assert the handle is already within the bounds of the present vector.
		}
		// Assert handle is within range of 
		aspect.access_from_existing_slot() ;
		return the_registry[ x ] ;
	}

} // end of namespace ACT
