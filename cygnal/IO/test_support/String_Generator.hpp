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

/// \file String_Generator.hpp

#pragma once
#ifndef __String_Generator_hpp__
#define __String_Generator_hpp__

#include "../IO_Generator.hpp"
#include "Null_Device.hpp"

#include <string>
#include <queue>
#include <list>
#include <boost/shared_ptr.hpp>

namespace IO {

	//-------------------------
	/**	\class String_Generator
	 *	\brief A generator of string devices.
	 */
	class String_Generator
		: public Device_Generator
	{
		///
		std::list< std::string > the_list ;

		/// Received shutdown?
		bool complete ;

	public:
		///
		String_Generator() ;

		///
		String_Generator( std::string initial ) ;

		///
		void add_source( std::string x ) ;

		/// Implements Device_Generator::next_device.
		shared_ptr< IO::Device > next_device() ;

		/// Implements Device_Generator::shutdown.
		void shutdown() ;

		/// Implements Device_Generator::completed.
		bool completed() ;
	} ;

	//-------------------------
	/**	\class Old_String_Generator
	 *	\brief A legacy generator of string devices.
	 */
	class Old_String_Generator
		: public Old_Device_Generator
	{
		///
		std::queue< std::string > the_queue ;

		shared_ptr< IO::Device > the_result ;

	public:
		///
		Old_String_Generator() ;

		///
		Old_String_Generator( std::string initial ) ;

		///
		void add_source( std::string x ) ;

		///
		ACT::ACT_State run( ACT::wakeup_listener * ) ;

		/// result accessor
		shared_ptr< IO::Device > result() ;

		void reset() ;
	} ;

} // end namespace IO
#endif
