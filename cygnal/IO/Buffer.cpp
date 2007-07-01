// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/**	\file Buffer.cpp
 *	
 */

#include "Buffer.hpp"
#include <string>

namespace IO {
	//--------------------------------------------------
	// Null_Buffer
	//--------------------------------------------------
	/* Null_Buffer has no internal state, so there's no need for more than one instance.
	 */
	Null_Buffer< char > Null_Buffer_Singleton ;

	//--------------------------------------------------
	// Abstract_Buffer
	//--------------------------------------------------
	// Explicit instantiation
	template bool Abstract_Buffer< char >::operator<( const Abstract_Buffer< char > & ) const ;

	//-------------------------
	template< class Ch >
	int
	Abstract_Buffer< Ch >::
	compare( const Abstract_Buffer< Ch > & x ) const
	{
		int provisional_result = 0 ;

		const_segment_iterator end( segment_end() ) ;
		const_segment_iterator begin( segment_begin() ) ;
		const_segment_iterator endx( x.segment_end() ) ;
		const_segment_iterator beginx( x.segment_begin() ) ;

		if ( begin == end ) {
			// Assert base is exhausted
			return beginx == endx ? 0 : -1 ;
		}
		if ( beginx == endx ) {
			// Assert param is exhausted and base is not
			return 1 ;
		}
		// Assert begin != end and beginx != endx

		// Don't indirect until after we know it will succeed.
		Segment< Ch > seg( * begin ) ;
		Segment< Ch > segx( * beginx ) ;

		/*	Loop Invariant
		 *	- begin != end
		 *	- beginx != end
		 *	- prefix of base is equal to the prefix of the parameter
		 */
		while ( true ) {
			size_t size = std::min( seg.length, segx.length ) ;
			int a = strncmp( seg.begin, segx.begin, size ) ;
			if ( a != 0 ) {
				// Assert prefixes of length 'size' do not match.
				return a ;
			}
			// Assert prefixes of length 'size' are equal.

			if ( seg.length < segx.length ) {
				// Assert  prefix of base is shorter than that of parameter
				++ begin ;
				if ( begin == end ) {
					// Assert base is exhausted
					return -1 ;
				}
				seg = * begin ;
				segx.begin += size ;
				segx.length -= size ;

			} else if ( segx.length < seg.length ) {
				// Assert prefix of parameter is shorter than that of base
				++ beginx ;
				if ( beginx == endx ) {
					// Assert param is exhausted
					return 1 ;
				}
				segx = * beginx ;
				seg.begin += size ;
				seg.length -= size ;

			} else {
				// Assert seg.length == segx.length
				++ begin ;
				++ beginx ;
				if ( begin == end ) {
					// Assert base is exhausted
					return beginx == endx ? 0 : -1 ;
				}
				if ( beginx == endx ) {
					// Assert param is exhausted and base is not
					return 1 ;
				}
				// Assert begin != end and beginx != endx
				seg = * begin ;
				segx = * beginx ;
			}
			// Comparison is inconclusive still and at least one comparand is not yet exhausted.
		}
	}

	//--------------------------------------------------
	// simplest_buffer
	//--------------------------------------------------
	// Explicit instantiation
	template result_buffer< char > ;

	//-------------------------
	template< class Ch >
	result_buffer< Ch >::
	result_buffer()
		: start( 0 ),
		total_size( 0 )
	{}

	//-------------------------
	template< class Ch >
	result_buffer< Ch >::
	result_buffer( Ch * start, size_t size )
		: start( start ),
		total_size( size )
	{}

	//-------------------------
	template< class Ch >
	result_buffer< Ch >::
	result_buffer( contiguous_buffer< Ch > b )
		: start( b.begin ),
		total_size( b.length )
	{}

	//-------------------------
	template< class Ch >
	Segment< Ch >
	result_buffer< Ch >::
	get_segment( unsigned int ) const
	{
		return Segment< Ch >( start, total_size ) ;
	}

	//--------------------------------------------------
	// E_AV_Buffer_Base
	//--------------------------------------------------
	//-------------------------
	// Explicit instantiation
	template E_AV_Buffer_Base< char > ;

	//-------------------------
	template< class Ch >
	void 
	E_AV_Buffer_Base< Ch >::
	append( Ch * x, size_t n )
	{
		unsigned int index( n_segments ) ;
		if ( index < n_array ) {
			// Current insertion point is in the static array
			arrayed[ index ] = Segment< Ch >( x, n ) ;
		} else {
			index -= n_array ;
			vectored[ index ] = Segment< Ch >( x, n ) ;
		}
		++ n_segments ;
	}

	template< class Ch >
	size_t 
	E_AV_Buffer_Base< Ch >::
	size() const
	{
		return 0 ;
	}

	template< class Ch >
	typename E_AV_Buffer_Base< Ch >::character_iterator
	E_AV_Buffer_Base< Ch >::
	begin()
	{
		return character_iterator( arrayed[0].begin, * this, 0 ) ;
	}

	template< class Ch >
	Segment< Ch > 
	E_AV_Buffer_Base< Ch >::
	get_segment( unsigned int x ) const 
	{
		if ( x < n_array )
			return arrayed[ x ] ; 
		x -= n_array ;
		if ( x < vectored.size() )
			return vectored[ x ] ;
		throw std::range_error( "" ) ;
	}

} // end namespace IO
