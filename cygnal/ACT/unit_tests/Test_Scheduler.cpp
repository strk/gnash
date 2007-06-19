// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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

/// \file Test_Scheduler.cpp
/// \brief Tests of the Scheduler

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "ACT/test_support/Simple_Actions.hpp"
#include "ACT/Scheduler.hpp"

using namespace ACT ;

BOOST_AUTO_UNIT_TEST( null )
{
	Basic_Scheduler b ;

	execution_trace tr ;
	b.add_task( act( new no_action( new simple_tracker( tr, "N" ) ) ) ) ;
	b.add_task( act( new single_action( new simple_tracker( tr, "A" ) ) ) ) ;
	b.add_task( act( new single_action( new simple_tracker( tr, "B" ) ) ) ) ;
	b.add_task( act( new single_action( new simple_tracker( tr, "C" ) ) ) ) ;

	b( 100 ) ;
	std::string expected( "ABC" ) ;
	BOOST_CHECK_MESSAGE( tr.result() == expected,
		"Found result " << tr.result() << ". Expected result " << expected << "." ) ;
	BOOST_CHECK( b.empty() ) ;
}

BOOST_AUTO_UNIT_TEST( act_n_interleaved )
{
	Basic_Scheduler b ;
	BOOST_REQUIRE( b.empty() ) ;

	execution_trace tr ;
	b.add_task( act( new N_to_completion( 2, new simple_tracker( tr, "A" ) ) ) ) ;
	b.add_task( act( new N_to_completion( 3, new simple_tracker( tr, "B" ) ) ) ) ;
	b.add_task( act( new N_to_completion( 5, new simple_tracker( tr, "C" ) ) ) ) ;
	
	b( 100 ) ;
	std::string expected( "ABCABCBCCC" ) ;
	BOOST_CHECK_MESSAGE( tr.result() == expected,
		"Found result " << tr.result() << ". Expected result " << expected << "." ) ;
	BOOST_CHECK( b.empty() ) ;
}

//--------------------------------------------------
#include "ACT/Pause_Service.hpp"

BOOST_AUTO_UNIT_TEST( pause_action )
{
	Basic_Scheduler b ;
	BOOST_REQUIRE( b.empty() ) ;

	Pause_Service x( & b ) ;
	b.add_task( act( new no_action( 0 ) ) ) ;
	x() ;
}

