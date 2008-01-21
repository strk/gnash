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

/**	\file Old_Device.cpp
 *	
 */

#include "Old_Device.hpp"

namespace IO {

	//-------------------------
	read_action_base::
	read_action_base()
	{}

	//-------------------------
	write_action_base::
	write_action_base( const char * s, unsigned int n )
		: oldstyle_buffer( s ),
		oldstyle_buffer_size( n ),
		number_written( 0 )
	{
		if ( n == 0 ) {
			// Nothing to do if buffer to write is empty
			set_completed() ;
		}
	}

} // end namespace IO

