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

/// \file Pause_Demon.cpp
///	\brief The Pause service suspends itself (and its thread) for a number of milliseconds 
///		if the scheduling queue has no high-priority items.

#include "Pause_Demon.hpp"
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>

namespace ACT {
	//-------------------------
	ACT_State
	Pause_Demon::
	run()
	{
		if ( the_scheduler -> ordinary_tasks_available() )
			return ACT_State::Ready ;

		boost::xtime t ;
		if ( 0 == xtime_get( & t, boost::TIME_UTC ) ) {
			return set_bad() ;
		}
		t.nsec += ms_to_pause * 1000 * 1000 ;
		boost::thread().sleep( t ) ;
		return ACT_State::Ready ;
	}

	//-------------------------
} // end namespace ACT
