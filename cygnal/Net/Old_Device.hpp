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

/**	\file Old_Device.hpp
 *	\brief Basic abstractions for handling ACT-based I/O
 *
 *	\par Design Notes
 *	The principle of ACT implementation is that each ACT carry its entire interrupted-execution state within its own instance.
 *	In order to write a generic filter as an ACT it's necessary to carry the state of its I/O device with the filter ACT.
 *	There are two basic form of attaining the requisite generic behavior: polymorphic subclassing and template instantiation.
 *	Polymorphism has a more compact representation in code, but entails a small performance penalty.
 *	It's easier to get up and running.
 *	Hence the current implementation uses polymorphism.
 *
 *	Now the ultimate I/O source for most network applications is _only_ the network.
 *	Thus eventually it may become desirable to refactor the present system into a template-based one.
 *	This doesn't abnegate the possibility of also using polymorphism, since a polymorphic base could be a template parameter.
 *	Indeed, this is the first step in conversion to templates.
 *
 *	A generic filter has variable-sized internal state, which depends upon its I/O source.
 *	Memory allocation thus becomes an issue.
 *	There are _two_ aspects of memory allocation: one for the sources/sinks and the other for read/write actions.
 *	The present implementation uses the following conventions:
 *	- Concrete implementations of read/write actions derive from IO::read_action_base and IO::write_action_base.
 *	- Concrete implementations of sources and sinks derive from IO::source_base and IO::sink_base.
 *		These have virtual factories that provide IO::read_action and IO::write_action.
 *	- Facade classes ACT::act wrap read and write actions.
 *	- Facade classes IO::Source, IO::Sink, and IO::Device are wrappers around smart pointers to sources and sinks.
 *	- Concrete sources/sinks allocate memory, within their factories, for their read/write actions.
 *
 *	\par Recommendations to Users of Concrete I/O classes.
 *	Here's a skeleton of the usage sequence
 *	- Allocate a new instance of the concrete I/O class you wish to use.
 *		You will, in general, want access to the concrete class directly to avail yourself of its non-generic affordances.
 *	- Construct an instance of IO::Device, IO::Source, or IO::Sink, as appropriate, with this pointer.
 *	- Use this device instance as a parameter to a filter constructor.
 *	- Use your filter.
 *
 *	\par Recommendations to Implementors of Concrete I/O classes.
 *	- First implement your specific I/O functionality, split into two or three classes. 
 *		One class represents the device itself, the other one or two are the read and/or write operations as functional classes.
 *		The way that you construct your classes is unconstrained.
 *		Whatever you choose, you'll be writing factory methods for read/write operations, so make sure that you _can_ write them. 
 *	- Refactor your read class and/or write class to derive from IO::read_action_base and IO::write_action_base.
 *		You certainly can do this at the outset, given that there's only one virtual function.
 *	- Refactor your device class to derive from IO::source_base and/or IO::sink_base, as appropriate.
 *		This entails writing read/write operation factories in this step, which isn't necessary beforehand.
 *		You can, of course, do this from the beginning.
 *		You may find it easier to deal with your concepts directly, rather than incurring this framework apparatus to start.
 *	- Write unit tests with the base classes only, avoiding the facades, to verify basic operations.
 *	- Write unit tests with the facades, using IO::Null_Filter to ensure that your device works correctly within the framework.
 *
 */

#pragma once
#ifndef __IO_Device_hpp___
#define __IO_Device_hpp___

#include "ACT/ACT.hpp"
#include "IO/Buffer.hpp"

namespace IO {

	//--------------------------------------------------
	// Read and Write operations
	//--------------------------------------------------

	//-------------------------
	/** \class read_action_base
	 *	\brief Base class for asynchronous read actions.
	 *
	 *	A read action may possess an internal buffer of data it has read from its source.
	 *	As long as a read action is in existence, it owns any data read.
	 *	When a read action destructs, it should give any unconsumed data back to its source.
	 *	By default, all data is considered consumed unless otherwise specified.
	 *
	 *	As an abbreviation to the destroy-then-construct-another cycle, this class provides reset().
	 *	This method has a similar effect as destroying one read action and constructing another from the same source.
	 *	This difference is that reset() does not invalidate its internal buffers, so buffer iterators remain valid.
	 *	Furthermore, the internal buffer may expand itself silently.
	 */
	class read_action_base
		: public ACT::autonomous_act
	{
 	protected:
		/// Hook for template instantiation
		typedef char char_type ;

	public:
		///
		read_action_base() ;

		/// Reset without destroying.
		virtual void reset() =0 ;
	} ;

	//-------------------------
	/** \class write_action_base
	 *	\brief Base class for asynchronous read actions.
	 */
	class write_action_base
		: public ACT::autonomous_act
	{
	protected:
		const char * oldstyle_buffer ;
		unsigned int oldstyle_buffer_size ;

		/// Number of character written so far to output device
		size_t number_written ;

	public:
		// TEMPORARY
		// old-style constructor
		write_action_base( const char *, unsigned int ) ;

		/// [result] Number of characters written as a result of most recent completed activation.
		size_t n_written() { return number_written ; }
	} ;

	//-------------------------
	/** \class source_base
	 *	\brief Abstract base classes for I/O sources.
	 *
	 *	\par Future
	 *	- An action need not use the data it reads from a source.
	 *		Implement a give_back() method may give unused input back to the source.
	 */
	class source_base
	{
	public:
		/// Factory method for read actions
		virtual boost::shared_ptr< read_action_base > new_read_action() =0 ;

		/// Because non-blocking I/O may not know when it's reached EOF, we have an indicative name.
		/// A source that returns false means "not known to be at EOF"; it may later (correctly) read zero characters at EOF.
		virtual bool known_eof() =0 ;

		/** \brief Are there at least 'n' characters in the next segment?
		 *
		 *	A calling function needs to know when to call the read action in order to replenish an in-memory buffer.
		 *	This function alleviates a need that a caller might have to track buffering state in parallel with the source.
		 */
		virtual bool characters_available( size_t n = 1 ) =0 ;

		/**	\brief The next available segment for a caller to use
		 */
		virtual IO::contiguous_buffer<> next_segment() =0 ;

		///
		virtual void consume_up_to( char * ) =0 ;

		/// The NEW reset
		/// To be made pure-virtual later.
		virtual void reset() { throw std::exception( "new reset() not implemented in child class" ) ; }

	} ;

} // end of namespace IO

#endif	// end of inclusion protection