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

/**	\file Buffer.hpp
 *	\brief Buffers, simple and vectored
 *
 *	Buffer discipline is at the core of zero-copy techniques.
 *	This buffer class presents the abstraction of a sequence of characters.
 *	The underlying data representation, however, need not be a simple sequence.
 *
 *	A buffer is a virtual sequence of characters, divided up into a sequence of segments.
 *	Characters in the same segment are adjacent to each other in memory.
 *	There are different modes of allocating and managing segments.
 *	Implementation classes of Abstract_Buffer may use any of them, in any combination.
 *
 *	Allocation of the underlying segments of memory:
 *	- statically allocation, typical for constant strings
 *	- dynamically allocated on the general heap
 *	- allocated from special store
 *
 *	Allocation of the segment data (pointer/size) as class member
 *	- ordinary single member
 *	- array member
 *	- dynamically allocated array
 *	- std::vector
 */
#pragma once
#ifndef __Buffer_hpp___
#define __Buffer_hpp___

#include <vector>

namespace IO {
	//--------------------------------------------------
	// Contiguous Buffer
	//--------------------------------------------------
	/** \class contiguous_buffer
	 *	\brief The simplest buffer possible, a single half-open interval [ begin, begin + length ).
	 *		Or, if you will, a concrete sequence of characters in memory.
	 *
	 *	A contiguous buffer is the result type for read actions.
	 *	It is the type for static constants, such as field names, commands, etc.
	 *	It doesn't have any fancy operators; those are to hide complexity of something complex, not like this.
	 *	And it's a struct, because everything's public.
	 *	We don't want to hide anything.
	 *
	 *	Not-hiding-anything is by design.
	 *	The reason it's the result type for read action is to enable scanners to operator efficiently,
	 *		using direct indirection on pointers rather than smart (thus slower) indirection on a bounds-check pointer type.
	 *	Callers of read actions use the elements of this structure directly to achieve this efficiency.
	 */
	template< class Ch = char >
	struct contiguous_buffer
	{
		/// Pointer to first character of sequence.
		Ch * begin ;

		/// Total length of sequence.
		size_t length ;

		/// Ordinary constructor is as obvious as possible.
		/// Default constructor parameters are all zero.
		contiguous_buffer( Ch * begin = 0, size_t length = 0 )
			: begin( begin ),
			length( length )
		{}

		/// Convenience method for finding the upper bound
		inline Ch * end() { return begin + length ; }
	} ;

	//-------------------------
	/**	\class Segment
	 */
	template< class Ch >
	struct Segment
	{
		Ch * begin ;
		size_t length ;

		Segment( Ch * x = 0, size_t n = 0 ) 
			: begin( x ), length( n )
		{}
	} ;

	//--------------------------------------------------
	/* Forward declarations for Abstract_Buffer
	 */
	template< class Ch > class Basic_Character_Iterator ;
	template< class Ch > class Basic_Segment_Iterator_Base ;
	template< class Ch > class Basic_Segment_Iterator ;
	template< class Ch > class Basic_Const_Segment_Iterator ;

	//--------------------------------------------------
	// Abstract_Buffer
	//--------------------------------------------------
	/** \class Abstract_Buffer
	 *	\brief Base class for buffers
	 */
	template< class Ch >
	class Abstract_Buffer
	{
		template< class Ch > friend class Basic_Segment_Iterator_Base ;

	protected:
		virtual Segment< Ch > get_segment( unsigned int ) const =0 ;

	public:
		//-------------------------
		// operations
		//-------------------------
		virtual void append( Ch *, size_t ) =0 ;

		//-------------------------
		// properties
		//-------------------------
		/// Total number of characters in virtual sequence.
		virtual size_t size() const =0 ;

		//-------------------------
		// character iteration
		//-------------------------
		/// Character iterator type
		typedef Basic_Character_Iterator< Ch > character_iterator ;

		///
		virtual character_iterator begin() =0 ;

		//-------------------------
		// segment iteration
		//-------------------------
		/// Segment iterator type
		typedef Basic_Segment_Iterator< Ch > segment_iterator ;
		/// Constant segment iterator type
		typedef Basic_Const_Segment_Iterator< Ch > const_segment_iterator ;

		///
		virtual const_segment_iterator segment_begin() const =0 ;
		///
		virtual const_segment_iterator segment_end() const =0 ;

		//-------------------------
		// comparison
		//-------------------------
		/// Compare two buffers as character sequences.
		int compare( const Abstract_Buffer< Ch > & ) const ;

		/// Less-than operator on character sequences.
		inline bool operator<( const Abstract_Buffer< Ch > & x ) const { return compare( x ) < 0 ; }

		/// Equality operator on character sequences.
		inline bool operator==( const Abstract_Buffer< Ch > & x ) const { return compare( x ) == 0 ; }
	} ;

	//--------------------------------------------------
	// Basic_Character_Iterator
	//--------------------------------------------------
	/** \class Basic_Character_Iterator
	 *	\brief Iterator for characters in Abstract_Buffer
	 *
	 *	Representation of an Iterator
	 *	- character pointer: what the iterator eventually resolves to
	 *	- buffer reference: because we don't want a registry of all buffers
	 *	- segment index: an optimization, the index of the segment within our buffer which contains the pointer
	 *	Note that this representation works for any sequence of character segments, 
	 *		no matter how the segments are allocated.
	 *	This allows us a single iterator for a variety of related Basic_Buffer types.
	 */
	template< class Ch >
	class Basic_Character_Iterator
	{
		/// The character pointer to which this iterator ultimately resolves.
		Ch * character_pointer ;

		/// The buffer for which this instance is an iterator.
		Abstract_Buffer< Ch > & our_buffer ;

		/// The index of the segment within our buffer which contains the character pointer.
		unsigned int segment_index ;

	public:
		/// 
		Basic_Character_Iterator( Ch * x, Abstract_Buffer< Ch > & y, unsigned int z )
			: character_pointer( x ),
			our_buffer( y ),
			segment_index( z )
		{}

		/// Indirection
		inline Ch operator*() { return * character_pointer ; }

		/// Retrieval
		inline Ch * get() const { return character_pointer ; }

		/// Checked prefix increment operator.
		///
		/// Known Defect: Increment operator is not bounds-checked.
		inline Basic_Character_Iterator & operator++()
		{
			++ character_pointer ;
			return * this ; 
		}

		/// Checked postfix increment operator.
		///
		/// Known Defect: Increment operator is not bounds-checked.
		inline Basic_Character_Iterator operator++(int)
		{
			Basic_Character_Iterator tmp = * this ;
			++ character_pointer ;
			return tmp ; 
		}

		/// Checked prefix decrement operator.
		///
		/// Known Defect: Increment operator is not bounds-checked.
		inline Basic_Character_Iterator & operator--()
		{
			-- character_pointer ;
			return * this ;
		}

		/// Checked postfix decrement operator.
		///
		/// Known Defect: Increment operator is not bounds-checked.
		inline Basic_Character_Iterator operator--(int)
		{
			Basic_Character_Iterator tmp = * this ;
			-- character_pointer ;
			return tmp ; 
		}
	} ;

	//--------------------------------------------------
	// Basic_Segment_Iterator_Base
	//--------------------------------------------------
	template< class Ch >
	class Basic_Segment_Iterator_Base
	{
		/// The buffer for which this instance is an iterator.
		/// This member is constant except for construction and assignment.
		const Abstract_Buffer< Ch > & our_buffer ;

		/// The index of the segment within our buffer which contains the character pointer.
		unsigned int segment_index ;

	protected:
		Basic_Segment_Iterator_Base( const Abstract_Buffer< Ch > & y, unsigned int z )
			: our_buffer( y ), segment_index( z ) {}

		Segment< Ch > operator*() {
			return our_buffer.get_segment( segment_index ) ;
		}

	public:
		bool operator==( const Basic_Segment_Iterator_Base & x ) const {
			return ( & our_buffer == & x.our_buffer ) && ( segment_index == x.segment_index ) ;
		}

		bool operator!=( const Basic_Segment_Iterator_Base & x ) const {
			return ( & our_buffer != & x.our_buffer ) || ( segment_index != x.segment_index ) ;
		}

		/// Prefix increment operator.
		inline Basic_Segment_Iterator_Base & operator++()
		{
			++ segment_index ;
			return * this ; 
		}

	} ;

	//--------------------------------------------------
	// Basic_Const_Segment_Iterator
	//--------------------------------------------------
	/** \class Basic_Const_Segment_Iterator
	 *	\brief Iterator for segments in Abstract_Buffer
	 *
	 *	\invariant
	 *	- 
	 */
	template< class Ch >
	class Basic_Const_Segment_Iterator
		: public Basic_Segment_Iterator_Base< Ch >
	{
	public:
		/// 
		Basic_Const_Segment_Iterator( const Abstract_Buffer< Ch > & y, unsigned int z )
			: Basic_Segment_Iterator_Base( y, z )
		{}

		inline const Segment< Ch > operator*() { return Basic_Segment_Iterator_Base::operator*() ; }
	} ;

	//--------------------------------------------------
	// Basic_Segment_Iterator
	//--------------------------------------------------
	/** \class Basic_Segment_Iterator
	 *	\brief Iterator for segments in Abstract_Buffer
	 *
	 *	\invariant
	 *	- 
	 */
	template< class Ch >
	class Basic_Segment_Iterator
		: public Basic_Const_Segment_Iterator< Ch >
	{
	public:
		Basic_Segment_Iterator( const Abstract_Buffer< Ch > & y, unsigned int z )
			: Basic_Const_Segment_Iterator( y, z )
		{}

		inline Segment< Ch > operator*() { return Basic_Segment_Iterator_Base::operator*() ; }

	} ;

	//--------------------------------------------------
	// result_buffer
	//--------------------------------------------------
	/** \class result_buffer
	 *	\brief A buffer specialized to hold results of scanning input buffers.
	 */
	template< class Ch = char >
	class result_buffer
		: public Abstract_Buffer< Ch >
	{
		Ch * start ;
		size_t total_size ;
		Segment< Ch > get_segment( unsigned int ) const ;

	public:
		/// Public setter for use by scanners
		void set_begin( Ch * x ) { start = x ; total_size = 0 ; }
		/// Public setter for use by scanners
		void set_length( size_t n ) { total_size = n ; }
		/// Public incrementer for use by scanners
		void incr_length() { ++ total_size ; }

		//-------------------------
		// constructors
		//-------------------------
		/// Construct a buffer from a pointer and a size.
		result_buffer( Ch *, size_t size ) ;

		/// Construct a buffer from an existing contiguous buffer
		result_buffer( contiguous_buffer< Ch > b ) ;

		/// Default constructor makes an empty buffer.
		result_buffer() ;

		//-------------------------
		// operations
		//-------------------------
		void append( Ch *, size_t ) { throw std::range_error( "" ) ; }

		//-------------------------
		// properties
		//-------------------------
		/// Total number of characters in virtual sequence.
		size_t size() const { return total_size ; }

		//-------------------------
		// character iteration
		//-------------------------
		/// 
		character_iterator begin() { return character_iterator( start, * this, 0 ) ; }

		//-------------------------
		// segment iteration
		//-------------------------
		/// Segment iterator type
		segment_iterator segment_begin() { return segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_begin() const { return const_segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_end() const { return const_segment_iterator( * this, 1 ) ; }

	} ;

	//--------------------------------------------------
	// E_AV_Buffer
	//--------------------------------------------------
	/**	\class E_AV_Buffer_Base
	 *	\brief 
	 */
	template< class Ch = char >
	class E_AV_Buffer_Base
		: public Abstract_Buffer< Ch >
	{
		/// Pointer to first element of array of segments
		Segment< Ch > * arrayed ;

		unsigned int n_array ;

		/// Vector of segments
		std::vector< Segment< Ch > > vectored ;

		/// Number of segments in the segment-sequence
		unsigned int n_segments ;

		/// Number of characters in the character-sequence
		unsigned int n_characters ;

	protected:
		E_AV_Buffer_Base( Segment< Ch > * x, unsigned int n_array, unsigned int n_vector )
			: arrayed( x ),
			n_array( n_array ),
			vectored( n_vector ),
			n_segments( 0 )
		{}

	public:
		void append( Ch *, size_t ) ;
		size_t size() const ;
		character_iterator begin() ;
		segment_iterator segment_begin() { return segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_begin() const { return const_segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_end() const { return const_segment_iterator( * this, n_segments ) ; }
		Segment< Ch > get_segment( unsigned int x ) const ;		
	} ;

	//-------------------------
	/**	\class E_AV_Buffer
	 *	\brief Externally allocated segments. Array and vectored segment data in instance.
	 *		Only the constructors are defined in this class.  All others are derived from E_AV_Buffer_Base.
	 */
	template< unsigned int N_Array = 1, unsigned int N_Vector = 1, class Ch = char >
	class E_AV_Buffer
		: public E_AV_Buffer_Base< Ch >
	{
		/// Array of segments
		Segment< Ch > arrayed[ N_Array ] ;
	public:
		/// Default constructor starts off empty
		E_AV_Buffer()
			: E_AV_Buffer_Base( arrayed, N_Array, N_Vector )
		{}
	} ;

	//--------------------------------------------------
	// Null_Buffer
	//--------------------------------------------------
	/** \class Null_Buffer
	 *	\brief Provides a permanently empty buffer used for default constructor of value class.
	 */
	template< class Ch >
	class Null_Buffer
		: public Abstract_Buffer< Ch >
	{
		Segment< Ch > get_segment( unsigned int ) const { throw std::range_error( "" ) ; }
	public:
		void append( Ch *, size_t ) {}
		size_t size() const { return 0 ; }
		character_iterator begin() { return character_iterator( 0, * this, 0 ) ; }
		segment_iterator segment_begin() { return segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_begin() const { return const_segment_iterator( * this, 0 ) ; }
		const_segment_iterator segment_end() const { return const_segment_iterator( * this, 0 ) ; }
	} ;

	extern Null_Buffer< char > Null_Buffer_Singleton ;

	//--------------------------------------------------
	// Basic_Buffer
	//--------------------------------------------------
	template< class Ch >
	class Basic_Buffer
	{
		/// wrapped reference to buffer
		Abstract_Buffer< Ch > * the_buffer ;

	public:
		/// Constructor converts a particular storage strategy.
		Basic_Buffer( Abstract_Buffer< Ch > & x )
			: the_buffer( & x )
		{}

		/// Constructor converts a particular storage strategy.
		Basic_Buffer()
			: the_buffer( & Null_Buffer_Singleton )
		{}

		/// assignment
		inline Basic_Buffer & operator=( const Basic_Buffer & x )
		{
			the_buffer = x.the_buffer ; 
			return * this ;
		}

		/// append a segment to the current buffer
		inline void append( Ch * x, size_t n ) { the_buffer -> append( x, n ) ; }

		/// total size of the buffer (the sum of the lengths of all segments)
		inline size_t size() const { return the_buffer -> size() ; }

		/// character iteration begin
		typedef typename Abstract_Buffer< Ch >::character_iterator character_iterator ;

		inline character_iterator begin() { return the_buffer -> begin() ; }

		/// Segment iterator type
		typedef Basic_Segment_Iterator< Ch > segment_iterator ;
		/// Constant segment iterator type
		typedef Basic_Const_Segment_Iterator< Ch > const_segment_iterator ;
		/// Constant iterator to first segment
		inline const_segment_iterator segment_begin() const { return the_buffer -> segment_begin() ; }
		/// Constant iterator to last-plus-one segment
		inline const_segment_iterator segment_end() const { return the_buffer -> segment_end() ; }

		/// Compare two buffers as character sequences.
		inline int compare( const Abstract_Buffer< Ch > & x ) const { return the_buffer -> compare( x ) ; }
		/// Less-than operator on character sequences.
		inline bool operator<( const Abstract_Buffer< Ch > & x ) const { return compare( x ) < 0 ; }
		/// Equality operator on character sequences.
		inline bool operator==( const Abstract_Buffer< Ch > & x ) const { return compare( x ) == 0 ; }
	} ;
	//--------------------------------------------------
	// Type definitions for ordinary use.
	//--------------------------------------------------
	typedef Basic_Buffer< char > buffer ;

} // end of namespace IO

#endif	// end of inclusion protection