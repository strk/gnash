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

/// \file Test_IO.cpp

#include "config.h"
#include <boost/test/auto_unit_test.hpp>

#include "../IO_Device.hpp"
#include "../test_support/Null_Filter.hpp"
#include "../test_support/Null_Device.hpp"
#include "IO/test_support/String_Device.hpp"

using namespace IO ;

//---------------------------------------------------------------------------
static const char message[] = "Hello, world.\n" ;
#define message_size ( sizeof( message ) / sizeof( char ) - 1 )		// eliminate trailing '\0'

BOOST_AUTO_TEST_CASE( Null_Filter ) 
{
	IO::String_Device device( message ) ;
	IO::null_read_filter filtered( & device ) ;
	filtered() ;
	BOOST_CHECK( filtered.completed() ) ;
	contiguous_buffer<> b( filtered.next_segment() ) ;
	BOOST_REQUIRE( b.begin != 0 ) ;
	BOOST_CHECK_MESSAGE( 0 == strncmp( b.begin, message, message_size ), "Messages sent and received did not match" ) ;
}

BOOST_AUTO_TEST_CASE( Null_Source_Basic )
{
	IO::Null_Source source ;
	BOOST_CHECK( source.completed() ) ;
	source( 0 ) ;
	BOOST_CHECK( source.completed() ) ;
	IO::contiguous_buffer<> r( source.next_segment() ) ;
	BOOST_CHECK( r.length == 0 ) ;
	BOOST_CHECK( source.known_eof() ) ;
}

BOOST_AUTO_TEST_CASE( Null_Sink_Basic )
{
	char stuff[] = "stuff" ;
	result_buffer< char > b( stuff, 5 ) ;

	IO::Null_Sink sink ;
	sink.to_write( b ) ;
	sink() ;
	BOOST_CHECK( sink.completed() ) ;
	BOOST_CHECK( sink.n_written() == 5 ) ;
}

BOOST_AUTO_TEST_CASE( Null_Device_Basic_Read_First )
{
	char stuff[] = "stuff" ;
	result_buffer< char > b( stuff, 5 ) ;
	IO::Null_Device device ;

	device.Source::operator()() ;
	BOOST_CHECK( device.Source::completed() ) ;
	IO::contiguous_buffer<> r( device.next_segment() ) ;
	BOOST_CHECK( r.length == 0 ) ;
	BOOST_CHECK( device.known_eof() ) ;

	device.to_write( b ) ;
	device.Sink::operator()() ;
	BOOST_CHECK( device.Sink::completed() ) ;
	BOOST_CHECK( device.n_written() == 5 ) ;
}

BOOST_AUTO_TEST_CASE( Null_Device_Basic_Write_First )
{
	char stuff[] = "stuff" ;
	result_buffer< char > b( stuff, 5 ) ;
	IO::Null_Device device ;

	device.to_write( b ) ;
	device.Sink::operator()() ;
	BOOST_CHECK( device.Sink::completed() ) ;
	BOOST_CHECK( device.n_written() == 5 ) ;

	device.Source::operator()() ;
	BOOST_CHECK( device.Source::completed() ) ;
	IO::contiguous_buffer<> r( device.next_segment() ) ;
	BOOST_CHECK( r.length == 0 ) ;
	BOOST_CHECK( device.known_eof() ) ;
}

