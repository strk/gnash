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

/**	\file URI.cpp
 *	
 */

#include "URI.hpp"

namespace cygnal {
	URI_Scanner::
	URI_Scanner( IO::Source * source )
		: Stream_Consumer( source )
	{}

	URI_Scanner::
	URI_Scanner( IO::Source * source, char * & ref_next_character, size_t & ref_n_left_to_process )
		: Stream_Consumer( source, ref_next_character, ref_n_left_to_process )
	{}

	//--------------------------------------------------
	// Character class predicates
	//--------------------------------------------------
	inline bool is_alpha( char ch ) 
	{
		return ( 'a' <= ch && ch <= 'z' ) 
			|| ( 'A' <= ch && ch <= 'Z' ) ; 
	}
	//-------------------------
	inline bool is_digit( char ch )
	{
		return '0' <= ch && ch <= '9' ;
	}
	//-------------------------
	inline bool is_hexdigit( char ch ) {
		return is_digit( ch )
			|| ( 'A' <= ch && ch <= 'F' ) ;
	}
	//-------------------------
	// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
	inline bool is_scheme_char( char ch ) {
		return is_alpha( ch )
			|| is_digit( ch )
			|| ch == '+' || ch == '-' || ch == '.' ;
	}
	//-------------------------
	// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
	inline bool is_unreserved( char ch ) {
		return is_alpha( ch )
			|| is_digit( ch )
			|| ch == '-' || ch == '.' || ch == '_' || ch == '~' ;
	}
	//-------------------------
	// gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
	inline bool is_gen_delim( char ch ) {
		return ch == ':' || ch == '/' || ch == '?' || ch == '#' 
			|| ch == '[' || ch == ']' || ch == '@' ;
	}
	//-------------------------
	// sub-delims =	  "!" / "$" / "&" / "'" / "(" / ")"
	//				/ "*" / "+" / "," / ";" / "="
	inline bool is_sub_delim( char ch ) {
		return ch == '!' || ch == '$' || ch == '&' || ch == '\'' 
			|| ch == '(' || ch == ')' || ch == '*' || ch == '+' 
			|| ch == ',' || ch == ';' || ch == '=' ;
	}
	//-------------------------
	// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
	inline bool is_pchar( char ch ) {
		return is_unreserved( ch )
			|| is_sub_delim( ch )
			|| ch == ':' || ch == '@' ;
		// Percent-encoding is handled separately
	}
	//-------------------------
	void
	URI_Scanner::
	init()
	{
		uri.r_URI.set_begin( next_character ) ;
		URI_begin = next_character ;
		current_state = scheme_0 ;
	}

	//--------------------------------------------------
	// URI_Scanner
	//--------------------------------------------------
	ACT::act_state
	URI_Scanner::
	scan( ACT::wakeup_listener * ) {
		// If we are at EOF, some states may accept, some may reject.
		if ( n_left_to_process == 0 ) {
			switch ( current_state ) {
				case hier_part_0:
					break ;
				case hier_part_slash:
					uri.r_path.set_begin( next_character - 1 ) ;
					uri.r_path.set_length( 1 ) ;
					break ;
				case authority_0:
				case auth_unknown:
					uri.r_authority.set_begin( component_begin ) ;
					uri.r_authority.set_length( next_character - component_begin ) ;
					break ;
				case path_absolute_0:
				case path_absolute:
				case path_abempty_0:
				case path_absolute_slash:
					uri.r_path.set_length( next_character - component_begin ) ;
					break ;
				case query_0:
				case query:
					uri.r_query.set_length( next_character - component_begin ) ;
					break ;
				case fragment_0:
				case fragment:
					uri.r_fragment.set_length( next_character - component_begin ) ;
					break ;
				default:
					return set_bad() ;
			}
			uri.r_URI.set_length( next_character - URI_begin ) ;
			if ( uri.r_URI.size() == 0 ) return set_bad() ;
			return set_completed() ;
		}

		char ch ;
		switch ( current_state ) {
			//-------------------------
			// scheme
			//-------------------------
			case scheme_0:
				if ( n_left_to_process > 0 ) {
					component_begin = next_character ;
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_alpha( ch ) ) {
						goto label_scheme ;
					} else if ( ch == '*' ) {
						uri.is_result_asterisk = true ;
						goto label_done ;
					} else if ( ch == '/' ) {
						uri.r_path.set_begin( component_begin ) ;
						goto label_path_absolute_0 ;
					} else if ( ch == '?' ) {
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						goto label_fragment_0 ;
					} else {
						goto label_done_unget_last_character ;
					}
				}
				current_state = scheme_0 ;
				return ACT::Working ;
			label_scheme:
			case scheme:
				while ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( ch == ':' ) {
						// Accept the scheme name
						uri.r_scheme.set_begin( component_begin ) ;
						uri.r_scheme.set_length( next_character - component_begin - 1 ) ;
						if ( uri.r_scheme.size() == 0 ) return set_bad() ;
						goto label_hier_part_0 ;
					} else if ( is_scheme_char( ch ) ) {
						// do another
					} else {
						return set_bad() ;
					}
				}
				current_state = scheme ;
				return ACT::Working ;
			//-------------------------
			// hier-part
			//-------------------------
			label_hier_part_0:
			case hier_part_0:
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( ch == '/' ) {
						goto label_hier_part_slash ;
					} else if ( is_pchar( ch ) ) {
						uri.r_path.set_begin( next_character - 1 ) ;
						component_begin = next_character - 1 ;
						goto label_path_absolute ;
					} else if ( ch == '?' ) {
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						goto label_fragment_0 ;
					} else {
						goto label_done_unget_last_character ;
					}
				}
				current_state = hier_part_0 ;
				return ACT::Working ;
			//----------
			label_hier_part_slash:
			case hier_part_slash:
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( ch == '/' ) {
						goto label_authority_0 ;
					} else if ( is_pchar( ch ) ) {
						uri.r_path.set_begin( next_character - 2 ) ;	// need both current character and previous "/"
						component_begin = next_character - 2 ;
						goto label_path_absolute ;
					} else if ( ch == '?' ) {
						uri.r_path.set_begin( next_character - 2 ) ;
						uri.r_path.set_length( 1 ) ;
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						uri.r_path.set_begin( next_character - 2 ) ;
						uri.r_path.set_length( 1 ) ;
						goto label_fragment_0 ;
					} else {
						// all we have is a naked "/"
						uri.r_path.set_begin( next_character - 2 ) ;
						uri.r_path.set_length( 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = hier_part_slash ;
				return ACT::Working ;
			//-------------------------
			// authority
			//-------------------------
			label_authority_0:
			case authority_0:
				component_begin = next_character ;
				uri.r_authority.set_begin( component_begin ) ;
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) ) {
						goto label_auth_unknown ;
					} else if ( ch == '/' ) {
						goto label_path_abempty_0 ;
					} else if ( ch == '?' ) {
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						goto label_fragment_0 ;
					} else {
						goto label_done_unget_last_character ;
					}
				}
				current_state = authority_0 ;
				return ACT::Working ;
			//----------
			label_auth_unknown:
			case auth_unknown:
				while ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) ) {
						// stay here
					} else if ( ch == '/' ) {
						uri.r_authority.set_length( next_character - component_begin - 1 ) ;
						goto label_path_abempty_0 ;
					} else if ( ch == '?' ) {
						uri.r_authority.set_length( next_character - component_begin - 1 ) ;
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						uri.r_authority.set_length( next_character - component_begin - 1 ) ;
						goto label_fragment_0 ;
					} else {
						uri.r_authority.set_length( next_character - component_begin - 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = auth_unknown ;
				return ACT::Working ;
			//-------------------------
			// path
			//-------------------------
			label_path_absolute_0:
			case path_absolute_0:
				uri.r_path.set_begin( next_character - 1 ) ;	// incorporate previous "/" character
				component_begin = next_character - 1 ;
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) ) {
						goto label_path_absolute ;
					} else if ( ch == '%' ) {
						goto label_percent_encoding_not_implemented ;
					} else if ( ch == '?' ) {
						uri.r_path.set_length( 1 ) ;
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						uri.r_path.set_length( 1 ) ;
						goto label_fragment_0 ;
					} else if ( ch == '/' ) {
						// Hey, this isn't an absolute path!  It's an authority.
						goto label_authority_0 ;
					} else {
						uri.r_path.set_length( 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = path_absolute_0 ;
				return ACT::Working ;
			//----------
			label_path_absolute:
			case path_absolute:
				while ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) ) {
						// stay here
					} else if ( ch == '%' ) {
						goto label_percent_encoding_not_implemented ;
					} else if ( ch == '?' ) {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_fragment_0 ;
					} else if ( ch == '/' ) {
						goto label_path_absolute_slash ;
					} else {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = path_absolute ;
				return ACT::Working ;
			//----------
			label_path_abempty_0:
			case path_abempty_0:
				component_begin = next_character - 1 ;
				uri.r_path.set_begin( next_character - 1 ) ;	// incorporate previous '/' character
			//----------
			label_path_absolute_slash:
			case path_absolute_slash:
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) ) {
						goto label_path_absolute ;
					} else if ( ch == '%' ) {
						goto label_percent_encoding_not_implemented ;
					} else if ( ch == '?' ) {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_query_0 ;
					} else if ( ch == '#' ) {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_fragment_0 ;
					} else {
						uri.r_path.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = path_absolute_0 ;
				return ACT::Working ;
			//-------------------------
			// query
			//-------------------------
			label_query_0:
			case query_0:
				uri.r_query.set_begin( next_character ) ;
				component_begin = next_character ;
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) || ch == '/' || ch == '?' ) {
						goto label_query ;
					} else if ( ch == '%'  ) {
						goto label_percent_encoding_not_implemented ;
					} else if ( ch == '#'  ) {
						goto label_fragment_0 ;
					} else {
						goto label_done_unget_last_character ;
					}
				}
				current_state = query_0 ;
				return ACT::Working ;
			//----------
			label_query:
			case query:
				while ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) || ch == '/' || ch == '?' ) {
						// stay here
					} else if ( ch == '%'  ) {
						goto label_percent_encoding_not_implemented ;
					} else if ( ch == '#'  ) {
						uri.r_query.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_fragment_0 ;
					} else {
						uri.r_query.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = query_0 ;
				return ACT::Working ;
			//-------------------------
			// fragment
			//-------------------------
			label_fragment_0:
			case fragment_0:
				uri.r_fragment.set_begin( next_character ) ;
				component_begin = next_character ;
				if ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) || ch == '/' || ch == '?' || ch == '#' ) {
						goto label_fragment ;
					} else if ( ch == '%'  ) {
						goto label_percent_encoding_not_implemented ;
					} else {
						goto label_done_unget_last_character ;
					}
				}
				current_state = fragment_0 ;
				return ACT::Working ;
			//----------
			label_fragment:
			case fragment:
				while ( n_left_to_process > 0 ) {
					-- n_left_to_process ;
					ch = * next_character ++ ;
					if ( is_pchar( ch ) || ch == '/' || ch == '?' || ch == '#' ) {
						// stay here
					} else if ( ch == '%'  ) {
						goto label_percent_encoding_not_implemented ;
					} else {
						uri.r_fragment.set_length( ( next_character - component_begin ) - 1 ) ;
						goto label_done_unget_last_character ;
					}
				}
				current_state = fragment ;
				return ACT::Working ;
			//----------
			label_done_unget_last_character:
				uri.r_URI.set_length( next_character - URI_begin - 1 ) ;
				if ( uri.r_URI.size() == 0 ) return set_bad() ;
				return set_completed() ;

			//----------
			label_done:
			case done:
				uri.r_URI.set_length( next_character - URI_begin ) ;
				if ( uri.r_URI.size() == 0 ) return set_bad() ;
				return set_completed() ;
		}
		label_percent_encoding_not_implemented:
		throw std::exception( "% encoding not yet implemented" ) ;

	}
} // end namespace cygnal
