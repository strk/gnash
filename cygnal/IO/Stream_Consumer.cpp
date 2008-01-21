// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

/**	\file Stream_Consumer.cpp
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

#include "Stream_Consumer.hpp"

namespace IO {

	//--------------------------------------------------
	// Stream_Consumer
	//--------------------------------------------------

	//-------------------------
	Stream_Consumer::
	Stream_Consumer( IO::Source * source )
		: the_source( source ),
		next_character( 0 ),
		n_left_to_process( 0 ),
		caller_next_character( next_character ),
		caller_n_left_to_process( n_left_to_process ),
		first_time( true )
	{}

	//-------------------------
	Stream_Consumer::
	Stream_Consumer( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process )
		: the_source( source ),
		next_character( 0 ),
		n_left_to_process( 0 ),
		caller_next_character( ref_next_character ),
		caller_n_left_to_process( ref_n_left_to_process ),
		first_time( true )
	{}

	//-------------------------
	ACT::ACT_State
	Stream_Consumer::
	run( ACT::wakeup_listener * w )
	{
		if ( first_time ) {
			// If we're at the top level, these are self-assignments.
			next_character = caller_next_character ;
			n_left_to_process = caller_n_left_to_process ;

			// We require that the scan-point be valid when we call the scanner's initialization.
			//
			if ( n_left_to_process == 0 ) {
				// Input loop is all right here
				replenish( w ) ;

				// Check the two control-oriented results.
				if ( source_still_working() ) {
					// Assert our underlying source is still working.
					// Nothing left to do now.
					return the_source -> internal_state() ; 
				}
				if ( source_went_bad() ) {
					// Assert our underlying source went bad.
					// Therefore we also go bad.
					return set_bad() ;
				}
			}

			// Subclass-specific initialization
			init() ;

			// Set flag at end so that we can return to call replenish, if necessary.
			first_time = false ;
		}

		// Scanning loop
		while ( true ) {
			if ( n_left_to_process == 0 ) {
				// Input loop is all right here
				replenish( w ) ;

				// Check the two control-oriented results.
				if ( source_still_working() ) {
					// Assert our underlying source is still working.
					// Nothing left to do now.
					return the_source -> internal_state() ; 
				}
				if ( source_went_bad() ) {
					// Assert our underlying source went bad.
					// Therefore we also go bad.
					return set_bad() ;
				}
			}

			// Polymorphic scan
			scan( w ) ;

			if ( completed() ) {
				// We only pass the scan point upwards for subroutines
				if ( caller_next_character != 0 ) {
					caller_next_character = next_character ;
					caller_n_left_to_process = n_left_to_process ;
				}
			}
			if ( ! working() ) return internal_state() ;
			// Assert scanning remains incomplete
		}
	}

	//-------------------------
	void
	Stream_Consumer::
	replenish( ACT::wakeup_listener * w )
	{
		//-------------------------
		// Ensure adequate input and acquire it if needed.
		//-------------------------
		// First check to see if we need to replenish our segment
		if ( n_left_to_process == 0 ) {
			//---------------
			// Buffer-Replenish Block
			//---------------
			/* Maybe we could roll this up into a loop, but it doesn't seem worth it.
			 */

			// Our action after the first call to next_segment() behaves differently than the second.
			IO::contiguous_buffer<> current_segment( next_segment() ) ;
			n_left_to_process = current_segment.length ;
			// Either we're done or we'll have to read.
			if ( n_left_to_process > 0 ) {
				// Assert n_left_to_process > 0
				// Assert (scan loop invariant) number_of_lookahead_characters_required == 0
				next_character = current_segment.begin ;
				// OK, we're done.
			} else {
				// Assert n_left_to_process == 0 and recently replenished current_segment is empty.
				// This means that either we know we're hit EOF or we need to read.
				if ( known_eof() ) {
					// Assert n_left_to_process == 0 and we have EOF
					// Done.
				} else {
					// Assert There's a possibility that the source might still have characters for us.
					( * the_source )( w ) ;
					if ( ! the_source -> completed() ) {
						// If we haven't completed, the caller will take care of the control issues.
						return ;
						/*
						 * Case: read_action -> working()
						 *
						 *	We're waiting on more input from our source.
						 *	There are two source behaviors to examine here.
						 *	(1) Source asynchronously fills its buffer.
						 *		The first next_segment() call above will return non-empty, since it was filled behind the scenes.
						 *		At that point our input cycle is finished.
						 *	(2) Source provides only notification of input waiting.
						 *		The first next_segment() call will return empty, since the notice didn't cause a read.
						 *		We'll next check EOF, whose status might have changed.
						 *		If not EOF, we'll again perform the read immediately above.
						 *		It will succeed, either with more data or with EOF.
						 *
						 * Case: read_action -> bad()
						 *
						 *	When a source goes bad, its consumer generally also goes bad.
						 *	Whatever the consumer might do, we give up here.
						 */
					}
					// Assert read_action has completed
					// Assert we've read when we had no next segment and we're not at EOF.
					// Thus, if our next segment is empty, there's a defect in our source.
					current_segment = next_segment() ;
					n_left_to_process = current_segment.length ;
					if ( n_left_to_process == 0 ) {
						// Nothing came back from our read.
						if ( known_eof() ) {
							// n_left_to_process == 0 and we have EOF
							// Done.
						} else {
							// An exception handler in the scheduler will recover.
							throw std::exception( "Invariant violation in source: empty next-segment_1 + read + not-EOF => empty next-segment_2" ) ;
						}
					} else {
						// Assert n_left_to_process > 0
						next_character = current_segment.begin ;
						// Done.
					}
				}
			}
		} 
		// [state at end of if-block]
		// Assert n_left_to_process == 0 implies we must process EOF
		// Assert n_left_to_process > 0 implies next_character points to beginning of our current segment.

		// If we don't need to replenish our segment, we might still have lookahead to deal with.
		else if ( ! characters_available( number_of_lookahead_characters_required ) ) {
			// Assert n_left_to_process > 0

			// We have lookahead characters to acquire
			// We want a loop here to handle multiple reads conceivably required if number_of_lookahead_characters >= 2
			while ( ! known_eof() && ! characters_available( number_of_lookahead_characters_required ) ) {
				// We've got reading to do.
				( * the_source )( w ) ;
				if ( ! the_source -> completed() ) {
					// Assert read_action is either Working or Bad.
					// In either case, we let the caller handle control issues.
					return ;
				}
			}
			// Assert the source is at EOF or there are adequate lookahead characters in the next segment.
			number_of_lookahead_characters_required = 0 ;

		} else {
			// Assert n_left_to_process > 0
			// Assert Either we had no lookahead required or there was enough of it.
		}
		// Assert n_left_to_process > 0
		// Assert there are at least top/number_of_lookahead_characters_required are available.
		// Assert number_of_lookahead_characters_to_read == 0 (perhaps because we just read them)
	}

} // end namespace IO
