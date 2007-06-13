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

/**	\file HTTP_Parse.hpp
 *	\brief HTTP syntax support
 *
 *	For the syntax definition of HTTP/1.1 messages, see
 *		http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
 */

#pragma once
#ifndef __HTTP_Parse_hpp___
#define __HTTP_Parse_hpp___

#include "URI.hpp"

#include <boost/static_assert.hpp>
#include <boost/scoped_array.hpp>
#include <map>

namespace cygnal { namespace HTTP {

	//-------------------------
	/**
	 *	Note that the definition sequence is important to be able to play category tricks in the representation.
	 *	Here are the categories:
	 *	- Controls.  Octets 0 - 31, 127.
	 *	- Separators.  Space, horizontal tab, punctuation "()<>@,;\/[]?={}", double quote, and colon.
	 *	- Text.  Everything but controls, but contextually may include leading white space.\
	 *	- Token text.  Text that isn't either a control or separator.
	 *
	 *	Note that HT is both an control and a separator, and that all other separators are text.
	 *	Here are the representations of these categories:
	 *	- Controls. other_ctl .. HT.
	 *	- Separators. HT .. SP.
	 *	- Text. sep .. text.
	 *	- Token.  text only.
	 *
	 *	This definition is in this header to enable unit testing.
	 *	Perhaps later it might be good to move this to an implementation-specific header,
	 *		included only in the implementation file and in unit tests.
	 */
	enum character_class {
		ctl = 0,		// all CTL characters except those identified: CR, LF, HT
		CR,				// carriage return '\r'
		LF,				// line feed '\n'
		HT,				// horizontal tab '\t'
		sep,			// all other separators
		SP,				// space ' '
		cln,			// ':'
		DQ,				// '\"'
		bsl,			// '\\'
		txt,			// everything else
	} ;
	const unsigned int n_character_classes = 10 ;
	BOOST_STATIC_ASSERT( n_character_classes - 1 == (unsigned int) txt ) ;

	//---------------
	/// Enumeration of internal parser states
	enum header_parse_state {
		// Nonterminal states
		initial,	// ^
		token,		// ^ token
		line0,		// ^ token : field-value*
		line,		// ^ token : field-value*
		p_head,		// ^ token : field-value* CR
		quot,		// ^ token : field-value* " qtext*
		quot2,		// ^ token : field-value* " qtext* \		Note: backslash quotes any single character
		lws,		// ^ token : field-value* CR LF ( SP | HT )+
		p_null,		// ^ CR
		// Accepting terminal states with lookahead
		header,		// ^ token : field-value* CR LF				
					// Note: in this state we require lookahead to check for LWS.  
					// If it's there, we transition manually to state [lws]
		// Accepting terminal states without lookahead
		null,		// ^ CR LF
		// Non-accepting terminal states without lookahead
		error
	} ;
	static const unsigned int n_nonterminal_states = 9 ;
	inline bool is_terminal( header_parse_state x ) { return header <= x ; }

	//-------------------------
	/**	\class read_single_header
	 *	\brief Obtain a single header line or the final "null header" from a designated device.
	 *
	 *	A well-formed "message-header" (see RFC2616/4.2 Message Headers)
	 *	- is a "field-name" followed by a colon followed by a sequence of "field-content"
	 *	- always ends with CR-LF (and neither naked CR nor naked LF), BUT ...
	 *	- may continue onto a subsequent lines if they begin with leading whitespace
	 *	- may contain CR or LF (or other controls) inside "quoted-string"
	 *
	 *	The sequence of "message-header" ends with an empty line: ^ CR LF.
	 *	An empty line is also permitted to be recognized before the block of "message-header".
	 *	Hence we treat the empty line as ordinary, and let the next level upwards deal with it.
	 *
	 *	Important consequences of this grammar include the following:
	 *	- Recognizer must be a stateful parser, rather than just a search for marker sequences.
	 *	- Parser must be at least LA(1) (one character of lookahead) to deal with continuation lines.
	 *	- CR and LF handling must be careful.
	 *
	 *	\par Security Note
	 *	Strictly speaking, the grammar in the RFC can't be made concrete, 
	 *		because it specifies no upper bound on the size of "message-header".
	 *	This is a buffer overflow waiting to happen.
	 *	Hence we alter the grammar and put on an upper bound to the size of a message header.
	 *	We pick the upper bound to be 1000 octets.
	 *	Sure, it _is_ arbitrary.	
	 *
	 *	\invariant
	 *	- the_state == Working implies
	 *		- input read so far is a valid prefix of message-header
	 *	- input_required implies read_action contains a read action that's ready to go
	 */
	class read_single_header
		: public IO::Stream_Consumer
	{
		/// [ACT state] Current parser state.
		header_parse_state current_state ;

		/// [ACT state] Total characters in the message-header, including any terminating CR LF
		/// 
		/// This the internal variable whose size cannot exceed our arbitrary message-header length limit.
		size_t message_header_size ;

		/// Length limit imposed upon message-header.  Length includes final CR-LF.
		static const size_t maximum_message_header_size = 1000 ;

		/// [ACT result] position of field-name in a valid header
		char * first_character ;

		/// [ACT result]
		IO::result_buffer<> r_field_name ;

		///
		ACT::act_state scan( ACT::wakeup_listener * ) ;

	public:
		/// Ordinary constructor
		read_single_header( IO::Source * source ) ;
		read_single_header( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process ) ;

		/// Result accessor
		size_t result_message_header_size() {
			// the final size does not include CR LF
			return message_header_size - 2 ; 
		}

		/// Result accessor
		inline IO::buffer result_field_name() {
			// HACK: This statement relies upon the buffer being contiguous.  This is true as of this writing, but won't be later.
			return IO::buffer( r_field_name ) ;
		}

		/**	Restart the action to operate again.
		 *	Resulting internal state must show identical behavior as if the following hold:
		 *	- all input from the source not yet read were given back to the source.
		 *	- this instance were a newly constructed object.
		 *	We cannot restart if we're in a Bad state.
		 */
		void restart() ;
	} ;

	//-------------------------
	/**	\class read_message_headers_action
	 *	\brief Obtain the headers of next HTTP message from a designated device.
	 *
	 *	\invariant
	 *	- the_state == Working implies
	 *		- initial prefix of message as-read is a well-formed prefix.
	 *	- the_state == Completed implies 
	 *		- message headers were well formed
	 *		- we've read at least as many, possibly more, characters as are in the headers.
	 *		- 
	 *	- message headers malformed implies
	 *		- the_state == Bad
	 */
	class read_message_headers_action
		: public IO::Stream_Consumer
	{
		/// Subroutine action.
		read_single_header read_single_header_action ;

		/// Body of action
		ACT::act_state scan( ACT::wakeup_listener * ) ;

	public:
		/// Ordinary constructor takes a nonblocking device as its source
		read_message_headers_action( IO::Source * source ) ;
	} ;

	//-------------------------
	// forward declaration
	class Header_Field_Detector ;

	//-------------------------
	/** \class Relevant_Header_Fields
	 *	\brief Holds a common copy of hash table
	 */
	class Relevant_Header_Fields
	{
		typedef IO::buffer key_type ;
		typedef std::map< key_type, int > map_type ;
		typedef map_type::value_type pair_type ;
		/// 
		map_type field_names ;
	public:
		Relevant_Header_Fields() ;
	} ;

	//-------------------------
	/**	\class Header_Field_Detector
	 *	\brief 
	 */
	class Header_Field_Detector
	{
		/// Precomputed enumeration of header fields that can be recognized
		static Relevant_Header_Fields field_table ;
	} ;

	//-------------------------
	/**	\class Request_Method_Scanner
	 *	\brief Recognizes the method of an HTTP request.
	 *
	 *	See RFC 2616, Section 5.1.1 for a definition.
	 *	The grammar is simply a 'token'.
	 *	Certain tokens are designated methods; all others are 'extension-method'.
	 *
	 *	The grammar this scanner accepts is "token SP".
	 *	The result of the scan is the text of the token, without the space.
	 */
	class Request_Method_Scanner
		: public IO::Stream_Consumer
	{
		/// [result]
		IO::result_buffer<> r_method ;

		///
		ACT::act_state scan( ACT::wakeup_listener * ) ;

		///
		void init() ;
		
	public:
		/// Ordinary constructor.
		Request_Method_Scanner( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process ) ;

		/// [result]
		inline IO::buffer method() { return r_method ; }
	} ;

	//-------------------------
	/**	\class HTTP_Version
	 */
	struct HTTP_Version
	{
		/// [result]
		IO::result_buffer<> r_version ;

		///
		unsigned int major_number ;

		///
		unsigned int minor_number ;
	} ;

	//-------------------------
	/**	\class Request_Version_Scanner
	 */
	class Request_Version_Scanner
		: public IO::Stream_Consumer
	{
		/// [result]
		HTTP_Version the_result ;

		///
		ACT::act_state scan( ACT::wakeup_listener * ) ;

		///
		void init() ;

		enum {
			initial,
			seen_H,
			seen_HT,
			seen_HTT,
			seen_HTTP,
			first_digits_0,
			first_digits,
			second_digits_0,
			second_digits
		} scan_state ;

	public:
		/// Ordinary constructor.
		Request_Version_Scanner( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process ) ;

		/// [result]
		inline IO::buffer version() { return the_result.r_version ; }

		inline unsigned int result_major_number() const { return the_result.major_number ; }

		inline unsigned int result_minor_number() const { return the_result.minor_number ; }

	} ;

	//-------------------------
	/**	\class Request_Scanner
	 *	\brief Obtain the headers of next HTTP message from a designated device.
	 */
	class Request_Scanner
		: public IO::Stream_Consumer
	{
		enum request_parse_state {
			in_method,
			in_request_URI,
			in_HTTP_version,
			p_final_CR,
			p_final_LF,
			error
		} ;

		/// [ACT state]
		request_parse_state current_state ;

		/// [ACT subroutine]
		URI_Scanner scan_URI ;

		/// [ACT subroutine]
		Request_Method_Scanner scan_method ;

		/// [ACT subroutine]
		Request_Version_Scanner scan_version ;

		///
		ACT::act_state scan( ACT::wakeup_listener * ) ;

	public:
		/// Ordinary constructor takes a nonblocking device as its source
		Request_Scanner( IO::Source * source ) ;

		/// [result] The first word of a request is its Method.
		inline IO::buffer method() { return scan_method.method() ; }

		/// [result] The second word of a request is its Request-URI.
		inline IO::buffer request_URI() { return scan_URI.request_URI() ; }

		/// [result] The third word of a request is its HTTP-Version.
		inline IO::buffer HTTP_version() { return scan_version.version() ; }

		/// [result] 
		inline unsigned int HTTP_version_major_number() { return scan_version.result_major_number() ; }
		inline unsigned int HTTP_version_minor_number() { return scan_version.result_minor_number() ; }

	} ;

} } // end namespace cygnal::HTTP

#endif	// end of inclusion protection
