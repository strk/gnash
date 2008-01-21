// 
// Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

/// \file Test_Support.cpp
/// \brief Generic Support for Unit Tests

#include "Test_Support.hpp"

boost::unit_test::test_case * 
auto_generator::
next() const
{
	if ( done ) return 0 ;
	shared_ptr<test> the_base_test( new test( g -> test_function() ) ) ;
	boost::unit_test::test_case * y = boost::unit_test::make_test_case( & test::run, basename + g -> name(), the_base_test ) ;
	g -> advance() ;
	done = g -> done() ;
	return y ;
}
