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

/**	\file HTTP_Behavior.hpp
 *	\brief HTTP protocol behavior
 *
 */

#pragma once
#ifndef __HTTP_Behavior_hpp___
#define __HTTP_Behavior_hpp___

#include "IO/IO_Generator.hpp"
#include "HTTP_Parse.hpp"

namespace cygnal { namespace HTTP {

	//-------------------------
	/**	\class HTTP_Behavior
	 */
	class HTTP_Behavior
		: public IO::Behavior
	{
		///
		IO::Device * the_device ;

		/// Scanner for the entire HTTP Request.
		///
		/// OK, so the current implementation of this only scans the Request-Line.
		/// The full version of this will be available later.
		Request_Scanner scan_request ;

		enum {
			initial,
			scanning_request,
			responding,
			sending_final_message

		} protocol_state ;

	public:
		///
		HTTP_Behavior( IO::Device * ) ;

		/// 
		ACT::ACT_State run( ACT::wakeup_listener * ) ;

		///
		static shared_ptr< ACT::autonomous_act > new_HTTP_Behavior( IO::Device * ) ;
	} ;


} } // end namespace cygnal::HTTP

#endif	// end of inclusion protection
