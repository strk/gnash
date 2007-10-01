// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// \file Supplied_Service.cpp
/// \brief A service that takes an outside supply of actions and supplies them as a service.

#include "Supplied_Service.hpp"
#include <stdexcept>

namespace ACT {
	shared_ptr< basic_act >
	Supplied_Generator::
	next_action( wakeup_listener * )
	{
		if ( ! complete ) {
			if ( ! the_tasks.empty() ) {
				shared_ptr< basic_act > x = the_tasks.front() ;
				the_tasks.pop_front() ;
				return x ;
			} else {
				if ( ! active ) complete = true ;
			}
			/// Assert no more tasks left to return
		}
		return shared_ptr< basic_act >() ;
	}

	void
	Supplied_Generator::
	add_task( shared_ptr< basic_act > x )
	{
		if ( ! active ) throw std::runtime_error( "Not accepting new actions after shutdown" ) ;
		the_tasks.push_back( x ) ;
	}

} // end namespace ACT

#include "ACT/Service.cpp"
namespace ACT {
	template Service< Supplied_Service_Aspect > ;
}
