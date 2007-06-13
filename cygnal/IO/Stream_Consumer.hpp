// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/**	\file Stream_Consumer.hpp
 *	\brief Assistance for treating ACT block readers as a stream.
 *
 *	Implementing a stream model with ACT-based I/O requires resolving a number of competing concerns.
 *	The Stream_Consumer base class provides a common implementation that does so.
 *	The concerns in the present implementation are as follows.
 *	- Asynchrony (non-blocking I/O with ACT control discipline)
 *	- Zero-copy buffering (vector of segments controlled by the source)
 *	- Lookahead replenishment (reading characters not yet required)
 *	- Lookahead access (reading across potential segment gaps)
 *	- End-of-file pseudocharacter
 *
 */

#pragma once
#ifndef __Stream_Consumer_hpp___
#define __Stream_Consumer_hpp___

#include "IO_Device.hpp"
#include "ACT/ACT.hpp"

namespace IO {
	//-------------------------
	/**	\class Stream_Consumer
	 *	\brief Assistant base class for consuming read actions as a stream.
	 *
	 *	To use this class, derive from it.
	 *	The principal members of this class are a scan pointer, a current segment length, and a function to replenish them.
	 *	Also provided are a number of convenience methods to use at specific points in a scanner.
	 *
	 *	This class is written to make ACT stream readers easier to write.
	 *	It does not, however, impose any particular control pattern.
	 *	Certain members are marked [ACT state].
	 *
	 *	Note that there are no public members of this class.
	 *	It is meant as an implementation assistant, so it doesn't provide any public services, only implementation one.
	 *	As a result, the 'protected' designation is appropriate.
	 */
	class Stream_Consumer
		: public ACT::autonomous_act
	{
		/// Ultimate source of characters
		IO::Source * the_source ;

		/// [ACT state] If lookahead required, the number of lookahead characters we need.
		unsigned int number_of_lookahead_characters_required ;

		/// Polymorphic scanner.
		virtual ACT::act_state scan( ACT::wakeup_listener * ) =0 ;

		/// Initial action
		virtual void init() {} ;

		/// [ACT in out] Location to update scan point to.
		/// Zero for top-level, non-zero for subroutine.
		char * & caller_next_character ;
			
		/// [ACT in out] Location to update scan point to
		/// Zero for top-level, non-zero for subroutine.
		size_t & caller_n_left_to_process ;

	protected:
		/// [ACT state] Next character to parse.
		char * next_character ;

		/// [ACT state] number of characters in the buffer we've not seen.
		size_t n_left_to_process ;

		/// Top-level constructor
		Stream_Consumer( IO::Source * source ) ;

		/// Subroutine constructor
		Stream_Consumer( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process ) ;

		/// An encapsulation of the input loop that converts a block interface into an interrupted-stream one.
		void replenish( ACT::wakeup_listener * w ) ;

		/// [replenish result]
		inline bool source_still_working() { return the_source -> working() ; }

		/// [replenish result]
		inline bool source_went_bad() { return the_source -> bad() ; }

		///
		inline void require_lookahead( unsigned int n ) { number_of_lookahead_characters_required = n ; }

		/// Consume characters up to where next_character points.
		inline void consume() { the_source -> consume_up_to( next_character ) ; }

		// TEMPORARY
		inline bool known_eof() { return the_source -> known_eof() ; }
		inline bool characters_available( size_t n = 1 ) { return the_source -> characters_available( n ) ; }
		inline IO::contiguous_buffer<> next_segment() { return the_source -> next_segment() ; }

		// Should be in act_basic as an initial state
		bool first_time ;

		/// [error] Indicate a syntax error at the current location.
		ACT::act_state syntax_error() { return set_bad() ; }

	public:
		/// The class implements the ACT invocation.
		/// This function wraps our own polymorphic \c run.
		ACT::act_state run( ACT::wakeup_listener * ) ;

	} ;

} // end namespace IO

#endif	// end of inclusion protection
