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

/**	\file Null_Device.hpp
 *	\brief 
 */

#pragma once
#ifndef __Null_Device_hpp___
#define __Null_Device_hpp___

#include "../IO_Device.hpp"

namespace IO {

	//-------------------------
	/**	\class Null_Source
	 *	\brief An eternal source of empty buffers.
	 */
	class Null_Source
		: public Source
	{
		friend class Null_Device ;
		template< class S > friend class Source_Adapter ;

		ACT::ACT_State run( ACT::wakeup_listener * ) ;

	public:
		/// Only need a default constructor for a source that's always the same.
		Null_Source() ;

		void reset() ;

		bool known_eof() ;
		bool characters_available( size_t n = 1 ) ;
		IO::contiguous_buffer<> next_segment() ;
		void consume_up_to( char * ) ;
	} ;

	//-------------------------
	/** \class Null_Sink
	 *	\brief A sink which is also the bit bucket.
	 */
	class Null_Sink
		: public Sink
	{
		friend class Null_Device ;
		template< class S > friend class Sink_Adapter ;
		template< class In, class Out > friend class Split_Device ;

		/// The write action.
		ACT::ACT_State run( ACT::wakeup_listener * ) { return set_completed() ; }

		/// [out parameter value]
		size_t out_n_written ;

	public:
		///
		Null_Sink() {} ;

		///
		inline size_t n_written() { return out_n_written ; }

		/// A null sink can instantly write terabytes to the bit bucket.  Instantly, I say!
		void to_write( buffer b ) { out_n_written = b.size() ; }
	} ;

	//-------------------------
	/**	\class Null_Device
	 *	\brief A device that never has input and write all output to the bit bucket.
	 */
	class Null_Device
		: public Device
	{
		Null_Sink the_sink ;
		Null_Source the_source ;
		
		inline ACT::ACT_State source_run( ACT::wakeup_listener * w ) { return Device::SSource::set_state( the_source.run( w ) ) ; }
		inline ACT::ACT_State sink_run( ACT::wakeup_listener * w ) { return Device::SSink::set_state( the_sink.run( w ) ) ; }

	public:
		///
		Null_Device() ;

		inline bool known_eof() { return the_source.known_eof() ; }
		inline bool characters_available( size_t n ) { return the_source.characters_available( n ) ; }
		inline IO::contiguous_buffer<> next_segment() { return the_source.next_segment() ; }
		inline void consume_up_to( char * x ) { return the_source.consume_up_to( x ) ; }
		inline void reset() { return the_source.reset() ; }

		inline void to_write( IO::buffer b ) { the_sink.to_write( b ) ; }
		inline size_t n_written() { return the_sink.n_written() ; }
	} ;

} // end of namespace IO

#endif	// end of inclusion protection