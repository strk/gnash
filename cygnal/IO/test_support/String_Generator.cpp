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

/// \file String_Generator.cpp

#include "String_Generator.hpp"
#include "String_Device.hpp"

namespace IO {

	String_Generator::
	String_Generator()
		: the_result( new String_Device( "" ) )
	{
	}

	String_Generator::
	String_Generator( std::string initial )
		: the_result( new String_Device( "" ) )
	{
		add_source( initial ) ;
	}

	///
	ACT::act_state 
	String_Generator::
	run( ACT::wakeup_listener * )
	{
		if ( the_queue.empty() ) return ACT::Working ;
		the_result = shared_ptr< IO::Device >( new String_Device( the_queue.front() ) ) ;
		the_queue.pop() ;
		return set_completed() ;
	}

	void
	String_Generator::
	add_source( std::string x )
	{
		the_queue.push( x ) ;
	}

	shared_ptr< IO::Device >
	String_Generator::
	result()
	{
		return the_result ;
	}

	void
	String_Generator::
	reset()
	{
		if ( bad() ) return ;
		set_working() ;
	}

} // end namespace IO

