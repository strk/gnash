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

/// \file socket_device.cpp

#include "socket_device.hpp"
#include "errno.h"

namespace Net {
	//--------------------------------------------------
	// socket_device
	//--------------------------------------------------

	//-------------------------
	socket_device::
	socket_device( socket_handle the_handle )
		: the_handle( the_handle ),
		is_closed( false ),
		was_selected( false )
	{
	}

	//-------------------------
	/** \par Implementation 
	 *	CURRENT VERSION IS A STUB.
	 */
	bool
	socket_device::
	closed()
	{
		if ( is_closed ) {
			return true ;
		}
		// Assert we did not already know that this socket had been closed.
		// Hence either it's open or we haven't yet determined that it's closed.
		return false ;
	}

	//-------------------------
	std::streamsize 
	socket_device::
	read( char * s, std::streamsize n )
	{
		// The static_cast is necessary for Microsoft Windows,
		//		where the socket type is unsigned and the file descriptor type is signed.
		// The use of recv() with zero flags instead of read() is required for Microsoft windows, 
		//		whose read() does not consider socket handles as valid.
		return ::recv( static_cast< int >( the_handle ), s, n, 0 ) ;
	}

	//-------------------------
	std::streamsize 
	socket_device::
	write( const char * s, std::streamsize n )
	{
		// The static_cast is necessary for Microsoft Windows,
		//		where the socket type is unsigned and the file descriptor type is signed.
		// The use of send() with zero flags instead of write() is required for Microsoft windows, 
		//		whose write() does not consider socket handles as valid.
		return ::send( static_cast< int >( the_handle ), s, n, 0 ) ;
	}

	//--------------------------------------------------
	// socket_device_blocking
	//--------------------------------------------------
	socket_device_blocking::
	socket_device_blocking( socket_handle the_handle )
		: socket_device( the_handle )
	{
		// The correctness of this implementation relies on the fact that sockets created
		// anew or received from accept() arrive with blocking mode turned on by default.
		// Should this not be true, we need to set an option here to force blocking mode.
	} ;

	//-------------------------
	std::streamsize
	socket_device_blocking::
	write( const char * s, std::streamsize n )
	{
		// The base class \c write() does not change the existing blocking mode.
		// By the precondition, the underlying socket will block, so this function will also.
		return socket_device::write( s, n ) ;
	}

	//-------------------------
	std::streamsize 
	socket_device_blocking::
	read( char * s, std::streamsize n ) 
	{
		// The base class \c read() does not change the existing blocking mode.
		// By the precondition, the underlying socket will block, so this function will also.
		return socket_device::read( s, n ) ;
	}

	//--------------------------------------------------
	// socket_device_nonblocking
	//--------------------------------------------------

	//-------------------------
	/** \class read_waiter
	 *	\brief waits for a socket to become ready for reading
	 *
	 */
	class read_waiter {
	public:
		read_waiter() ;
	} ;

 
	//--------------------------------------------------
	// socket_device_nonblocking
	//--------------------------------------------------

	socket_device_nonblocking::
	socket_device_nonblocking( socket_handle the_handle )
		: socket_device( the_handle ),
		raw_input_buffer( new char[ raw_input_buffer_length ] ),
		master( raw_input_buffer.get(), raw_input_buffer_length ),
		the_next_segment( master.begin, 0 )
	{
		// We only need set non-blocking mode on this socket once, but we must set it at least once.
		the_handle.set_option_non_blocking( true ) ;
	}

	boost::shared_ptr< IO::read_action_base >
	socket_device_nonblocking::
	new_read_action()
	{
		return boost::shared_ptr< IO::read_action_base >( new read_action( * this ) ) ;
	}

	//-------------------------
	void 
	socket_device_nonblocking::
	consume_up_to( char * )
	{
		throw std::exception( "socket_device_nonblocking::consume_up_to() not implemented" ) ;
	}

	bool
	socket_device_nonblocking::
	known_eof()
	{
		return is_closed ;
	}

	bool
	socket_device_nonblocking::
	characters_available( size_t n )
	{
		throw std::exception( "socket_device_nonblocking::characters_available() not implemented" ) ;
	}

	IO::contiguous_buffer<>
	socket_device_nonblocking::
	next_segment()
	{
		IO::contiguous_buffer<> return_value( the_next_segment ) ;

		the_next_segment.begin = return_value.end() ;
		the_next_segment.length = 0 ;

		return return_value ;
	}

	//--------------------------------------------------
	// read_action
	//--------------------------------------------------

	//-------------------------
	read_action::
	read_action( socket_device_nonblocking & the_socket )
		: the_socket( the_socket )
	{}

	//-------------------------
	ACT::act_state 
	read_action::
	run( ACT::wakeup_listener * waken )
	{
		char * begin = std::min( the_socket.the_next_segment.end(), the_socket.master.end() ) ;
		char * end = std::max( begin, the_socket.master.end() ) ;
		// This read() call is that from the device, not a raw socket read.
		std::streamsize x = the_socket.read( begin, end - begin ) ;
		if ( x > 0 ) {
			// Assert read found something in the stream, so we're done.
			the_socket.the_next_segment.length += x ;
			set_completed() ;
		} else if ( x == 0 ) {
			// Assert read found no characters
			// Assert stream read successfully (its return != -1) and read returned zero.
			// Assert stream is now closed.
			the_socket.is_closed = true ;
			set_completed() ;
		} else {
			// Assert x < 0, thus read() had an error.
			int error = last_error() ;
			if ( error == EWOULDBLOCK ) {
				// Assert we read no characters and we don't know the stream has closed.
				// The scheduler may now try the operation later to attempt completion.
				// We now set up a wakeup.

				/// \todo Implement the wake-up call.
			} else if ( error == EINTR ) {
				// Assert the read() call was interrupted.
				// Rather than enter a loop here, we let the scheduler retry.
				if ( waken != 0 ) {
					( * waken )() ;
				}
			} else {
				// Assert the socket had a non-recoverable error
				set_bad() ;
			}
		}
		// Restore the invariant for was_selected.
		the_socket.was_selected = false ;
		return internal_state() ;
	}


	//--------------------------------------------------
	// write_action
	//--------------------------------------------------

	//-------------------------
	write_action::
	write_action(
		socket_device_nonblocking & the_socket, 
		const char * s, 
		std::streamsize n
	) 
		: IO::write_action_base( s, n ),
		the_socket( the_socket )
	{
		if ( n == 0 ) {
			// Nothing to write, so we can go straight into Completed state
			set_completed() ;
		}
	} ;

	//-------------------------
	ACT::act_state 
	write_action::
	run( ACT::wakeup_listener * waken )
	{
		// This write() call is that from the device; it's not a write to a raw socket.
		int n_just_written = the_socket.write( oldstyle_buffer, oldstyle_buffer_size ) ;
		if ( n_just_written > 0 ) {
			number_written += n_just_written ;
			if ( n_just_written == oldstyle_buffer_size ) {
				// Assert we've written all we can
				set_completed() ;
			} else {
				// Assert there's still something left to write
				oldstyle_buffer += n_just_written ;
				oldstyle_buffer_size -= n_just_written ;

				/// \todo Register a notification when write is ready to go.
			}
		} else if ( n_just_written == 0 ) {
			// Assert buffer_size > 0
			// But we just wrote zero bytes without error
			// What's going on here?
			// An exception will certainly get someone's attention.
			throw std::exception() ;
		} else {
			// Assert n_just_written < 0, thus write() had an error.
			int error = last_error() ;
			if ( error == EWOULDBLOCK ) {
				// Assert we wrote no characters and we don't know the stream has closed.
				// The scheduler may now try the operation later to attempt completion.
				// We now set up a wakeup.

				/// \todo Implement the wake-up call.
			} else if ( errno == EINTR ) {
				// Assert the read() call was interrupted.
				// Rather than enter a loop here, we let the scheduler retry.
				if ( waken != 0 ) {
					( * waken )() ;
				}
			} else {
				// Assert the socket had a non-recoverable error
				set_bad() ;
			}
		}
		return internal_state() ;
	}

}
