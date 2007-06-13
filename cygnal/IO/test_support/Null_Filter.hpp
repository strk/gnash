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

/** \file Null_Filter.hpp
 *	\brief A filter that simply forward read/write requests to an underlying device.
 *		Use for testing and as a starting point for writing new filters and devices.
 */

#pragma once
#ifndef __Null_Filter_hpp__
#define __Null_Filter_hpp__

#include "../IO_Device.hpp"
 
namespace IO {

	//-------------------------
	/** \class null_read_filter
	 */
	class null_read_filter
		: public Source
	{
		/// The source to which to forward our own read actions.
		Source * the_source ;

		/// 
		ACT::act_state run( ACT::wakeup_listener * ) ;

	public:
		/// Ordinary constructor
		null_read_filter( Source * ) ;

		///
		bool known_eof() ;

		/// Are 'n' characters available in the buffer for reading?
		bool characters_available( size_t n = 1 ) ;

		///
		IO::contiguous_buffer<> next_segment() { return the_source -> next_segment() ; }

		///
		void consume_up_to( char * x ) { return the_source -> consume_up_to( x ) ; }

		/// Polymorphic reset
		inline void reset() { the_source -> reset() ; }
	} ;

} // end namespace IO
#endif
