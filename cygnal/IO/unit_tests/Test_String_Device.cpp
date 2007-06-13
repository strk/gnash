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

/// \file Test_String_Device.cpp

#include <boost/test/auto_unit_test.hpp>
#include "../test_support/String_Device.hpp"
using namespace IO ;

//---------------------------------------------------------------------------
static const char message[] = "Hello, world.\n" ;
#define message_size ( sizeof( message ) / sizeof( char ) - 1 )		// eliminate trailing '\0'

BOOST_AUTO_TEST_CASE( String_Device_Read )
{
	IO::String_Device device( message ) ;
	IO::Source & source( device ) ;

	BOOST_CHECK( source.working() ) ;
	contiguous_buffer<> b( source.next_segment() ) ;
	source() ;
	BOOST_CHECK( source.completed() ) ;
	b = source.next_segment() ;
	BOOST_CHECK_MESSAGE( b.length == message_size,
		"Read " << b.length << " characters. Expected to read " << message_size << " characters." ) ;
	BOOST_REQUIRE( b.begin != 0 ) ;
	BOOST_CHECK_MESSAGE( 0 == strncmp( b.begin, message, message_size ), "Messages sent and received did not match" ) ;

	// Now test consuming only part of the string.
	source.consume_up_to( b.begin + 1 ) ;
	device.restart() ;
	BOOST_CHECK( source.working() ) ;
	b = source.next_segment() ;
	BOOST_CHECK( b.length == 0 ) ;
	source() ;
	BOOST_CHECK( source.completed() ) ;
	b = source.next_segment() ;
	BOOST_CHECK_MESSAGE( b.length == message_size - 1,
		"Read " << b.length << " characters. Expected to read " << message_size - 1 << " characters." ) ;\
	BOOST_REQUIRE( b.begin != 0 ) ;
	BOOST_CHECK_MESSAGE( 0 == strncmp( b.begin, message + 1, message_size - 1 ), "Messages sent and received did not match" ) ;
}

BOOST_AUTO_TEST_CASE( String_Device_Write )
{
	E_AV_Buffer< 2 > x2 ;
	char first_half[] = "12" ;
	char second_half[] = "345" ;
	x2.append( first_half, 2 ) ;
	x2.append( second_half, 3 ) ;

	IO::String_Device device( "" ) ;
	device.to_write( x2 ) ;
	device.Sink::operator()() ;
	BOOST_CHECK( device.Sink::completed() ) ;
	BOOST_CHECK( device.n_written() == 5 ) ;
}