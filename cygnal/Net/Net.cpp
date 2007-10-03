// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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

/// \file Net.cpp

#include "Net.hpp"
#include "socket_device.hpp"
#include "../libbase/log.h"

#include <errno.h>
#include <stdexcept>

#ifndef HAVE_WINSOCK_H
#	define close_socket close
#else
#	define close_socket closesocket
#endif


gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance() ;

/** \class exception
 */
class exception 
	: public std::exception
{
public:
	exception( std::string x ) 
		: std::exception( x.c_str() )
	{
		dbglogfile << x << std::endl ;
	}
} ;

namespace Net {
	//-------------------------
	const interface_address anylocal( INADDR_ANY ) ;
	const interface_address loopback( inet_addr( "127.0.0.1" ) ) ;

	//--------------------------------------------------
	// socket_address
	//--------------------------------------------------

	//-------------------------
	socket_address::
	socket_address( interface_address x, port_address y )
	{
		the_socket.sin_family = AF_INET ;
		the_socket.sin_addr.s_addr = x ;
		the_socket.sin_port = y ;
	}


#ifdef HAVE_WINSOCK_H
	//--------------------------------------------------
	// net_use_guard
	//--------------------------------------------------

	//-------------------------
	// Initialization for class variable
	unsigned int
	net_use_guard::call_count = 0 ;

	//-------------------------
	net_use_guard::
	net_use_guard()
	{
		if ( call_count == 0 ) {
			WORD wVersionRequested ;
			WSADATA wsaData ;
			wVersionRequested = MAKEWORD( 1, 1 ) ;		// Require Windows Sockets 1.1
			if ( WSAStartup( wVersionRequested, & wsaData ) != 0 ) {
				throw std::runtime_error( "Could not initialize winsock" ) ;
			}
		}
		++ call_count ;
	}

	//-------------------------
	net_use_guard::
	~net_use_guard() 
	{
		-- call_count ;
		if ( call_count == 0 ) {
		    WSACleanup() ;
		}
	}
#else
	// If no functionality is needed in the guard class, then its full definition is in Net.hpp.
#endif

	//--------------------------------------------------
	// body_of_socket_handle
	//--------------------------------------------------

	//-------------------------
	body_of_socket_handle::
	body_of_socket_handle() 
		: please_move_along(),
		// We use 0 for the default protocol here rather than searching a protocol table
		// for the identifier "tcp".  We don't anticipate that there's a default streaming 
		// protocol that's not TCP, at least on any platform we care about here.
		the_handle( socket( PF_INET, SOCK_STREAM, 0 ) )
	{
		if ( the_handle < 0 ) {
			throw_exception( "Did not create socket" ) ;
		}
	}

	//-------------------------
	/** \par Implementation
	 *	Note that we need an ordinary sentry here, not for its constructor, but for its destructor.
	 *	Since the \c handle parameter was received from some networking function, 
	 *		we must assume that networking was already initialized.
	 *	The lifetime of this object, however, may continue past that of any that spawned it.
	 *	Since the networking system must remain in place for the close() call,
	 *		we use the networking sentry as normal.
	 */
	body_of_socket_handle::
	body_of_socket_handle( socket_t handle )
		: please_move_along(),
		the_handle( handle )
	{}

	//-------------------------
	body_of_socket_handle::
	~body_of_socket_handle() {
		// We must check the return value, in case the call to close() was interrupted by a signal.
		// Note: \c close_socket is a platform-specific macro
		// A potential infinite loop in return of EINTR is handled with a counter.
		int close_counter = 0 ;
		while ( close_socket( the_handle ) < 0 ) {
			if ( last_error() == EINTR ) {
				// Assert close failed because of an interruption.
				++ close_counter ;
				if ( close_counter == 1000 ) {
					// Assert We've had 1000 EINTR in a row.
					// Something is seriously wrong.  
					throw_exception( "Did not close socket after many interruptions; suspect system corruption" ) ;
				}
				// Skip throwing an exception and retry the call.
				continue ;
			}
			// Assert close failed for some other reason, which is a non-recoverable error for us.
			throw_exception( "Did not close socket" ) ;
		}
		// Assert close() returned a value >= 0
	}

	void
	body_of_socket_handle::
	throw_exception( std::string message )
	{
		std::stringstream ss ;
		ss << message << " [errno=" << strerror( last_error() ) << "]" ;
		throw exception( ss.str() );
	}

	//--------------------------------------------------
	// socket_handle
	//--------------------------------------------------

	//-------------------------
	socket_handle::
	socket_handle()
		: the_body( shared_ptr< body_of_socket_handle >( new body_of_socket_handle() ) )
	{}

	//-------------------------
	socket_handle::
	socket_handle( socket_t x )
		: the_body( shared_ptr< body_of_socket_handle >( new body_of_socket_handle( x ) ) )
	{}

	//-------------------------
	socket_t
	socket_handle::
	the_handle() const
	{
		return the_body -> the_handle ;
	}

	//-------------------------
	int
	last_error()
	{
#ifdef HAVE_WINSOCK_H
		return WSAGetLastError() ;
#else
		return errno ;
#endif
	}
	//-------------------------
	// \par Design Note
	// Setting a socket only fails when there's been a system error (undetectable 
	// from here) or when this implementation has a defect (as-yet undetected, clearly).
	// Neither situation is actionable, so this function returns void and signals
	// failure by throwing an exception.
	void
	socket_handle::
	set_option( int option, void * param, int option_length ) {
		int x = setsockopt( the_handle(), SOL_SOCKET, option, (char *) param, option_length ) ;
		if ( x < 0 ) {
			// Assert setsockopt failed
			throw_exception( "Did not set option" ) ;
		}
	}

	//-------------------------
	void 
	socket_handle::
	set_option_reuse_address( bool reuse ) {
		int param = reuse ? 1 : 0 ;
		set_option( SO_REUSEADDR, & param, sizeof( param ) ) ;
	}

	//-------------------------
	void 
	socket_handle::
	set_option_non_blocking( bool non_blocking ) {
#ifdef HAVE_WINSOCK_H
		unsigned long param = non_blocking ? 1 : 0 ;
		// FIONBIO stands for, best I can tell "File I/O Non-Blocking I/O"
		ioctlsocket( the_handle(), FIONBIO, & param ) ;
#else
	    fcntl( the_handle, F_SETFL, O_NONBLOCK) ;
#endif
	}

	//-------------------------
	/// \todo Existing exception message doesn't adequately describe the socket that
	/// generated the error.  The handle is available as an integer and not yet used.
	void
	socket_handle::
	throw_exception( std::string message )
	{
		std::stringstream ss ;
		ss << message << " [errno=" << strerror( last_error() ) << "]" ;
		throw exception( ss.str() );
	}

	//--------------------------------------------------
	// socket_receptor
	//--------------------------------------------------

	//-------------------------
	/// \todo Inadequate source information; requires expansion.
	void
	socket_receptor::
	throw_exception( std::string message )
	{
		std::stringstream ss ;
		ss << message << " [errno=" << strerror( last_error() ) << "]" ;
		throw exception( ss.str() );
	}

	//-------------------------
	/*
	 */
	socket_receptor::
	socket_receptor( socket_address requested )
		: the_handle()
	{
		init( requested ) ;
	}

	//-------------------------
	/*
	 */
	socket_receptor::
	socket_receptor( in_port_t x )
		: the_handle()
	{
		init( socket_address( anylocal, x ) ) ;
	}

	//-------------------------
	/** \par Implementation
	 *		We set the flag SO_REUSEADDR on the socket so that it will bind without error
	 *	when the socket has existing connections in the TIME_WAIT state.  This allows restarting
	 *	a running server, with existing connections, without the delay required to time-out all
	 *	the existing connections.
	 *	\par
	 *		There are additional socket calls in this class over and above what are in 
	 *	socket_handle. This is on purpose.  Calling \c bind and \c listen are appropriate on
	 *	a server socket, not on a client one.  Thus we separate out server-specific interaction
	 *	into a separate class.
	 */
	void
	socket_receptor::
	init( socket_address requested )
	{
		the_handle.set_option_reuse_address( true ) ;
		the_handle.set_option_non_blocking( true ) ;
		// Assert setting the options succeeded (otherwise there would have been an exception)
		
		// We only try once to bind, rather than multiple times, 
		// because reusing the address should suffice to handle the one recoverable condition
		// we may encounter.
		// Disclaimer: Omission of a retry loop may not work.
		// Oops.  EINTR not handled here.
		int x = bind( the_handle, requested.struct_ptr(), requested.struct_size() ) ;
		if ( x < 0 ) {
			// Assert bind failed
			throw_exception( "bind failed" ) ;
		}

		// TODO: inserted logging of successful address assignment here

		x = listen( the_handle, queue_length ) ;
		if ( x < 0 ) {
			// Assert listen failed
			throw_exception( "listen failed" ) ;
		}
	}

	//-------------------------
	/** \par Limitations
	 *	- We do not wait on error conditions for the socket.
	 *	- We do not keep track of internal state, in particular of pending shutdown.
	 *	- We do protect the select call with a mutex to atomically change its state and
	 *		also to record it.
	 */
	bool
	socket_receptor::
	wait_for_connection()
	{
		// The most recent return value of select()
		int x ;

		// Set up a descriptor array for the select call
		fd_set input_descriptors ;
		FD_ZERO( & input_descriptors ) ;
		FD_SET( the_handle, & input_descriptors ) ;

		// Wait for input on the handle
		// We wait in a loop to handle interrupted calls.
		int call_counter = 0 ;
		while ( true ) {
			// The loop is a wrapper around this select() call.
			x = select( 1 + (int) the_handle, & input_descriptors, NULL, NULL, NULL ) ;
			// Assert x == 0 implies an interrupted call, because we don't time out
			// Assert x < 0 implies an error
			// Assert X > 0 implies a waiting handle
			if ( x > 0 ) {
				// Assert x == 1, because we set only one handle and only select on input
				// select succeeded, so we break out of the loop.
				break ;
			} else if ( x == 0 ) {
				// We have an interruption
				/// \todo Distinguish between ordinary interruptions, which indicate a retry,
				/// and shutdown requests, which do not.
				continue ;
			}
			// Assert x <= 0
			// If x == 0, we have an implementation defect with select()
			// Assert (derived) x < 0
			if ( last_error() == EINTR ) {
				// Assert select was interrupted
				// If we have already tried too often, we give up
				if ( ++ call_counter == 1000 ) {
					throw_exception( "Did not select after many interruptions; suspect system corruption" ) ;
				}
				// Try again
				continue ;
			}
			// Assert x < 0 and error code != EINTR
			// None of the other error conditions is recoverable.
			throw_exception( "select failed" ) ;
		}
		return true ;
	}

	//-------------------------
	/** \par Limitations
	 *	- We don't do anything related to shutdown
	 */
	socket_handle
	socket_receptor::
	next_connection()
	{
		if ( ! wait_for_connection() ) {
			// Assert no connection was present after select
			// This is an ordinary condition after an external shutdown request.
			// For now, we ignore this.
			// [2007-04-17] As of this writing, wait_for_connection() never returns false
			throw_exception( "waited for connection but found none" ) ;
		}
		// Assert wait_for_connection() returned true
		// In ordinary circumstances, that means there is a connection on the listening queue.
		// However, that might not be true.
		// The connection might have disappeared in the interim.
		// Hence we don't assert that there is actually anything in the listening queue.

		// We're going to ignore the return socket side.
		// Thus we don't care what address family is used and we use the generic structure.
		struct sockaddr the_other_side ;
		int addr_size = sizeof( the_other_side ) ;

		// Hence the call to accept should not block and we must also check its return code.
		socket_t stream_handle = accept( the_handle, & the_other_side, & addr_size ) ;
		if ( stream_handle < 0 ) {
			// Assert accept() failed
			if ( last_error() == EWOULDBLOCK ) {
				// This is an ordinary error, albeit pretty unusual.
				// Yes, we waited for a connection to be present before calling accept.
				// BUT, the listen queue is, as it were, declared "volatile".
				// Therefore, the listen queue may have emptied between the return of select()
				//		and the call to accept().
				// Our action here is to throw an exception.
				throw std::exception() ;
			}
			// Other errors are not recoverable
			throw_exception( "accept failed" ) ;
		}
		return socket_handle( stream_handle ) ;
	}

	//-------------------------
	socket_device_blocking
	socket_receptor::
	next_device_blocking()
	{
		// Assert stream_handle is the handle of a connected socket ready for I/O
		// Finally, construct a device from the new socket
		return socket_device_blocking( next_connection() ) ;
	}

	//-------------------------
	boost::shared_ptr< socket_device_nonblocking > 
	socket_receptor::
	next_device_nonblocking()
	{
		// Assert stream_handle is the handle of a connected socket ready for I/O
		// Finally, construct a device from the new socket
		return boost::shared_ptr< socket_device_nonblocking >( new socket_device_nonblocking( next_connection() ) ) ;
	}

	//--------------------------------------------------
	// socket_receptor
	//--------------------------------------------------

	//-------------------------
	/* Constructor
	 */
	socket_emitter::
	socket_emitter( socket_address remote )
		: the_handle()
	{
		// Blocking call to connect()
		int x = connect( the_handle, remote.struct_ptr(), remote.struct_size() ) ;
		if ( x < 0 ) {
			if ( last_error() == EINTR ) {
				throw std::runtime_error( "EINTR not handled yet" ) ;
			}
			// Assert connect failed.
			/// \todo Distinguish between implementation/system failures, e.g. not-a-socket,
			/// and operational failures, e.g. host-not-available
			throw std::runtime_error( "Did not connect" ) ;
		}
	}

	//-------------------------
	/*
	 */
	socket_device_blocking
	socket_emitter::
	device_blocking()
	{
		return socket_device_blocking( the_handle ) ;
	}

	boost::shared_ptr< socket_device_nonblocking >
	socket_emitter::
	device_nonblocking()
	{
		return boost::shared_ptr< socket_device_nonblocking >( new socket_device_nonblocking( the_handle ) ) ;
	}

} // end namespace Net

