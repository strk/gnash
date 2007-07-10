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

/**	\file HTTP_Behavior.cpp
 *	
 */

#include "HTTP_Behavior.hpp"
using namespace ACT ;

namespace cygnal { namespace HTTP {

	//-------------------------
	HTTP_Behavior::
	HTTP_Behavior( IO::Device * device )
		: the_device( device ), 
		protocol_state( initial ),
		scan_request( device )
	{}

	//-------------------------
	ACT::ACT_State
	HTTP_Behavior::
	run( wakeup_listener * w )
	{
		ACT::ACT_State x( ACT::ACT_State::Ready ) ;

		switch ( protocol_state ) {
			case initial:
				// nothing yet to do before first ACT
				protocol_state = scanning_request ;
			case scanning_request:
				x = scan_request( w ) ;
				if ( x.bad() ) return set_bad() ;
				if ( x.working() ) return set_state( x ) ;
				protocol_state = responding ;
			case responding:
				// We'll support HTTP/1.0 later.
				if ( scan_request.HTTP_version_major_number() != 1 
					|| scan_request.HTTP_version_minor_number() != 1 )
				{
					// generate response 505 HTTP Version Not Supported
					// We'll close the connection too.
					char response_status_line[] = "HTTP/1.1 505 HTTP Version Not Supported" "\r\n" ;
					char connection_close_header[] = "Connection: close" "\r\n" ;

					IO::E_AV_Buffer< 2 > b ;
					b.append( response_status_line, sizeof( response_status_line ) - 1 ) ;
					b.append( connection_close_header, sizeof( connection_close_header ) - 1 ) ;
					the_device -> to_write( b ) ;
					goto label_sending_final_message ;
				}
				// Assert HTTP version is 1.1
				break ;

			label_sending_final_message:
				protocol_state = sending_final_message ;
			case sending_final_message:
				x = the_device -> Sink::operator()( w ) ;
				if ( x.bad() ) return set_bad() ;
				if ( x.working() ) return set_state( x ) ;
				return set_completed() ;
		} ;
		return set_would_block() ;
	}

} } // end namespace cygnal::HTTP
