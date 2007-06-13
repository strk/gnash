// 
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of GNU Cygnal.
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
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

/// \file Server.hpp

#pragma once
#ifndef __Service_hpp__
#define __Service_hpp__

#include "ACT.hpp"
#include "Scheduler.hpp"

namespace ACT {
	class Generator
		: public ACT::autonomous_act
	{
	public:
		///
		virtual act result() =0 ;

		void reset() {} ;
	} ;

	//-------------------------
	/**	\class Service
	 *	\brief A service is a persistent action whose result to create new actions and to schedule them for execution.
	 *
	 *	NOTE: This class should be "IO_Service", and have a primitive notion of service use ACT::Generator (to be defined) 
	 *	rather than a device generator.  It might also prove useful to define an ACT trait that means "generates actions".
	 *	This would be a way of integrating user interface events, for example.
	 *
	 *	This generic base class has no result accessor, since services run primarily under the scheduler.
	 *	The scheduler, which deals with a multiplicity of action types, has no way of dealing with specific results.
	 *	
	 *	A service, in addition, is "always working".
	 *	It only completes when it needs to shut down, either from an internal failure or by external command.
	 */
	class Service 
		: public autonomous_act
	{
		Basic_Scheduler & our_scheduler ;

		Generator & the_generator ;
	public:
		/// 
		Service( Generator & x, Basic_Scheduler & z ) ;

		///
		act_state run( wakeup_listener * ) ;
	} ;

	//-------------------------
} // end namespace ACT

#endif