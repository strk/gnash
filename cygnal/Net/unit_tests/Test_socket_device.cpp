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

/// \file Test_socket_device.cpp

#include "gnashconfig.h"

#include <boost/test/auto_unit_test.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/thread/thread.hpp>
#include <stdexcept>

#include "../socket_device.hpp"

namespace io = boost::iostreams ;

#define SERVER_PORT	27884
#define server_socket Net::socket_address( Net::loopback, SERVER_PORT )

BOOST_AUTO_UNIT_TEST( First_Test )
{
	// This test ensures that the blocking device plays well with filters.
	// Simply instantiating the class checks a number of template requirements.
	io::stream< Net::socket_device_blocking > x ;
}

BOOST_AUTO_UNIT_TEST( Bind_One )
{
	// Test ordinary constructor
	Net::socket_receptor( Net::socket_address( Net::anylocal, SERVER_PORT ) ) ;
}

BOOST_AUTO_UNIT_TEST( Bind_Two )
{
	// Test port-only constructor
	Net::socket_receptor( SERVER_PORT ) ;
}


Net::socket_receptor
local_server()
{
	try {
		return Net::socket_receptor( server_socket ) ;
	} catch (...) {
		BOOST_FAIL( "Unexpected exception while creating server socket" ) ;
		throw std::logic_error( "Reached a not-reachable statement." ) ;	// present to avoid warning "not all paths return a value"
	}
}

const char message[] = "Hello, world.\n" ;
#define message_size ( sizeof( message ) / sizeof( char ) - 1 )

static void
Listener_Body_One()
{
	Net::socket_receptor server = local_server() ;
	BOOST_CHECKPOINT( "Bound the server port" ) ;
	Net::socket_device_blocking ss = server.next_device_blocking() ;
	BOOST_CHECKPOINT( "Accepted a connection" ) ;
	std::streamsize n = ss.write( message, message_size ) ;
	BOOST_CHECKPOINT( "Wrote a message" ) ;
}

BOOST_AUTO_UNIT_TEST( Server_Writes_Blocking )
{
	try {
		char buffer[ 100 ] ;

		// Start a thread that contains the "server"
		boost::thread server_thread( & Listener_Body_One ) ;

		Net::socket_emitter client( server_socket ) ;
		BOOST_CHECKPOINT( "Initiated a connection" ) ;
		Net::socket_device_blocking cs = client.device_blocking() ;
		std::streamsize n = cs.read( buffer, 100 ) ;
		BOOST_CHECKPOINT( "Read a message" ) ;
		BOOST_CHECK_MESSAGE( 0 == strncmp( buffer, message, message_size ), "Messages sent and received did not match" ) ;

		server_thread.join() ;
	} catch ( std::exception e ) {
		BOOST_FAIL( "std::exception says \"" << e.what() << "\"" ) ;
	} catch ( ... ) {
		BOOST_FAIL( "exception found not derived from std::exception" ) ;
	}
}

static void
Listener_Body_Two()
{
	char buffer[ 100 ] ;

	Net::socket_receptor server = local_server() ;
	BOOST_CHECKPOINT( "Bound the server port" ) ;
	Net::socket_device_blocking ss = server.next_device_blocking() ;
	BOOST_CHECKPOINT( "Accepted a connection" ) ;
	std::streamsize n = ss.read( buffer, 100 ) ;
	BOOST_CHECKPOINT( "Read a message" ) ;
	BOOST_CHECK_MESSAGE( 0 == strncmp( buffer, message, message_size ), "Messages sent and received did not match" ) ;
}

/* In this test, the client writes and the server reads
 */
BOOST_AUTO_UNIT_TEST( Server_Reads_Blocking )
{
	try {
		// Start a thread that contains the "server"
		boost::thread server_thread( & Listener_Body_Two ) ;

		Net::socket_emitter client( server_socket ) ;
		BOOST_CHECKPOINT( "Initiated a connection" ) ;
		Net::socket_device_blocking cs = client.device_blocking() ;
		std::streamsize n = cs.write( message, message_size ) ;
		BOOST_CHECKPOINT( "Wrote a message" ) ;

		server_thread.join() ;
	} catch ( std::exception e ) {
		BOOST_FAIL( "std::exception says \"" << e.what() << "\"" ) ;
	} catch ( ... ) {
		BOOST_FAIL( "exception found not derived from std::exception" ) ;
	}
}

/* Note that this test, like others of non-blocking ACT's, have no need for multiple threads.
 */
BOOST_AUTO_UNIT_TEST( Server_Writes_Nonblocking )
{
	// First make the server and client sockets
	Net::socket_receptor server = local_server() ;
	Net::socket_emitter client( server_socket ) ;

	// Now make the server and client nonblocking devices
	boost::shared_ptr< Net::socket_device_nonblocking > sd = server.next_device_nonblocking() ;
	boost::shared_ptr< Net::socket_device_nonblocking > cd = client.device_nonblocking() ;

	/* Set up a server write and a client read
	 */
	// Use socket_write_act for socket-specific semantics.
	// Use write_act of I/O-generic semantics.
	Net::write_action socket_write_act( *sd, message, message_size ) ;
	IO::write_action_base & write_act = * static_cast< IO::write_action_base * >( & socket_write_act ) ;

	// Ditto for read
	Net::read_action socket_read_act( *cd ) ;
	IO::read_action_base & read_act = * static_cast< IO::read_action_base * >( & socket_read_act ) ;

	// Now exercise a sequence of ACT invocations
	// All of these invocations have empty activation listeners,
	//		hence this is a test only of the basic i/o functions.

	// Check the initial states
	BOOST_CHECK( read_act.working() ) ;	
	BOOST_CHECK( write_act.working() ) ;	
	BOOST_CHECK( ! read_act.completed() ) ;
	BOOST_CHECK( ! write_act.completed() ) ;
	BOOST_CHECK( ! read_act.bad() ) ;
	BOOST_CHECK( ! write_act.bad() ) ;

	read_act( 0 ) ;
	// should not complete because nothing has been written yet.
	BOOST_CHECK( read_act.working() ) ;	

	read_act( 0 ) ;
	// should still be no change
	BOOST_CHECK( read_act.working() ) ;

	write_act( 0 ) ;
	// This action should complete because write buffer is empty.
	BOOST_CHECK( write_act.completed() ) ;
	// Mere writing should not invoke the read action simply because the read might succeed.
	// This would violate the separation of scheduling and action
	BOOST_CHECK( read_act.working() ) ;

	write_act( 0 ) ;
	// Acting on a completed item should do nothing.
	BOOST_CHECK( write_act.completed() ) ;
	BOOST_CHECK_MESSAGE( write_act.n_written() == message_size, 
		"Wrote " << write_act.n_written() << ". Expected to write " << message_size << "." ) ;

	read_act( 0 ) ;
	// This action might or might not succeed because of routing delay
	int j ;
	for ( j = 0 ; read_act.working() && j < 10 ; ++ j ) {
		// We'll try once more after a delay.  To delay, we'll open a file.
		int x = open( "foo", 0 ) ;
		if ( x >= 0 ) close( x ) ;
		read_act( 0 ) ;
	}
	// Assert j >= 10 \implies read_act.working() last returned true.
	BOOST_CHECK_MESSAGE( j < 10, "Tried ten times to complete the read and it's still working!" ) ;
	BOOST_CHECK( read_act.completed() ) ;

	read_act( 0 ) ;
	// an execution after completion should do nothing
	BOOST_CHECK( read_act.completed() ) ;
	IO::contiguous_buffer<> b( cd -> next_segment() ) ;
	BOOST_CHECK_MESSAGE( b.length == message_size,
		"Read " << b.length << ". Expected to read " << message_size << "." ) ;
	BOOST_REQUIRE( b.begin != 0 ) ;

	// Just ensure nothing went bad along the way
	BOOST_CHECK( ! read_act.working() ) ;	
	BOOST_CHECK( ! write_act.working() ) ;	
	BOOST_CHECK( ! read_act.bad() ) ;
	BOOST_CHECK( ! write_act.bad() ) ;

	// The messages should match.
	BOOST_CHECK_MESSAGE( 0 == strncmp( b.begin, message, message_size ), "Messages sent and received did not match" ) ;

}


/* This test of the source fixture checks that a blocking client can talk to a non-blocking server.
 */
/*
BOOST_AUTO_TEST_CASE( Fixture_OK )
{
	char buffer[ 100 ] ;
	Net::socket_device_nonblocking sd = server_device( message ) ;

	Net::read_action read_act( sd, buffer, 100 ) ;
	read_act( 0 ) ;
	BOOST_CHECK( read_act.completed() ) ;
	BOOST_CHECK_MESSAGE( 0 == strncmp( buffer, message, message_size ), "Messages sent and received did not match" ) ;
}
*/

#include <boost/bind.hpp>

//#define SERVER_PORT	27884
//static Net::socket_address server_socket( Net::loopback, SERVER_PORT ) ;

static void
Client_Body_One( const std::string x )
{
	Net::socket_emitter client( server_socket ) ;
	// We can make this output block because it's in its own thread

	Net::socket_device_blocking cs = client.device_blocking() ;
	std::streamsize n = cs.write( x.c_str(), static_cast< std::streamsize >( x.length() ) ) ;
}

boost::shared_ptr< Net::socket_device_nonblocking >
server_device( const std::string HTTP_message ) {
	Net::socket_receptor ss( server_socket ) ;
	// If the server socket isn't set up at this point, the constructor will fail because TCP cannot connect.
	boost::thread client_thread( boost::bind( & Client_Body_One, HTTP_message ) ) ;

	// Wait just a bit.  
	// This is avoid a race condition between client and server.  
	// It may not be reliable, in the sense that if HTTP_message is too large, the write() call in the client
	//		may never complete unless the server, whose reader is first instantiated after this, reads, which it can't.
	client_thread.join() ;

	// Note that next_device_nonblocking() is a blocking call and waits for an incoming connection.
	// If a client is not already sending data, this call will block.
	return ss.next_device_nonblocking() ;
}

//---------------------------------------------------------------------------
// end of socket-specific section
//---------------------------------------------------------------------------

void Null_Body()
{
}

class t1 {
	boost::thread it ;
public:
	t1() 
		: it( & Null_Body )
	{}
} ;

class t2 {
	boost::thread it ;
public:
	t2() 
		: it( & Null_Body )
	{}

	~t2()
	{
		it.join() ;
	}
} ;

/*
BOOST_AUTO_TEST_CASE( Validate_Thread_Usage )
{
	{
		t1 x ;
	}{
		t2 y ;
	}
}
*/
