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

/// \file Pause_Service.hpp
///	\brief The Pause service suspends itself (and its thread) for a number of milliseconds 
///		if the scheduling queue has no high-priority items.

#pragma once
#ifndef __Pause_Service_hpp__
#define __Pause_Service_hpp__

#include "ACT.hpp"
#include "Scheduler.hpp"

namespace ACT {
	//-------------------------
	/**	\class Pause_Service
	 */
	class Pause_Service
		: public simple_act
	{
		/// Configuration parameter.
		///
		/// Should eventually transition to a template parameter.
		static const unsigned int ms_to_pause = 20 ;

		/// Action body.
		act_state run() ;

		/// Scheduler determines whether pause is necessary or not.
		Basic_Scheduler * the_scheduler ;

	public:
		/// 
		Pause_Service( Basic_Scheduler * sc )
			: the_scheduler( sc )
		{}

	} ;

	//-------------------------
} // end namespace ACT

#endif