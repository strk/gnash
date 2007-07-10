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

/// \file IO_Generator.cpp

#include "IO_Generator.hpp"

namespace IO {
	//-------------------------
	Device_Adapter::
	Device_Adapter( Device_Generator & x, Behavior_Factory & y ) 
		: the_generator( x ),
		the_behavior_factory( y )
	{}

	//-------------------------
	ACT::ACT_State
	Device_Adapter::
	run( ACT::wakeup_listener * w )
	{
		the_generator( w ) ;
		if ( the_generator.bad() ) return set_bad() ;
		if ( the_generator.working() ) return set_ready() ;
		// Assert the_generator has a result ready for us.
		
		the_result = the_behavior_factory( the_generator.result() ) ;
		return set_completed() ;
	}

	//-------------------------
	ACT::act 
	Device_Adapter::
	result()
	{
		return the_result ;
	}


} // end namespace IO
