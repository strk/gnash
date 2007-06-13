// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// \file Test_Service.cpp

#include <boost/test/auto_unit_test.hpp>

#include "../test_support/String_Generator.hpp"
#include "ACT/Service.hpp"

class Null_Behavior
	: public IO::Behavior
{
public:
	Null_Behavior() { set_completed() ; }
	ACT::act_state run( ACT::wakeup_listener * ) { return ACT::Completed ; }
} ;

//-------------------------
shared_ptr< ACT::autonomous_act >
Null_Behavior_Factory( shared_ptr< IO::Device > )
{
	return shared_ptr< ACT::autonomous_act >( new Null_Behavior() ) ;
}

BOOST_AUTO_UNIT_TEST( Generator_Basic )
{
	IO::String_Generator generator( "a" ) ;
	generator.add_source( "b" ) ;
	generator.add_source( "c" ) ;

	BOOST_CHECK( generator.working() ) ;
	generator( 0 ) ;
	BOOST_CHECK( generator.completed() ) ;
	shared_ptr< IO::Device > device( generator.result() ) ;
	// Check contents of source here. Should be "a".
	generator.reset() ;

	BOOST_CHECK( generator.working() ) ;
	generator( 0 ) ;
	BOOST_CHECK( generator.completed() ) ;
	device = generator.result() ;
	// Check contents of source here. Should be "b".
	generator.reset() ;

	BOOST_CHECK( generator.working() ) ;
	generator( 0 ) ;
	BOOST_CHECK( generator.completed() ) ;
	device = generator.result() ;
	// Check contents of source here. Should be "c".
	generator.reset() ;

	BOOST_CHECK( generator.working() ) ;
	generator( 0 ) ;
	// A fourth activation should remain working, because the internal queue has emptied.
	BOOST_CHECK( generator.working() ) ;
}

BOOST_AUTO_UNIT_TEST( Service_Basic )
{
	IO::String_Generator x( "" ) ;
	x.add_source( "" ) ;
	x.add_source( "" ) ;

}