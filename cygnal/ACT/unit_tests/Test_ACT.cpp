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

/// \file Test_ACT.cpp
/// \brief Tests of the simple actions, exercising the basic ACT model.

#include <boost/test/auto_unit_test.hpp>
#include <boost/test/test_tools.hpp>

#include "../ACT.hpp"
#include "ACT/test_support/Simple_Actions.hpp"

using namespace ACT ;

//--------------------------------------------------
/** \class act_twice
 *	\brief A test class for ACT.
 *	Behavior is to complete only after two calls to operator().
 *
 *	Suggestions for use:
 *	- As a well-formed and simple ACT to check compilation
 *	- As a non-notifying ACT to exercise a scavenger thread in a scheduler.
 */
class act_twice
	: public autonomous_act
{
	int number_of_calls ;
public:
	act_twice()
		: number_of_calls( 0 )
	{} ;

	act_state run( wakeup_listener * ) 
	{
		switch ( number_of_calls ) {
			case 0 :
				number_of_calls = 1 ;
				return ACT::Working ;
			case 1 :
				number_of_calls = 2 ;
				return set_completed() ;
			case 2 :
				return ACT::Completed ;
		}
		throw std::exception( "Not Reached" ) ;
	}
} ;

//--------------------------------------------------

BOOST_AUTO_UNIT_TEST( act_twice_works )
{
	 act x( new act_twice() ) ;

	 BOOST_CHECK_MESSAGE( x.working(), "initial state should be 'Working'" ) ;
	 x( 0 ) ;
	 BOOST_CHECK_MESSAGE( x.working(), "state after one call should still be 'Working'" ) ;
	 x( 0 ) ;
	 BOOST_CHECK_MESSAGE( x.completed(), "state after two calls should be 'Completed'" ) ;
	 x( 0 ); 
	 BOOST_CHECK_MESSAGE( x.completed(), "state after three calls should still be 'Completed'" ) ;
}

BOOST_AUTO_UNIT_TEST( no_action_works )
{
	execution_trace trace ;
	act x( new no_action( new simple_tracker( trace, "N" ) ) ) ;

	BOOST_CHECK_MESSAGE( x.completed(), "initial state should be 'Completed'" ) ;
	x( 0 ) ;
	BOOST_CHECK_MESSAGE( x.completed(), "final state should be 'Completed'" ) ;
	BOOST_CHECK_MESSAGE( trace.result() == "", "Tracking should not have recorded an activation, since action was already completed" ) ;
}

BOOST_AUTO_UNIT_TEST( act_n_equals_two_works )
{
	execution_trace trace ;
	act x( new N_to_completion( 2, new simple_tracker( trace, "C" ) ) ) ;
	
	BOOST_CHECK_MESSAGE( x.working(), "initial state should be 'Working'" ) ;
	BOOST_CHECK( trace.result() == "" ) ;
	x( 0 ) ;
	BOOST_CHECK_MESSAGE( x.working(), "state after one call should still be 'Working'" ) ;
	BOOST_CHECK( trace.result() == "C" ) ;
	x( 0 ) ;
	BOOST_CHECK_MESSAGE( x.completed(), "state after two calls should be 'Completed'" ) ;
	BOOST_CHECK( trace.result() == "CC" ) ;
	x( 0 ); 
	BOOST_CHECK_MESSAGE( x.completed(), "state after three calls should still be 'Completed'" ) ;
	BOOST_CHECK( trace.result() == "CC" ) ;
}

