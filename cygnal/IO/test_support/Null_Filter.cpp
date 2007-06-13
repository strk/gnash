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

/// \file Null_Filter.cpp

#include "Null_Filter.hpp"

namespace IO {
	//---------------
	null_read_filter::
	null_read_filter( Source * source )
		: the_source( source )
	{
	}

	//---------------
	bool
	null_read_filter::
	known_eof()
	{
		return the_source -> known_eof() ;
	}

	//---------------
	bool 
	null_read_filter::
	characters_available( size_t n )
	{
		return the_source -> characters_available() ;
	}

	//---------------
	ACT::act_state
	null_read_filter::
	run( ACT::wakeup_listener * w )
	{
		return set_state( the_source -> operator()( w ) ) ;
	}

} // end namespace IO
