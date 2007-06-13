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

/// \file IO_Generator.hpp

#pragma once
#ifndef __IO_Generator_hpp__
#define __IO_Generator_hpp__

#include "ACT/Service.hpp"
#include "IO_Device.hpp"

#include <boost/shared_ptr.hpp>
using boost::shared_ptr ;

namespace IO {
	//-------------------------
	/**	\class Behavior
	 *	\brief 
	 */
	class Behavior 
		: public ACT::autonomous_act
	{
	protected:
		/// Protected default constructor should be called only by subclasses.
		Behavior() {} ;
	public:
	} ;

	//-------------------------
	/**	\type Behavior_Factory
	 *	A function that returns a protocol behavior on input Device.
	 *	As a rule such a function will be a static function defined in the class it's a factory for.
	 */
	typedef shared_ptr< ACT::autonomous_act > Behavior_Factory( shared_ptr< Device > x ) ;

	//-------------------------
	/**	\class Device_Generator
	 *	\brief A generator is an ACT whose result is a Device.
	 *
	 *	The prototypical Generator is a bound listening socket.
	 *	A successful call to accept(), yielding a new connection, triggers completion of the action.
	 *
	 *	A generator completes when it has a device ready to be retrieved as a result.
	 *	If a generator has an internal list of devices (explicitly or implicitly), then completion makes only one of them available.
	 *	Subsequent devices will be made available after a reset and then another activation.
	 */
	class Device_Generator
		: public ACT::autonomous_act
	{
	public:

		///
		virtual shared_ptr< IO::Device > result() =0 ;

		void reset() {} ;
	} ;

	//-------------------------
	/**	\class Device_Adapter
	 */
	class Device_Adapter
		: public ACT::Generator
	{
		/// We use shared_ptr here
		Device_Generator & the_generator ;

		///
		Behavior_Factory & the_behavior_factory ;

		shared_ptr< ACT::autonomous_act > the_result ;

	public:
		///
		Device_Adapter( Device_Generator & x, Behavior_Factory & y ) ;

		///
		ACT::act_state run( ACT::wakeup_listener * w ) ;

		///
		ACT::act result() ;

	} ;


} // end namespace IO

#endif