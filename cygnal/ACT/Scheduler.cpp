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

/// \file Scheduler.cpp

#include "Scheduler.hpp"

namespace ACT {

	//--------------------------------------------------
	// Basic_Scheduled_Item
	//--------------------------------------------------
	Basic_Scheduled_Item::
	Basic_Scheduled_Item( act x, unsigned int n, Action_Category action_type )
		: the_action( x ),
		sequence_number( n ),
		action_type( action_type )
	{
		switch ( action_type ) {
			case Task :
				priority_category = Initial ;
				break ;
			case Demon :
				priority_category = Background ;
				break ;
			default :
				priority_category = Background ;
				break ;
		}
	}

	//-------------------------
	bool
	Basic_Scheduled_Item::
	operator<( const Basic_Scheduled_Item & x ) const
	{
		return	priority_category > x.priority_category
			||	(		priority_category == x.priority_category 
					&&	sequence_number > x.sequence_number 
				) ;
	}

} // end namespace ACT
