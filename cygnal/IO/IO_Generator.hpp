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
	 *	\brief A behavior is a task whose constructor parameter is a Device.
	 *		When the Device is an internet socket, this task acts as a protocol handler.
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
	/**	\class Old_Device_Generator
	 *	\brief Legacy generator.  On its way out.
	 *
	 */
	class Old_Device_Generator
		: public ACT::autonomous_act
	{
	public:

		///
		virtual shared_ptr< IO::Device > result() =0 ;

		void reset() {} ;
	} ;

	//-------------------------
	/**	\class Device_Generator
	 *	\brief A device generator is an input stream of devices.
	 *		Its interface is modeled after ACT::Generator.
	 *
	 *	The prototypical Generator is a bound listening socket.
	 *	A successful call to accept(), yielding a new connection, triggers completion of the action.
	 *
	 *	A generator completes when it has a device ready to be retrieved as a result.
	 *	If a generator has an internal list of devices (explicitly or implicitly), then completion makes only one of them available.
	 *	Subsequent devices will be made available after a reset and then another activation.
	 */
	class Device_Generator
	{
	public:

		/// Next device in the generator stream.
		/// \post
		/// - return is a null shared pointer at the current end of the stream.
		virtual shared_ptr< IO::Device > next_device() =0 ;

		/// Query whether generator will ever return non-null again.
		/// See also ACT::Generator::completed.
		virtual bool completed() { return false ; }

		/// Order a shutdown.
		/// See also ACT::Generator::shutdown.
		virtual void shutdown() =0 ;
	} ;

	//-------------------------
	/**	\class IO_Generator
	 *	\brief A device adapter converts a device generator into an action generator.
	 *		It does so by passing a device from our upstream generator to a behavior factory.
	 *		The result is the downstream result of our activity as an action generator.
	 *	
	 *	To use this class, create an instance and pass it to the constructor of ACT::Service.
	 *	The Service class provides for control under a scheduler.
	 *	The wakeup_listener function provided to next_action is that of this Service instance.
	 *
	 *	\invariant
	 *	- the_generator.bad() implies complete
	 */
	class IO_Generator
		: public ACT::Generator
	{
		/// Our device generator provides us a stream of device.
		Old_Device_Generator & the_generator ;

		/// Our behavior factory provices bind-conversion of devices to actions.
		Behavior_Factory & the_behavior_factory ;

		/// Operation of this generator is complete, initially false except for null generators.
		bool complete ;

		/// Since completion is one-way, perhaps this belongs in the base class.
		inline void set_completed() { complete = true ; }

		/// Overrides ACT::Generator::completed
		bool completed() { return complete ; }

	public:
		/// Ordinary constructor
		IO_Generator( Old_Device_Generator & x, Behavior_Factory & y ) ;

		/// Implements of ACT::Generator::next_action
		virtual shared_ptr< ACT::basic_act > next_action( ACT::wakeup_listener * ) =0 ;

		/// Implements of ACT::Generator::shutdown
		void shutdown() ;
	} ;

} // end namespace IO

#endif