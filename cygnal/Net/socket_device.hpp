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

/// \file socket_device.hpp

#pragma once
#ifndef __SOCKET_DEVICE_HPP__
#define __SOCKET_DEVICE_HPP__

#include "ACT/ACT.hpp"
//#include "IO/IO_Device.hpp"
#include "Old_Device.hpp"
#include "Net.hpp"

#include <boost/iostreams/traits.hpp>
#include <boost/scoped_array.hpp>

namespace Net {
	//-------------------------
	/** \class socket_device
	 *	\brief A bidirectional stream associated with a connected socket.
	 *	Base class for the blocking and non-blocking versions of the device.
	 *
	 *	The constructors for this class are private and only available to friends.
	 *	Friend instances establish connections first then convert them into connections.
	 *	Thus do friend classes act as factory classes for this one.
	 */
	class socket_device {
	protected:
		/// Constructor is protected to enable subclass-only construction
		socket_device( socket_handle the_handle ) ;

		/// Some may view this class as a facade surrounding a socket handle.
		socket_handle the_handle ;

		/**	Platforms do not uniformly have a call that verifies that a socket has closed.
		 *	\post return is true implies we're certain that socket is closed.
		 *	\note Postcondition does not mean that the socket has actually closed, only that we're sure that it has.
		 *	The generic situation is that the TCP connection underlying the socket has finished, 
		 *		but that we haven't found out about it yet.
		 */
		bool closed() ;

		/// Memo for closed().
		/// Socket closure is irreversible, so this flag need be set only upon the first
		///		determination of closure.
		bool is_closed ;

		/** Selection status flag used to determined whether a handle has closed.
		 *	A handle that selected true for reading that reads zero bytes has closed.
		 *	\invariant was_selected is true between (1) the return from a select call that
		 *	indicates the socket is ready to read and (2) the next read() call.
		 */
		bool was_selected ;

		/// Behavior is blocking or nonblocking depending on how socket is configured.
		std::streamsize write( const char * s, std::streamsize n ) ;

		/// Behavior is blocking or nonblocking depending on how socket is configured.
		std::streamsize read( char * s, std::streamsize n ) ;

	public:
		/// The character type, after the manner of standard string library definitions.
		/// Should it become expedient to support multiple stream types, 
		///		this definition can become a template parameter.
		typedef char Ch ;
	} ;

	//-------------------------
	// Forward declarations of factory-containing classes.
	class socket_receptor ;
	class socket_emitter ;

	//-------------------------
	/** \class socket_device_blocking
	 *	\brief A socket device with blocking read and write calls.
	 */
	class socket_device_blocking
		: private socket_device
	{
		friend socket_device_blocking socket_receptor::next_device_blocking() ;
		friend socket_device_blocking socket_emitter::device_blocking() ;

		/// Ordinary constructor converts an existing handle and enables its use for I/O.
		socket_device_blocking( socket_handle the_handle ) ;

	public:
		///	Required to model Boost.Iostreams.Sink.
		///	\pre
		///	- underlying socket is in blocking mode.
		std::streamsize write( const char * s, std::streamsize n ) ;

		///	Required to model Boost.Iostreams.Source.
		///	\pre
		///	- underlying socket is in blocking mode.
		std::streamsize read( char * s, std::streamsize n ) ;

		/// Required to model Boost.Iostreams.BidirectionalDevice
		typedef Ch char_type ;

		/// Trait definition to model Boost.Iostreams.BidirectionalDevice
		typedef boost::iostreams::bidirectional_device_tag category ;
	} ;

	//-------------------------
	/** \class socket_device_nonblocking
	 *	\brief A socket device with non-blocking read and write calls.
	 */
	class socket_device_nonblocking
		: private socket_device,
		public IO::source_base
	{
		/// read_action is friend in order to get access to socket_device.read()
		friend class read_action ;

		/// write_action is friend in order to get access to socket_device.write()
		friend class write_action ;

		/// socket_receptor::next_device_nonblocking() is a factory function and requires access to constructor
		friend boost::shared_ptr< socket_device_nonblocking > socket_receptor::next_device_nonblocking() ;

		/// socket_emitter::device_nonblocking() is a factory function and requires access to constructor
		friend boost::shared_ptr< socket_device_nonblocking > socket_emitter::device_nonblocking() ;

		/// Private to enforce factory-only construction
		socket_device_nonblocking( socket_handle the_handle ) ;

		/// [read buffer] Size of allocated array.
		static const int raw_input_buffer_length = 16 * 1024 - 64 ;

		/** \brief A source manages its own stream buffer.
		 *
		 *	This buffer is the primary receptacle for data coming off the socket.
		 *	The results of individual read actions are concatenated in here.
		 *
		 *	\par Known Defects
		 *	- Small Maximum Input Length.
		 *		The I/O system is premature; it's not possible to release input yet.
		 *		More than this buffer size can't be kept within this class all at once.
		 *		Too many read() calls will fill up this internal buffer.
		 *		This will cause the read action to go bad when the source is still fine.
		 */
		/// [read buffer] Our lone allocated array.
		boost::scoped_array< char > raw_input_buffer ;

		/// The master buffer, containing all characters read but not yet consumed.
		IO::contiguous_buffer<> master ;

		/// what's already been read
		IO::contiguous_buffer<> the_next_segment ;

		/// next point from which to deliver to caller
		char * first_undelivered_character ;

	public:
		/// Read action factory
		boost::shared_ptr< IO::read_action_base > new_read_action() ;

		/// 
		bool known_eof() ;

		bool characters_available( size_t n ) ;

		IO::contiguous_buffer<> next_segment() ;

		/// 
		void consume_up_to( char * ) ;

	} ;

	//-------------------------
	/** \class read_action
	 *	\brief Retrieve something from the socket or else give up because it has closed.
	 *
	 *	\invariant
	 *	the_state == Completed iff n_read > 0 or socket has closed
	 *
	 *	\par WARNING
	 *	This implementation is awful.
	 *	It's awful because it's a quick and dirty way of simulating asynchronous I/O
	 *		out of the standard socket library, which doesn't support it directly.
	 *	And on top of that, it's not even particularly efficient at that.
	 *	It does, however, have two principal merits:
	 *	- It works.  Which is to say, that it really does simulate asynchronous I/O.
	 *	- It's expedient.  Putting in better asynchrony requires much more effort,
	 *		not appropriate at an initial stage of development.
	 *
	 *	There are two directions for this class to take.
	 *	Both should be taken.
	 *	-# A better version of this basic approach using only the standard socket library.
	 *	-# A new version for each API for performing asynchronous I/O.
	 *		There are several such available:
	 *		- AIO.
	 *		- kqueue().
	 *		- epoll().
	 *		- /dev/poll.
	 *		- whatever Microsoft Windows does.  
	 *		- Whatever the other way is that Microsoft Windows does it.
	 *
	 *	\par Implementation
	 *	The basic approach twofold:
	 *	- Use a non-blocking read() call, which of course may return nothing.
	 *	- If nothing comes back, create a thread to select() on the socket and wait until it returns.
	 *
	 *	The essential inefficiency of this implementation is 
	 *		that a new thread is constructed for each instance of read_something_action().
	 *	The better way of doing this is to use a singleton waiter thread,
	 *		which would simulataneous select() on all waiting sockets.
	 */
	class read_action
		: public IO::read_action_base
	{
		/// The underlying socket device
		socket_device_nonblocking & the_socket ;

	public:
		//---------------
		/**
		 */
		ACT::act_state run( ACT::wakeup_listener * waken ) ;

		//---------------
		/** \brief Ordinary constructor 
		 *	\pre
		 *	- socket is opened in non-blocking mode
		 */
		read_action( socket_device_nonblocking & the_socket ) ;

		/// Reset the action
		void reset() {}

	} ;

	//-------------------------
	/**	\class write_action
	 *	\brief Write a sequence to the socket or give up because the socket has closed.
	 *
	 *	\invariant
	 *	- the_state == Completed iff n_written == in.n or socket has closed or in.n == 0
	 *	- in.n == 0 implies the_state == Completed
	 *
	 */
	class write_action
		: public IO::write_action_base
	{
		// The underlying socket device
		socket_device_nonblocking & the_socket ;

	public:
		/// Action operator actually does the write.
		ACT::act_state run( ACT::wakeup_listener * waken ) ;

		/// Ordinary constructor
		write_action(
			socket_device_nonblocking & the_socket, 
			const char * s, 
			std::streamsize n
		) ;
	} ;

}

#endif // end inclusion protection