// 
//   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
	IO_Generator::
	IO_Generator( Old_Device_Generator & x, Behavior_Factory & y ) 
		: the_generator( x ),
		the_behavior_factory( y ),
		complete( false )
	{}

	//-------------------------
	shared_ptr< ACT::basic_act >
	IO_Generator::
	next_action( ACT::wakeup_listener * w )
	{
		if ( ! complete ) {
			the_generator( w ) ;
			if ( the_generator.completed() ) {
				// Assert the_generator has a result ready for us.
				return the_behavior_factory( the_generator.result() ) ;
			}
			// Assert no result is ready; return must be null

			if ( the_generator.bad() ) {
				// A good generator is required for continued operation.
				set_completed() ;
			}
		}
		return shared_ptr< ACT::basic_act >() ;
	}

} // end namespace IO
