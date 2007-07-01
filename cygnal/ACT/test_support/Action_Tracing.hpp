// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// \file Action_Tracing.hpp

#pragma once
#ifndef __Action_Tracing_hpp__
#define __Action_Tracing_hpp__

#include <string>

namespace ACT {

	//-------------------------
	/** \class execution_trace
	 */
	class execution_trace
	{
		/// Storage for tracking output
		std::string tracking_string ;

	public:
		execution_trace()
			: tracking_string( "" ) {}

		void add( std::string x )
		{
			tracking_string += x ;
		}

		std::string result() const
		{
			return tracking_string ;
		}
	} ;

	//-------------------------
	/**	\class tracking_function
	 *	\brief A function to call once per activation.
	 */
	class tracking_function
	{
	public:
		/// Operation expected to be called once per activation.
		///
		/// It's an activation of \c run, rather than \c operator().
		/// Thus, if an activation is to a completed or bad action, it won't track.
		virtual void operator()( const std::string ) =0 ;
	} ;

	//-------------------------
	/**	\class simple_tracker
	 *	\brief Marks operation with a prefix designated at construction time.
	 */
	class simple_tracker
		: public tracking_function
	{
		/// Prefix for tracking item
		const std::string prefix ;

		/// Trace to which to added tracking event
		execution_trace & trace ;

	public:
		///
		simple_tracker( execution_trace & trace, const std::string prefix )
			: trace( trace ), prefix( prefix )
		{}

		///
		void operator()( const std::string x )
		{
			std::string tracking_string( prefix ) ;
			if ( ! x.empty() ) {
				tracking_string += "(" ;
				tracking_string += x ;
				tracking_string += ")" ;
			}
			trace.add( tracking_string ) ;
		}
	} ;

	//-------------------------
} // end namespace ACT

#endif