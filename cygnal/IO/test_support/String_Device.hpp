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

/// \file String_Device.hpp

#pragma once
#ifndef __String_Device_hpp__
#define __String_Device_hpp__

#include "../IO_Device.hpp"
#include <string>

namespace IO {
	//-------------------------
	/** \class String_Device
	 *	\brief A simple string device designed for unit tests.
	 *	
	 */
	class String_Device
		: public Device
	{
		/// Private copy constructor to prohibit copying.
		String_Device( const String_Device & ) ;

		/// [source] The string from which read operations are drawn.
		const std::string the_source_string ;

		/// [source] The master buffer, containing all characters read but not yet consumed.
		contiguous_buffer<> master ;

		/// [source] Whether our read sequence has seen EOF yet
		bool at_eof ;

		/// [source] The segment of the string data we haven't yet read.
		contiguous_buffer<> the_next_segment ;

		/// [source] read activation body
		ACT::act_state source_run( ACT::wakeup_listener * ) ;

		/// [sink] This string receives the results of write operations.
		std::string the_sink_string ;

		/// [sink]
		size_t n_characters_written ;

		/// [sink] buffer to write at next activation
		buffer next_to_write ;

		/// [sink] activation body
		ACT::act_state sink_run( ACT::wakeup_listener * ) ;

	public:
		/// Ordinary constructor
		String_Device( const std::string ) ;

		/// [Source] We always know whether we're at EOF or not.
		bool known_eof() ;

		/// [Source] Are 'n' characters available in the buffer for reading?
		bool characters_available( size_t n = 1 ) ;

		/// [Source] 
		IO::contiguous_buffer<> next_segment() ;

		/// [Source] Consume an initial prefix of the results.
		void consume_up_to( char * ) ;

		/// [Source] 
		void reset() ;

		/// [Source]
		void restart() ;

		/// [Sink in parameter] The buffer that the action will write upon activation.
		void to_write( buffer ) ;

		/// [Sink out parameter]
		size_t n_written() ;

		/// [Sink out parameter]
		std::string string_written() ;
	} ;

} // end namespace IO
#endif
