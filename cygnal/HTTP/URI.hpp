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

/**	\file URI.hpp
 *	\brief URI support: data class, parsing element
 *
 *	RFC3986 is the most recent definition of URI as of this writing.
 *	The top-level production in that grammar is as follows:
 *		 URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
 */

#pragma once
#ifndef __URI_hpp___
#define __URI_hpp___

#include "IO/Stream_Consumer.hpp"

namespace cygnal {
	//-------------------------
	/** \class Request_URI_Data
	 *	\brief Data class holding the result of parsing Request-URI ( RFC 2616, Section 5.1.2 ).
	 */
	class Request_URI_Data
	{
		// Direct access by scanner is just fine for us.
		friend class URI_Scanner ;

		bool is_result_asterisk ;		/// Request-URI = "*"

		// We need separate copies of these because we require the vtables for the abstract base class.
		//
		IO::result_buffer<> r_URI ;		/// [result] Request-URI (the whole thing)
		IO::result_buffer<> r_scheme ;	/// [result] scheme part of absoluteURI
		IO::result_buffer<> r_authority ;	/// [result] authority (part of absoluteURI or by itself)
		IO::result_buffer<> r_path ;		/// [result] path (part of absoluteURI or by itself as abs_path)
		IO::result_buffer<> r_query ;		/// [result] query part
		IO::result_buffer<> r_fragment ;	/// [result] fragment part

	public:
		/* Result type queries
		 */
		inline bool is_asterisk() { return is_result_asterisk ; }			/// Request-URI = "*"

		/* Result accessors
		 */
		inline IO::buffer result() { return r_URI ; }			/// [result] Request-URI (the whole thing)
		inline IO::buffer result_scheme() { return r_scheme ; }			/// [result] scheme part of absoluteURI
		inline IO::buffer result_authority() { return r_authority ; }	/// [result] authority (part of absoluteURI or by itself)
		inline IO::buffer result_path() { return r_path ; }				/// [result] path (part of absoluteURI or by itself as abs_path)
		inline IO::buffer result_query() { return r_query ; }			/// [result] query part
		inline IO::buffer result_fragment() { return r_fragment ; }		/// [result] fragment part

		Request_URI_Data()
			: is_result_asterisk( false )
		{}
	} ;

	//-------------------------
	/** \class URI_Scanner
	 *
	 *	\todo Fix the silent consumption of the next character after the URI.
	 *		It interferes with a caller's grammar.
	 */
	class URI_Scanner
		: public IO::Stream_Consumer
	{
		/// [result] The second word of a request is its Request-URI.
		Request_URI_Data uri ;
		
		char * URI_begin ;
		char * component_begin ;

		///
		ACT::ACT_State scan( ACT::wakeup_listener * ) ;

		///
		void init() ;

		//---------------
		// unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
		// sub-delims    = "!" / "$" / "&" / "'" / "(" / ")"
		//				 / "*" / "+" / "," / ";" / "="
		// pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
		// pct-encoded   = "%" HEXDIG HEXDIG		
		//---------------
		enum scan_state {
			//----------
			// ^
			scheme_0,
				// ALPHA -> scheme
				// '/' -> path_absolute_0
				// '*' -> done_asterisk
				// others -> error

			//----------
			// ^ ALPHA (scheme-char) *
			scheme,		
				// scheme-char -> scheme
				// ':' -> hier_part_0
				// others -> error

			//-------------------------
			// hier-part     = "//" authority path-abempty
			//               / path-absolute
			//               / path-rootless
			//               / path-empty
			// path-abempty  = *( "/" segment )
			// path-absolute = "/" [ segment-nz *( "/" segment ) ]
			// path-rootless = segment-nz *( "/" segment )
			// path-empty    = 0<pchar>
			// segment       = *pchar
			// segment-nz    = 1*pchar
			//-------------------------
			// scheme ":"
			hier_part_0,
				// '/' -> hier_part_slash

			//----------
			// scheme ":/"
			hier_part_slash,
				// '/' -> authority_0

			//-------------------------
			// authority     = [ userinfo "@" ] host [ ":" port ]
			// userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
			// host          = IP-literal / IPv4address / reg-name
			// IPv4address   = dec-octet "." dec-octet "." dec-octet "." dec-octet
			// reg-name      = *( unreserved / pct-encoded / sub-delims )
			// port          = *DIGIT
			//---------------
			// We're leaving out IP-literal, which is for IPv6 and IPvFuture
			// userinfo is terminated by '@', but can contain ':'.
			// If authority doesn't contain @, there's no userinfo, but you can't tell that until the end.
			// In particular, ':' is legal in userinfo, so it can't be immediately distinguished from a port number.
			// Possible terminators for authority:
			//		'?' (marks a query)
			//		'#' (marks a fragment)
			//		'/' (marks start of a path)
			//		or the end of the URI (gad!), which is to say, any not-excluded character.
			// We look for IPv4address after recognizing reg-name, since it's a subset.
			// Strictly speaking, this makes the grammar definition ill-formed.  Grumble.
			//-------------------------
			// scheme "://"
			authority_0,			
				// '@' -> auth_host_0 [out: empty userinfo]
				// ':' -> [out: empty userinfo, host]
				// '/' -> [out: empty authority]
				// '[' -> IP-literal (not supported as yet) (but this is where we'd insert it)
				// reg-name-char -> auth_unk [mark: something]
				// '?' -> query_0 [out: empty authority]
				// '#' -> fragment_0 [out: empty authority, empty query]
			//----------
			// scheme "://" [mark/something] (reg-name-char)+
			auth_unknown,
				// '@' -> auth_userinfo_0 [out: something => userinfo]
				// ':' -> auth_unknown_colon_0
				// '/' -> [out: something => host, empty port]
				// reg-name-char -> .
				// '?' -> query_0 [out: something => host, empty port, authority]
				// '#' -> fragment_0 [out: something => host, empty port, authority, empty query]
			//----------
			// scheme "://" userinfo "@"
			auth_host_0,
				// ':' -> [out: empty userinfo, empty host]
				// '/' -> [out: empty userinfo, empty host, empty port]
				// reg-name-char -> auth_host [mark: host] 
				// '?' -> query_0 [out: empty userinfo, empty host, empty port, authority]
				// '#' -> fragment_0 [out: empty userinfo, empty host, empty port, authority, empty query]
				// other -> done [unget] [out: empty host, empty port, empty query, empty frag]
			//----------
			// scheme "://" userinfo "@" (reg-name-char)+
			auth_host,
				// ':' -> auth_host_0 [out: host]
				// '/' -> auth_abempty_0 [out: host, empty port]
				// reg-name-char -> .
				// '?' -> query_0 [out: host, empty port, authority]
				// '#' -> fragment_0
				// other -> done [unget] [out: host, empty port, authority, empty query, empty frag]
			//----------
			// scheme "://" (userinfo "@")? host ":"
			auth_port_0,
				// '/' -> [out: empty port]
				// DIGIT -> auth_host [mark: port]
				// other -> done [unget] [out: empty port, authority, empty query, empty frag]
			//----------
			// scheme "://" userinfo "@" host ":" (DIGIT)+
			auth_port,

			//-------------------------
			// Absolute Path
			//-------------------------
			// "/"
			path_absolute_0,
				// pchar -> path_absolute_nz
				// '/' -> error
							// This error is the place to insert parsing for a relative URI reference, which can begin with "//"
				// '?' -> query_0 [out: path => "/"]
				// '#' -> fragment_0 [out: path => "/"]
				// others -> error

			// ... "/" (segment "/")* pchar
			path_absolute,
				// pchar -> .
				// '/' -> error
				// '?' -> query_0
				// '#' -> fragment_0

			// scheme "://" authority "/"
			path_abempty_0,
				// 

			// ... "/" pchar (segment "/")+
			path_absolute_slash,
				// pchar -> path_absolute_nz
				// '/' -> error
							// This error is fundamental, since it doesn't occur at the beginning
				// '?' -> query_0
				// '#' -> fragment_0

			//-------------------------
			// Query
			//-------------------------
			// scheme ":" hier-part "?"
			query_0,

			query,

			//-------------------------
			// Fragment
			//-------------------------
			// scheme ":" hier-part "?" query "#"
			fragment_0,

			fragment,

			//-------------------------
			// Final states
			//-------------------------
			error,
			done

		} ;

		scan_state current_state ;
	public:
		///
		URI_Scanner( IO::Source * ) ;

		///
		URI_Scanner( IO::Source *, char * & ref_next_character, size_t & ref_n_left_to_process ) ;

		/// [result] The second word of a request is its Request-URI.
		inline IO::buffer request_URI() { return uri.result() ; }

		inline Request_URI_Data result_uri() { return uri ; }

	} ;

	//-------------------------

} // end namespace cygnal
#endif	// end of inclusion protection
