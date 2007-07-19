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

/// \file Pause_Demon.hpp
///	\brief The Pause service suspends itself (and its thread) for a number of milliseconds 
///		if the scheduling queue has no high-priority items.

#pragma once
#ifndef __Pause_Demon_hpp__
#define __Pause_Demon_hpp__

#include "ACT.hpp"
#include "Scheduler.hpp"

namespace ACT {
	//-------------------------
	/**	\class Pause_Demon
	 *	\brief A service that does nothing for some number of milliseconds if the scheduler has no tasks pending.
	 *
	 *	This service is only pauses during quiet or quiescent operation of a scheduler.
	 *	The point is to reduce CPU load when nothing is happening.
	 *	This class is not the most efficient for a low-load service, 
	 *		since it consumes some constant fraction of CPU regardless of other load.
	 *	At high load, this service consumes neglible CPU.
	 *
	 *	\par Future
	 *	For low-load and/or utility server applications, this service should be replaced with one that simply blocks.
	 *	Such a service would have to cooperate with all other listening facilities, any of which could unblock it.
	 *
	 *	\par Defects
	 *	- The present implementation is immortal, preventing graceful cooperative shutdown.
	 *		It should be able to terminate itself under at least one condition.
	 *		For example, if no other tasks or services are present, then this one too can end.
	 */
	class Pause_Demon
		: public simple_act
	{
		/// Configuration parameter.
		///
		/// Should eventually transition to a template parameter.
		static const unsigned int ms_to_pause = 20 ;

		/// Action body.
		ACT_State run() ;

		/// Scheduler determines whether pause is necessary or not.
		Scheduler * the_scheduler ;

	public:
		/// 
		Pause_Demon( Scheduler * sc )
			: the_scheduler( sc )
		{}

	} ;

	//-------------------------
} // end namespace ACT

#endif
