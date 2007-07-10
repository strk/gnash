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

/// \file Simple_Actions.cpp
/// \brief Implementations of a set of simple actions with which to test the scheduler

#include "Simple_Actions.hpp"

namespace ACT {
	//--------------------------------------------------
	// simple_action
	//--------------------------------------------------
	ACT_State
	single_action::
	run()
	{
		if ( tracker.get() != 0 ) ( * tracker )( "" ) ;
		return set_completed() ;
	}

	//--------------------------------------------------
	// no_action
	//--------------------------------------------------
	ACT_State
	no_action::
	run()
	{
		if ( tracker.get() != 0 ) ( * tracker )( "" ) ;
		return set_completed() ;
	}

} // end namespace ACT
