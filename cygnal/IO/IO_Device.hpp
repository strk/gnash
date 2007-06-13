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

/**	\file IO_Device.hpp
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
#include "Buffer.hpp"

namespace IO {
	//--------------------------------------------------
	// Device, Source, Sink
	//--------------------------------------------------

	//-------------------------
	/** \class Source
	 */
	class Source
		: public ACT::autonomous_act
	{
	protected:
		/// Protected default constructor only to be invoked by subclasses.
		Source() {}

	public:
		virtual bool known_eof() =0 ;
		virtual bool characters_available( size_t n = 1 ) =0 ;
		virtual IO::contiguous_buffer<> next_segment() =0 ;
		virtual void consume_up_to( char * x ) =0 ;
		virtual void reset() =0 ;
	} ;

	//-------------------------
	/** \class Sink
	 *	\brief Abstract base class of an asynchronous I/O device that acts as a sink.
	 *	A sink is also an action, in this case a write action to the device it represents.
	 *
	 *	The write action
	 */
	class Sink
		: public ACT::autonomous_act
	{
	protected:
		/// Protected default constructor only to be invoked by subclasses.
		Sink() {}

	public:
		/// [in parameter] The buffer that the action will write upon activation.
		virtual void to_write( buffer ) =0 ;

		/// [out parameter]
		virtual size_t n_written() =0 ;
	} ;

	//-------------------------
	// SSource
	//-------------------------
	class SSource 
		: public Source 
	{
	protected:
		virtual ACT::act_state source_run( ACT::wakeup_listener * ) =0 ;
		inline ACT::act_state run( ACT::wakeup_listener * w ) { return source_run( w ) ; }
	} ;

	//-------------------------
	// Source_Adapter
	//-------------------------
	/** \class Source_Adapter
	 *	\brief Converts Source to SSource for initialization of Device
	 */
	template< class S >
	class Source_Adapter
		: public SSource
	{
		S & the_source ;
		inline ACT::act_state source_run( ACT::wakeup_listener * w ) { return the_source.run( w ) ; }
	public:
		Source_Adapter( S & source )
			: the_source( source )
		{}
		inline bool known_eof() { return the_source.known_eof() ; }
		inline bool characters_available( size_t n ) { return the_source.characters_available( n ) ; }
		inline IO::contiguous_buffer<> next_segment() { return the_source.next_segment() ; }
		inline void consume_up_to( char * x ) { return the_source.consume_up_to( x ) ; }
		inline void reset() { return the_source.reset() ; }
	} ;

	//-------------------------
	// SSink
	//-------------------------
	class SSink : public Sink {
	protected:
		virtual ACT::act_state sink_run( ACT::wakeup_listener * ) =0 ;
		inline ACT::act_state run( ACT::wakeup_listener * w ) { return sink_run( w ) ; }
	} ;

	//-------------------------
	// Sink_Adapter
	//-------------------------
	/** \class Sink_Adapter
	 *	\brief Converts Sink to SSink for initialization of Device
	 */
	template< class S >
	class Sink_Adapter
		: public SSink
	{
		S & the_sink ;
		inline ACT::act_state sink_run( ACT::wakeup_listener * w ) { return the_sink.run( w ) ; }
	public:
		Sink_Adapter( S & sink )
			: the_sink( sink )
		{}
		inline void to_write( IO::buffer b ) { the_sink.to_write( b ) ; }
		inline size_t n_written() { return the_sink.n_written() ; }
	} ;

	//-------------------------
	/**	\class Device
	 *	\brief Both a Source and a Sink.
	 */
	class Device
		: public SSource, public SSink
	{
	protected:
		Device( SSource & x, SSink & y )
			: SSource( x ), SSink( y )
		{}
	public:
		Device() {} ;
	} ;

	//-------------------------
	/**	\class Split_Device
	 *	\brief A device composed from a separate Source and a separate Sink.
	 */
	template< class In, class Out >
	class Split_Device
		: public Device
	{
		In the_source ;
		Out the_sink ;
		inline ACT::act_state sink_run( ACT::wakeup_listener * w ) { return the_sink.run( w ) ; }
		inline ACT::act_state source_run( ACT::wakeup_listener * w ) { return the_source.run( w ) ; }
	public:
		Split_Device( In in, Out out )
			: the_source( in ), the_sink( out )
		{}
		inline bool known_eof() { return the_source.known_eof() ; }
		inline bool characters_available( size_t n ) { return the_source.characters_available( n ) ; }
		inline IO::contiguous_buffer<> next_segment() { return the_source.next_segment() ; }
		inline void consume_up_to( char * x ) { return the_source.consume_up_to( x ) ; }
		inline void reset() { return the_source.reset() ; }

		inline void to_write( IO::buffer b ) { the_sink.to_write( b ) ; }
		inline size_t n_written() { return the_sink.n_written() ; }
	} ;

	//-------------------------
	template< class Oldsource, class Action >
	class adapter_for_old_source
		: public Source
	{
		Oldsource & the_source ;
		Action the_action ;

		inline ACT::act_state run( ACT::wakeup_listener * w ) { return the_action.operator()( w ) ; }

		template< class In, class Out > friend class Split_Device ;
	public:
		adapter_for_old_source( Oldsource & source )
			: the_source( source ),
			the_action( Action( & source ) )
		{}

		inline bool known_eof() { return the_source.known_eof() ; }
		inline bool characters_available( size_t n ) { return the_source.characters_available( n ) ; }
		inline IO::contiguous_buffer<> next_segment() { return the_source.next_segment() ; }
		inline void consume_up_to( char * x ) { return the_source.consume_up_to( x ) ; }
		inline void reset() { return the_source.reset() ; }
	} ;

} // end of namespace IO

#endif	// end of inclusion protection