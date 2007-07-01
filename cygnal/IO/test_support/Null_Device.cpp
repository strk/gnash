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

/**	\file Null_Device.hpp
 *	\brief 
 */

#include "Null_Device.hpp"

namespace IO {
	//-------------------------
	// Null_Source
	//-------------------------
	Null_Source::
	Null_Source()
	{
		set_completed() ;
	}

	ACT::act_state
	Null_Source::
	run( ACT::wakeup_listener * )
	{
		return internal_state() ;
	}

	void
	Null_Source::
	reset()
	{}

	bool
	Null_Source::
	known_eof()
	{
		return true ;
	}

	bool
	Null_Source::
	characters_available( size_t )
	{
		return false ;
	}

	IO::contiguous_buffer<> 
	Null_Source::
	next_segment()
	{
		return IO::contiguous_buffer<>() ;
	}

	void 
	Null_Source::
	consume_up_to( char * )
	{}

	//-------------------------
	// Null_Device
	//-------------------------
	Null_Device::
	Null_Device()
		: the_source(),
		the_sink(),
		Device( Source_Adapter< Null_Source >( the_source ), Sink_Adapter< Null_Sink >( the_sink ) )
	{}

} // end of namespace IO
