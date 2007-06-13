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

/// \file Net.hpp

#pragma once
#ifndef __Net_hpp__
#define __Net_hpp__

#ifdef HAVE_CONFIG_H
#	include "config.h"
#endif

#include <string>
#include <boost/shared_ptr.hpp>

#ifndef HAVE_WINSOCK_H
#	include <netinet/in.h>
#	include <arpa/inet.h>
	typedef int socket_t ;
#else
#	include <winsock2.h>
#	include <fcntl.h>
#	include <sys/stat.h>
#	include <io.h>
	// If max and min are defined as macros, they'll interfere with numeric_limits<T>::min and max
#	ifdef max
#		undef max
#	endif
#	ifdef min
#		undef min
#	endif

	/* Winsock doesn't define in_addr_t, but it does define some supporting conversion functions.
	 * Why they don't use in_addr_t, even internally, in winsock2.h defies me.
	 */
	typedef unsigned long in_addr_t ;

	typedef u_short in_port_t ;
	typedef SOCKET socket_t ;
#	define EWOULDBLOCK WSAEWOULDBLOCK
#endif

namespace Net {
	//-------------------------
	// Forward declarations
	class socket_receptor ;
	class socket_emitter ;
	class socket_device_blocking ;
	class socket_device_nonblocking ;

	//-------------------------
	/// Abstraction of 'errno', necessary for Microsoft Windows.
	int last_error() ;
		
	//-------------------------
	/**	\class interface_address
	 *	\brief Facade for \c in_addr_t from netinet/in.h
	 *
	 *	We anticipate IPv6 support by encapsulating a network address within a facade.
	 *	Right now, however, all we support is IPv4.
	 *
	 *	We deal with network vs. host byte order issues here in this class,
	 *		by putting them into the conversion operators.
	 */
	class interface_address {
		in_addr_t the_address ;
	public:
		/// Constructor
		interface_address( in_addr_t x )
			: the_address( x )
		{} ;

		/// Conversion operator
		operator in_addr_t() const { return the_address ; }
	} ;

	//-------------------------
	// Constant addresses

	/** \var anylocal
	 *	\brief Pseudo-address for any local interface.
	 */
	extern const interface_address anylocal ;

	/**	\var loopback
	 *	\brief Predefined address for the localhost (127.0.0.1) interface
	 */
	extern const interface_address loopback ;
	//-------------------------
	/** \class port_address
	 *	\brief Facade for \c in_port_t, which isn't a standard definition
	 *
	 *	The main purpose of this class is to provide an encapsulation of byte order conversions.
	 */
	class port_address {
		in_port_t the_port ;
	public:
		/// Ordinary constructor.
		port_address( in_port_t x )
			: the_port( x )
		{} ;

		/// Conversion operator
		operator in_port_t() const { return the_port ; }
	} ;

	//-------------------------
	/**	\class socket_address
	 *	\brief Facade for \c sockaddr_in from netinet/in.h
	 */
	class socket_address {
		/// Classes that call bind(), connect(), etc. need to get the underlying sockaddr_in structure.
		friend socket_receptor ;
		friend socket_emitter ;

		/// The data which this facade surrounds.
		struct sockaddr_in the_socket ;

		/// To support more than one address family, make this function virtual
		struct sockaddr * struct_ptr() {
			return reinterpret_cast< struct sockaddr * > ( & the_socket ) ; 
		}

		/// A constant value when implementation has a single address family
		int struct_size() const { return sizeof( struct sockaddr_in ) ; }

	public:
		socket_address( struct sockaddr_in x )
			: the_socket( x ) {} ;
		socket_address( interface_address x, port_address y ) ;
	} ;

	//-------------------------
	/** \class net_use_guard
	 *	\brief A guard class surrounding any use of OS and/or library network resources.
	 *	This class is primarily present for Windows; on most *x platforms this is a no-op.
	 *
	 *	Initialization happens in constructor; de-init in destructor.
	 *	To use this class, simply declare a local variable.
	 *	The C++ object construction discipline will handle everything, including exception safety.
	 */
#ifdef HAVE_WINSOCK_H
	// If we must use winsock, then we must use a winsock-specific guard class
	class net_use_guard {
		/// The winsock documentation says that it will keep an internal counter to 
		/// ensure that resources are released only after the last paired call.
		/// Do _you_ believe this will work reliably?  On windows?
		static unsigned int call_count ;
	public:
		/// Constructor initializes windows networking subsystem for this program
		net_use_guard() ;
		/// Destructor deinitializes networking subsystem
		~net_use_guard() ;
	} ;
#else
	// If we do not require initialization for network use, this class need not do anything.
	class net_use_guard {} ;
#endif

	//--------------------------------------------------
	class body_of_socket_handle ;

	//--------------------------------------------------
	/** \class socket_handle
	 *	\brief Facade class for a socket handle as the OS sees it.
	 *	This class is a socket as an identifier with specific characteristics.
	 *	It doesn't perform reading or writing.
	 *	For that, use one of the other socket classes.
	 *
	 *	By way of reference to C-style sockets programming, 
	 *		this class abstracts \c socket, \c close, and \c setsockopt.
	 *	It provides exception safety for these calls.
	 *	It converts an error from a \c socket call in the constructor into an exception
	 *		in order to support the RAII convention.
	 *	The resource is the socket handle itself.
	 *
	 *	\inv Instance holds an allocated socket.
	 *
	 *	\par Limitations
	 *	- Class opens sockets in default streaming protocol (meaning TCP) only.
	 *	- There's no support for any other protocol, such as UDP.
	 *		(Adding support should be by template, and not by parameter, so as to allow compile-time checks.)
	 *	- Current implementation doesn't forward to subclass for polymorphic behavior.
	 *		This isn't necessary as yet because we're only doing IPv4 as yet.
	 *	- Will need a custom allocator for highest levels of performance.  Probably should
	 *		allocate from a fixed pool of sufficient size, say, 10K, that matches the load 
	 *		capacity of the server on which it's deployed.
	 */
	class socket_handle {
		/// Use of wrapper cleans up option setters
		void set_option( int option, void * param, int option_length ) ;

		/// Facility for exception reporting
		void throw_exception( std::string message ) ;

		/// Pointer to body
		boost::shared_ptr< body_of_socket_handle > the_body ;

		/// Private accessor function obtains underlying socket handle from the body.
		socket_t the_handle() const ;

	public:
		/// Default constructor allocates a new socket handle.
		socket_handle() ;

		/// Parametric constructor wraps up a socket handle allocated indirectly,
		///		for example, the socket handle returned by accept().
		///	This class then accepts responsibility for closing the handle.
		explicit socket_handle( socket_t handle ) ;

		/// Rather than a getter function, we provide a conversion operator.
		operator socket_t() const { return the_handle() ; }

		/// Set the option to reuse the address if TIME_WAIT connections are still present
		void set_option_reuse_address( bool reuse ) ;

		/// Set the option to block or not block on I/O
		///
		/// On server sockets that await incoming connections, 
		///		the call that's affected is accept(), which isn't exactly I/O.
		void set_option_non_blocking( bool non_blocking ) ;

	} ;

	//--------------------------------------------------
	/**	\class body_of_socket_handle
	 *	\brief The body of the socket_handle class.
	 *		Confusingly, this class holds a socket handle received from the OS.
	 *
	 *	Class is designed only to be referenced through a shared_ptr.
	 *	If not, close() will be called multiple times and throw an exception in some destructor.
	 */
	class body_of_socket_handle {
		/// Accessor function in facade class may obtain the handle.
		friend class socket_handle ;

		/// Sentry object is only referenced in constructor and can otherwise be ignored.
		net_use_guard please_move_along ;

		/// In many ways this class is a wrapper around a handle value.
		socket_t the_handle ;

		/// Ordinary constructor is private, accessed only by friend
		body_of_socket_handle() ;

		/// Parametric constructor is private, accessed only by friend
		explicit body_of_socket_handle( socket_t ) ;

		/// Class-specific exception reporting
		void throw_exception( std::string message ) ;

	public:
		/// Destructor closes file descriptor
		~body_of_socket_handle() ;

	} ;

	//-------------------------
	/** \class socket_receptor
	 *	\brief A bound socket that receives incoming TCP connections.
	 *
	 *	By way of reference to C-style socket programming, 
	 *		this class abstracts \c bind and \c listen in its constructor
	 *		and \c select and \c accept in its stream factory.
	 *	The failure of a bind call in the constructor converts into an exception
	 *		in support of the RAII convention.
	 *	The managed resource here is a port-binding on the interface in question.
	 *
	 *	\invariant 
	 *	- Instance is bound to a socket specified by \c requested.
	 *	- Underlying socket is listening for incoming connections.
	 *	- this.socket_handle > 0.
	 *
	 *	\par Design Notes
	 *	This class should not have a default constructor.
	 *	The invariant requires a bound socket, and no sensible default socket address exists.
	 *	\par
	 *	We split waiting for connections and accepting them in order to isolate system calls
	 *	and to allow different tactics for each.
	 */
	class socket_receptor {
		/// The usual handle for a socket.
		socket_handle the_handle ;

		/// Class-specific exception reporting
		void throw_exception( std::string message ) ;

		/// Trait specifying the length of listening queue holding pending connections
		static const int queue_length = 5 ;

		/// Separate initialization allows multiple constructors.
		/// CAUTION: Call only as a separate constructor body!
		void init( socket_address ) ;

		/// Wait for a new connection on this socket and block until one arrives.
		/// Return value indicates whether a connection is available.
		/// A connection might not be available if the server is shutting down.
		///
		/// This function uses a \c select call as its waiting tactic.
		/// Apropos possible generalizations, other waiting tactics are possible.
		bool wait_for_connection() ;

		/// Internal wrapper around accept().
		/// This function blocks.
		///
		/// \par OK, it's weird that a blocking function can later generate
		/// a non-blocking action.  If it bothers you, don't do that.
		socket_handle next_connection() ;

	public:
		/// Ordinary constructor binds to the requested socket
		socket_receptor( socket_address ) ;

		/// Convenience constructor binds to any local address with specified port.
		socket_receptor( in_port_t x ) ;

		/// Factory method for creating I/O devices from incoming connections.
		/// This method blocks until a stream is established.
		socket_device_blocking next_device_blocking() ;
		boost::shared_ptr< socket_device_nonblocking > next_device_nonblocking() ;
	} ;

	//-------------------------
	/** \class socket_emitter
	 *	\brief A new socket that initiates an outgoing TCP connection.
	 *
	 *	This construction of this object blocks during initiation of a TCP session.
	 */
	class socket_emitter {
		/// The usual handle for a socket.
		socket_handle the_handle ;

	public:
		/// Ordinary constructor allocates an ephemeral socket address.
		socket_emitter( socket_address remote ) ;

		/// Factory method for creating I/O devices from a constructed outgoing connection.
		socket_device_blocking device_blocking() ;
		boost::shared_ptr< socket_device_nonblocking > device_nonblocking() ;
	} ;

} // end of namespace Net


#endif
