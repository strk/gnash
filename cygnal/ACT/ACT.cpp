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

/// \file ACT.cpp

#include "ACT.hpp"

#include <stdexcept>
#include <string>

namespace ACT {
	act::
	act( basic_act * x )
		: the_body( x )
	{
		if ( x == 0 ) {
			// The recommended expression for our input parameter is 'new X',
			//		which motivates the text of the exception
			throw std::runtime_error( "Probable memory allocation error" ) ;
		}
	}

	act::
	act( shared_ptr< basic_act > x )
		: the_body( x )
	{}

	act::
	~act()
	{}
}
