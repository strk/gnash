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

/**	\file HTTP_Parse.cpp
 *	
 */

#include "HTTP_Parse.hpp"
using namespace ACT ;

namespace cygnal { namespace HTTP {

	//--------------------------------------------------
	// character class table
	//--------------------------------------------------
	extern character_class ch_class[ 256 ] = { 
		// 0 - 31
		ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	
		ctl,	HT,		LF,		ctl,	ctl,	CR,		ctl,	ctl,	
		ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	
		ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	ctl,	

		// 32 - 63
		//		!		"		#		$		%		&		'
		SP,		txt,	DQ,		txt,	txt,	txt,	txt,	txt,	
		// (	)		*		+		,		-		.		/
		sep,	sep,	txt,	txt,	sep,	txt,	txt,	sep,	
		// 0	1		2		3		4		5		6		7	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,
		// 8	9		:		;		<		=		>		?
		txt,	txt,	cln,	sep,	sep,	sep,	sep,	sep,	

		// 64 - 95
		sep,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	sep,	bsl,	sep,	txt,	txt,	
		// 96 - 127
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	sep,	txt,	sep,	txt,	ctl,	
		// 128 - 255
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
		txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	txt,	
	} ;

	/// Convenience predicate for characters permissible in production "token".
	inline bool is_token( char ch ) { return txt == ch_class[ ch ] ; }

	///
	header_parse_state transitions[ n_nonterminal_states ][ n_character_classes ] = {
		{
			// [initial] ^
			//		txt -> token
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	p_null,	error,	error,	error,	error,	error,	error,	error,	token
		},{
			// [token] ^ token
			//		cln -> toknam
			//		txt -> token
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	error,	error,	error,	error,	error,	line0,	error,	error,	token
		},{
			// [line0] ^ token :
			//		CR -> p_head
			//		DQ -> quot
			//		sep, SP, cln, bsl, txt -> line
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	p_head,	error,	error,	line,	line,	line,	quot,	line,	line
			/*
			 * Note that only a single transition goes into this state.
			 * This allows us to mark the token-name efficiently.
			 */
		},{
			// [line] ^ token : field-value*
			//		CR -> p_head
			//		DQ -> quot
			//		sep, SP, cln, bsl, txt -> line
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	p_head,	error,	error,	line,	line,	line,	quot,	line,	line
		},{
			// [p_head] ^ token : field-value* CR
			//		LF -> header
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	error,	header,	error,	error,	error,	error,	error,	error,	error
		},{
			// [quot] ^ token : field-value* " qtext*
			//		sep, SP, cln, txt -> quot
			//		bsl -> quot2
			//		DQ -> line
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	error,	error,	error,	quot,	quot,	quot,	line,	quot2,	line
		},{
			// [quot2] ^ token : field-value* " qtext* \	
			//		<all> -> quot
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				quot,	quot,	quot,	quot,	quot,	quot,	quot,	quot,	quot,	quot
			/*
			 * Note that there's a small deviation from the standard grammar here.
			 * The HTTP/1.1 spec says:
			 *		quoted-pair = "\" CHAR
			 *		CHAR = <any US-ASCII character (octets 0 - 127)>
			 * This parser uses OCTET instead of CHAR for what can follow a backslash.
			 * Should anybody care to fix this, it requires another character class, say "txt2",
			 *		that's just like "txt" except for this rule.
			 * [EH] I consider this a defect in HTTP/1.1, albeit minor.
			 */
		},{
			// [lws] ^ token : field-value* CR LF (SP HT)+
			//		CR -> p_head
			//		HT, SP -> lws
			//		sep, cln, bsl, txt -> line
			//		DQ -> quot
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	p_head,	error,	lws,	line,	lws,	line,	quot,	line,	line
		},{
			// [p_null] ^ CR
			//		LF -> null
			/*	ctl,	CR,		LF,		HT,		sep,	SP,		cln,	DQ,		bsl,	txt		*/
				error,	error,	null,	error,	error,	error,	error,	error,	error,	error
		}
	} ;

	//--------------------------------------------------
	// read_single_header
	//--------------------------------------------------

	//-------------------------
	read_single_header::
	read_single_header( IO::Source * source )
		: IO::Stream_Consumer( source ),
		current_state( initial ),
		message_header_size( 0 )
	{}

	//-------------------------
	read_single_header::
	read_single_header( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process )
		: IO::Stream_Consumer( source, ref_next_character, ref_n_left_to_process ),
		current_state( initial ),
		message_header_size( 0 )
	{}

	//-------------------------
	ACT::act_state
	read_single_header::
	scan( ACT::wakeup_listener * w )
	{
		//-------------------------
		// Handle EOF pseudo-character as next character
		//-------------------------
		if ( n_left_to_process == 0 ) {
			// Assert most recent segment was empty and the source is at EOF.
			// Assert we have no more input.
			switch ( current_state ) {
				// header is the only accepting terminal state with lookahead
				case header:
					// Assert input stream is "... CR LF EOF ^"
					// This syntax is OK for a message-header line.
					// BUT: in an HTTP header, such an input stream may be invalid.  Not our problem here.
					consume() ; 
					set_completed() ;
					break ;
				default:
					// If we see EOF in any other case (non-terminal or non-accepting terminal with lookahead), we go bad.
					set_bad() ;
			} ;
			// Assert the_state != Working
			// Always return after EOF.
			return internal_state() ;
		}

		//-------------------------
		// Transition action into initial state
		//-------------------------
		if ( current_state == initial ) {
			r_field_name.set_begin( next_character ) ;
			r_field_name.set_length( 0 ) ;
		}

		//-------------------------
		// Character Processing Loop
		//-------------------------
		while ( n_left_to_process -- > 0 )
		{
			// Separating next state from current state enables rollback in case we must read more for lookahead
			header_parse_state next_state = transitions[ current_state ][ ch_class[ * next_character ] ] ;
			++ next_character ;
			++ message_header_size ;
			switch ( next_state ) {
				//---------------
				// Nonterminal States
				//---------------
				case token:
					// We have one more
					r_field_name.incr_length() ;
					break ;
				case line0:
					// Assert * next_character == ':', since it's the only transition in.
					// We've just seen the marker between field-name and field-content.
					break ;
				//---------------
				// Terminal States
				//---------------
				case header :
					// Assert input stream is "CR LF ^"
					// Must check for leading white space with lookahead.
					// characters_available checks for characters in the next segment not yet already granted to us.
					if ( n_left_to_process == 0 ) {
						// Assert we have no more characters left to look ahead into in the current segment.
						if ( known_eof() ) {
							// Assert input stream is "CR LF EOF ^"
							consume() ;
							return set_completed() ;
						}
						// Check for characters available in any next segment
						if ( characters_available( 1 ) ) {
							// Assert there's lookahead for us.
							
							// But we cannot handle it yet.
							throw std::exception( "Implementation Defect: Cannot handle lookahead not in current segment." ) ;
						} else {
							// Assert next segment is empty.

							// Set state to require lookahead
							require_lookahead( 1 ) ;

							// Unwind the last character advance.
							++ n_left_to_process ;
							-- next_character ;
							-- message_header_size ;
							// Go to top of main loop and get input
							goto end_of_processing_loop ;		// I'm GOTO, dammit!
						}
					}
					// Look ahead at next character
					if ( * next_character == ' ' || * next_character == '\t' ) {
						// Assert we have leading whitespace
						next_state = lws ;
						break ;
					}
					// Lookahead is not leading whitespace
					consume() ; 
					return set_completed() ;
				case null :
					// Assert we have an empty line.
					// We encode this result with a zero field-name length.
					// Certainly not a semantically-direct way to return results, but it makes for no extra work here.
					consume() ;
					return set_completed() ;
				case error :
					// Assert we have a syntactically invalid line
					return set_bad() ;
				default :
					// Assert we have an ordinary non-terminal state
					break ;
			}
			// Assert next_state is non-terminal.  If not, we would have returned.
			// Roll the state forward. (Rollback Support)
			current_state = next_state ;

			// Check for maximum size exceeded.
			if ( message_header_size >= maximum_message_header_size ) {
				// Assert we're in a non-terminal state and have exceeded our message size.
				return set_bad() ;
			}
		}
		// Assert n_left_to_process == 0 

		end_of_processing_loop:
		return ACT::Working ;
	}

	//-------------------------
	void
	read_single_header::
	restart()
	{
		if ( bad() ) {
			// We don't restart bad actions.
			return ;
		}

		//---------------
		/* These members alter on reset:
		 *	- current_state.  We must restart the DFA.
		 *	- message_header_size.  This size is for an individual header, not the whole header block.
		 *	- r_field_name.  We have a new first character of the field name.
		 */
		current_state = initial ;
		message_header_size = 0 ;
		r_field_name.set_begin( next_character ) ;
		r_field_name.set_length( 0 ) ;
		set_working() ;
	}

	//--------------------------------------------------
	// read_message_headers_action
	//--------------------------------------------------

	//-------------------------
	read_message_headers_action::
	read_message_headers_action( IO::Source * source )
		: IO::Stream_Consumer( source ),
		read_single_header_action( source, next_character, n_left_to_process )
	{} ;

	//-------------------------
	ACT::act_state
	read_message_headers_action::
	scan( ACT::wakeup_listener * waken )
	{
		// Parsing loop
		// Assert (loop precondition) input prefix is "(message-header CR LF)*"
		while( true ) {
			read_single_header_action( waken ) ;
			if ( ! read_single_header_action.completed() ) {
				// Assert the action is either still working or gone bad.
				// Both require immediate exit.
				if ( read_single_header_action.bad() ) {
					// Our subroutine went bad.  So do we.
					return set_bad() ;
				}
				return ACT::Working ;
			}
			// Assert (postcondition--read_single_header action completed) input processed was "message-header? CR LF"
			IO::buffer field_name( read_single_header_action.result_field_name() ) ;
			// We're done if the header we parsed is an empty line (i.e. no message-header)
			if ( field_name.size() == 0 ) {
				// Assert input processed was "CR LF"
				// Assert input prefix has been "(message-header CR LF)* CR LF"
				// Thus we're done.
				return set_completed() ;
			}
			// Assert input prefix is "(message-header CR LF)+"
			read_single_header_action.restart() ;
		}
	}

	//--------------------------------------------------
	// Relevant_Header_Fields
	//--------------------------------------------------
	static char * known_field_names[] = { "Referer" } ;
	const unsigned int n_known_field_names = sizeof( known_field_names ) / sizeof( char * ) ;
	static IO::result_buffer<> known_field_buffers[ n_known_field_names ] ;

	Relevant_Header_Fields::
	Relevant_Header_Fields()
	{
	}

	//--------------------------------------------------
	// Header_Field_Detector
	//--------------------------------------------------

	/// static field table
	Relevant_Header_Fields Header_Field_Detector::field_table ;

	//--------------------------------------------------
	// Request_Method_Scanner
	//--------------------------------------------------
	Request_Method_Scanner::
	Request_Method_Scanner( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process )
		: IO::Stream_Consumer( source, ref_next_character, ref_n_left_to_process )
	{}

	//-------------------------
	void
	Request_Method_Scanner::
	init()
	{
		r_method.set_begin( next_character ) ;
		r_method.set_length( 0 ) ;
	}

	//-------------------------
	ACT::act_state
	Request_Method_Scanner::
	scan( ACT::wakeup_listener * w )
	{
		// EOF is always an error.
		// Since we require a final space in the grammar, EOF is not a terminator (as it often is).
		if ( n_left_to_process == 0 ) {
			return set_bad() ;
		}

		char ch ;
		while ( n_left_to_process -- > 0 ) {
			ch = * next_character ++ ;
			if ( ch == ' ' ) {
				if ( r_method.size() == 0 ) {
					// Zero-length word disallowed
					return set_bad() ;
				}
				break ;
			} else if ( is_token( ch ) ) {
				// token character is OK
				r_method.incr_length() ;
			} else {
				return set_bad() ;
			}
		}
		return set_completed() ;
	}

	//--------------------------------------------------
	// Request_Version_Scanner
	//--------------------------------------------------
	inline bool is_digit( char ch )
	{
		return '0' <= ch && ch <= '9' ;
	}

	//-------------------------
	Request_Version_Scanner::
	Request_Version_Scanner( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process )
		: IO::Stream_Consumer( source, ref_next_character, ref_n_left_to_process ),
		scan_state( initial )
	{}

	//-------------------------
	void
	Request_Version_Scanner::
	init()
	{
		the_result.r_version.set_begin( next_character ) ;
	}

	//-------------------------
	/**	\par Implementation Notes
	 *	- The protocol designator "HTTP" is sensitive to case.
	 */
	ACT::act_state
	Request_Version_Scanner::
	scan( ACT::wakeup_listener * w )
	{
		char ch ;
		switch ( scan_state ) {
			case initial:
				if ( n_left_to_process == 0 ) { scan_state = initial ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ch != 'H' ) return syntax_error() ;
			case seen_H:
				if ( n_left_to_process == 0 ) { scan_state = seen_H ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ch != 'T' ) return syntax_error() ;
			case seen_HT:
				if ( n_left_to_process == 0 ) { scan_state = seen_HT ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ch != 'T' ) return syntax_error() ;
			case seen_HTT:
				if ( n_left_to_process == 0 ) { scan_state = seen_HTT ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ch != 'P' ) return syntax_error() ;
			case seen_HTTP:
				if ( n_left_to_process == 0 ) { scan_state = seen_HTTP ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ch != '/' ) return syntax_error() ;
			case first_digits_0:
				if ( n_left_to_process == 0 ) { scan_state = first_digits_0 ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ! is_digit( ch ) ) return syntax_error() ;
				the_result.major_number = ( ch - '0' ) ;
				the_result.r_version.set_length( 6 ) ;
			case first_digits:
				while ( n_left_to_process > 0 ) {
					ch = * next_character ++ ; -- n_left_to_process ;
					if ( is_digit( ch ) ) {
						the_result.major_number *= 10 ;
						the_result.major_number += ( ch - '0' ) ;
						the_result.r_version.incr_length() ;
						// keep going
					} else if ( ch == '.' ) {
						the_result.r_version.incr_length() ;
						goto label_second_digits_0 ;
					} else {
						return syntax_error() ;
					}
				}
				scan_state = first_digits_0 ; 
				return Working ;
			label_second_digits_0:
			case second_digits_0:
				if ( n_left_to_process == 0 ) { scan_state = second_digits_0 ; return Working ; }
				ch = * next_character ++ ; -- n_left_to_process ;
				if ( ! is_digit( ch ) ) return syntax_error() ;
				the_result.minor_number = ( ch - '0' ) ;
				the_result.r_version.incr_length() ;
			case second_digits:
				while ( n_left_to_process > 0 ) {
					ch = * next_character ++ ; -- n_left_to_process ;
					if ( is_digit( ch ) ) {
						the_result.minor_number *= 10 ;
						the_result.minor_number += ( ch - '0' ) ;
						the_result.r_version.incr_length() ;
						// keep going
					} else {
						goto label_end_of_scan ;
					}
				}
				scan_state = first_digits_0 ; 
				return Working ;
		}
	label_end_of_scan:
		++ n_left_to_process ;
		-- next_character ;
		return set_completed() ;
	}

	//--------------------------------------------------
	// Request_Scanner
	//--------------------------------------------------
	Request_Scanner::
	Request_Scanner( IO::Source * source )
		: IO::Stream_Consumer( source ),
		current_state( in_method ),
		scan_method( source, next_character, n_left_to_process ),
		scan_URI( source, next_character, n_left_to_process ),
		scan_version( source, next_character, n_left_to_process )
	{}

	//-------------------------
	ACT::act_state
	Request_Scanner::
	scan( ACT::wakeup_listener * w )
	{
		// EOF always generates an error
		if ( n_left_to_process == 0 ) {
			goto label_error ;
		}

		char ch ;
		// NOTE: This switch statement is an annotated statement sequence that allows re-entry after ACT interruption.
		switch ( current_state ) {
			case in_method:
				scan_method( w ) ;
				if ( scan_method.working() ) return ACT::Working ;
				if ( scan_method.bad() ) goto label_error ;
				current_state = in_request_URI ;
				// fall through
			case in_request_URI:
				scan_URI( w ) ;
				if ( scan_URI.working() ) return ACT::Working ;
				if ( scan_URI.bad() ) goto label_error ;
				current_state = in_HTTP_version ;
				// fall through
			case in_HTTP_version:
				scan_version( w ) ;
				if ( scan_version.working() ) return ACT::Working ;
				if ( scan_version.bad() ) goto label_error ;
				current_state = p_final_CR ;
				// fall through
			case p_final_CR:
				if ( n_left_to_process == 0 ) return ACT::Working ;
				-- n_left_to_process ;
				ch = * next_character ++ ;
				if ( ch != '\r' ) goto label_error ;
				current_state = p_final_LF ;
				// fall through
			case p_final_LF:
				if ( n_left_to_process == 0 ) return ACT::Working ;
				-- n_left_to_process ;
				ch = * next_character ++ ;
				if ( ch != '\n' ) goto label_error ;
				// Assert ch == '\n'
				// Assert input is "... CR LF ^"
				// We have finished successfully
				goto label_end_of_scan ;
			label_error:
				current_state = error ;
			case error:
				return syntax_error() ;
		}
	label_end_of_scan:
		return set_completed() ;
	}

} } // end namespace cygnal::HTTP
