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

/// \file String_Device.cpp

#include "String_Device.hpp"
#include <string>

namespace IO {
	//-------------------------
	String_Device::
	String_Device( const std::string x )
		: the_source_string( x ),
		at_eof( false ),
		master( const_cast< char * >( the_source_string.data() ), the_source_string.length() ),
		the_next_segment( master.begin, 0 ),
		n_characters_written( 0 )
	{
	}

	//--------------------------------------------------
	// Source portion of String_Device 
	//--------------------------------------------------

	//-------------------------
	// Since we read the entire string on the first action, we are always at EOF after the first read.
	bool 
	String_Device::
	known_eof() 
	{
		return at_eof ;
	}

	//-------------------------
	void
	String_Device::
	consume_up_to( char * x )
	{
		// Guard for validity of X.
		// We may consume from 0 to length, so our validity range has size "length + 1".  Tricky.
		if ( x < master.begin || master.begin + master.length + 1 < x ) {
			// Assert x isn't part of the master buffer
			throw std::exception( "May not consume to a point not in the master buffer" ) ;
		}
		master = contiguous_buffer<>( x, ( master.begin + master.length ) - x ) ;
	}

	//-------------------------
	bool
	String_Device::
	characters_available( size_t n )
	{
		return n <= the_next_segment.length ;
	}

	//-------------------------
	IO::contiguous_buffer<> 
	String_Device::
	next_segment()
	{
		IO::contiguous_buffer<> return_value = the_next_segment ;

		the_next_segment.begin = return_value.end() ;
		the_next_segment.length = std::max( master.end() - the_next_segment.begin, 0 ) ;

		return return_value ;
	}

	//-------------------------
	ACT::ACT_State
	String_Device::
	source_run( ACT::wakeup_listener * )
	{
		if ( known_eof() ) {
			// Some other read invocation has already taken our data.
			// Nothing further to do.
		} else {
			/// Read the entire string on an available 
			/// Indicate that we've read the string.  One read is all it takes.
			if ( master.end() == the_source_string.data() + the_source_string.length() ) {
				// Assert we've already read to the end of the string.
			} else {
				// Assert we still have something left to read.
				// Thus read in the whole length of the string.
				master.length = the_source_string.length() ;
				the_next_segment.length = master.length ;
			}
		}
		at_eof = true ;
		/// We're done, regardless.
		return SSource::set_completed() ;
	}

	//-------------------------
	void
	String_Device::
	reset()
	{
		SSource::set_ready() ;
	}

	//-------------------------
	void
	String_Device::
	restart()
	{
		reset() ;
		if ( master.length > 0 ) at_eof = false ;
		the_next_segment.begin = master.begin ;
		the_next_segment.length = 0 ;
	}

	//--------------------------------------------------
	// Sink portion of String_Device 
	//--------------------------------------------------

	//-------------------------
	void
	String_Device::
	to_write( buffer b )
	{
		next_to_write = b ;
	}

	//-------------------------
	ACT::ACT_State
	String_Device::
	sink_run( ACT::wakeup_listener * )
	{
		buffer::const_segment_iterator next( next_to_write.segment_begin() ) ;
		buffer::const_segment_iterator end( next_to_write.segment_end() ) ;

		while ( next != end ) {
			the_sink_string += std::string( (*next).begin, (*next).length ) ;
			n_characters_written += (*next).length ;
			++ next ;
		}
		next_to_write = buffer() ;
		return Sink::set_completed() ;
	}

	//-------------------------
	std::string
	String_Device::
	string_written()
	{
		return the_sink_string ;
	}

	//-------------------------
	size_t
	String_Device::
	n_written()
	{
		return n_characters_written ;
	}

	//-------------------------
} // end namespace IO

