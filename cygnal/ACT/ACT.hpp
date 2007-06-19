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

/**	\file ACT.hpp
 *	Asynchronous Computation Task
 */

#pragma once
#ifndef __ACT_hpp___
#define __ACT_hpp___

#include <boost/functional.hpp>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr ;

namespace ACT {

	class wakeup_listener ;

	//-------------------------
	/** \enum act_state
	 *	\brief Generically relevant ACT states.
	 *
	 *	There are exactly three generic ACT states.
	 *	Particular kinds of ACT may refine this set of states, but they must also preserve the meaning of these three.
	 *	- Working.  Not yet finished.
	 *	- Completed.  Finished normally.
	 *	- Bad.  Finished abnormally.
	 */
	enum act_state { Working, Completed, Bad } ;

	//-------------------------
	/**	\class basic_act
	 *	\brief Base class for specific ACT uses.
	 *
	 *	This class has no actions.
	 *	That's a job for the subclasses.
	 *	This class provides notation for the control state of a task.
	 *
	 *	States:
	 *	- Working.  The ACT hasn't yet completed.  This is the initial state,
	 *	- Done.  The ACT has completed successfully.  This is a final state.
	 *	- Bad.  The ACT experienced an error that caused it to be unable to proceed.
	 *		Ordinarily this is a final state, but certain kinds of ACT's may be able to recover.
	 *
	 *	\par Theory Notes
	 *	As of this writing, there's no formalization of asynchronous computation tasks.
	 *	The ordinary notion of the keyword "return" interlinks two different kinds of ideas:
	 *	- Passing control flow back to the statements of a calling function.
	 *	- Indicating completion of the task that a subroutine represents.
	 *	Presumably the caller of an ACT wants it to be able to complete, but not necessarily.
	 *	Perhaps it simply wants an initial try at something, trying something more complicated if that doesn't succeed.
	 *	In any case, it's up to the caller to determine what it wants to do with an incomplete task.
	 *	
	 *	The basic idea of an ACT splits the keyword "return" into two different kinds.
	 *	- The ordinary "return", which indicates completion of the task.
	 *		This is represented by the states \c Completed and \c Bad.
	 *		This kind of
	 *	- An interruption "yield", which indicates not-yet-completion of the task.
	 *		This is represented by the state \c Working.
	 *
	 *	A question of nested control arises.
	 *	A typical use (although not the only one) is to build an ACT by calling other ACT's.
	 *	In this idiom, an ACT with a still-working sub-ACT is itself also still working.
	 *	Thus the following code often appears:
	 *		action() ;
	 *		if ( action.working() ) return ;
	 *	Furthermore, such code should have the property that when called again, 
	 *		its internal state leads the action to be called again.
	 *
	 *	\par Design Notes
	 *	There's no particular _a priori_ distinction between Completed and Bad states.
	 *	The distinction between these two is whether an action has finished in a good or bad state.
	 *	What _is_ common is that in either state, processing of this task has finished.
	 *
	 *	It's up to the specifier of a subclass to distinguish the meaning of "good" and "bad".
	 *	We may formalize this as a "success condition", a predicate similar to a postcondition.
	 *	The postcondition of operator() says that Completed actions satisfy the success condition
	 *		and that Bad actions violate them.
	 *	From a language point of view, a success condition would be (an analogue of) a template parameter.
	 *	Such predicates are not part of most languages as of this writing.
	 *
	 *	\par Implementation Notes
	 *	The underlying state is private.
	 *	Its assigners are protected for use only in subclasses.
	 *	Its accessors are public, since they're involved in control flow.
	 */
	class basic_act {
		/// Child classes may access and alter the internal state.
		act_state the_state ;

	protected:
		/// Protected constructor for child classes.
		basic_act() 
			: the_state( Working )
		{}

		/// Protected setter for subclass implementation.
		inline act_state set_working() { return the_state = Working ; }

		/// Protected setter for subclass implementation.
		inline act_state set_completed() { return the_state = Completed ; }

		/// Protected setter for subclass implementation.
		inline act_state set_bad() { return the_state = Bad ; }

		/// Protected arbitrary setter for subclasses that filter.
		inline act_state set_state( act_state x ) { return the_state = x ; }

	public:
		///
		inline act_state internal_state() { return the_state ; }

		/// 
		inline bool working() const { return the_state == Working ; }

		/// 
		inline bool completed() const
		{
			return the_state == Completed ;
		}

		/// 
		inline bool bad() const { return the_state == Bad ; }

		///
		virtual act_state operator()( void ) =0 ;

		///
		virtual act_state operator()( wakeup_listener * ) =0 ;

	} ;

	//-------------------------
	/**	\class simple_act
	 *	\brief Base class for ACT without background processing and no need for notification
	 *
	 */
	class simple_act
		: public basic_act
	{
	protected:
		/**	\brief This function is the body of an ACT.
		 *	It is reasonable to consider this as an analogue of a thread body.
		 *
		 *	\pre 
		 *	- the_state == Working.
		 *		(Note: operator(), acting as a facade, ensures this predicate.)
		 *	\post
		 *	- the_state == Working implies success conditions are not yet met nor are they yet precluded.
		 *	- return == internal_state().
		 */
		virtual act_state run( void ) =0 ;

	public:
		/**	\brief operator() is a facade for the body of an ACT.
		 *
		 *	\post
		 *	- the_state == Working implies success conditions are not yet met nor are they yet precluded.
		 *	- return == internal_state().
		 */
		inline act_state operator()( void )
		{
			if ( ! working() ) return internal_state() ;
			return run() ;
		}

		inline act_state operator()( wakeup_listener * )
		{
			return operator()() ;
		}
	} ;

	//-------------------------
	/**	\class autonomous_act
	 *	\brief Base class for ACT that may initiate background activity and require notification
	 *
	 */
	class autonomous_act
		: public basic_act
	{
		/// Facade class has direct access to avoid indirecting status calls.
		friend class act ;

	protected:
		/**	\brief This is the body of the ACT.
		 *	It is reasonable to consider this as an analogue of a thread body.
		 *	This is the variety of the body that may initiate autonomous activity that sends asynchronous notice of completion.
		 *
		 *	\pre 
		 *	- the_state == Working.
		 *		(Note: the proxy for this function in the facade class enforces this.)
		 *	\post
		 *	- the_state == Working implies wakeup_listener !=0 implies 
		 *		wakeup_listener may be called when progress on the ACT is possible.
		 *
		 *	\note
		 *	The postcondition is not a guarantee that a \c wakeup_listener <b>will</b> be called,
		 *		but rather that a reliance upon the caller that this instance <b>may</b> call
		 *		the listener (assuming it's not null).
		 *	Certain ACT's may provide such a guarantee that they will call.
		 *	They might do this by, for example, registering with the OS for a notification
		 *		when I/O is available.
		 *
		 *	\note
		 *	The input parameter \c wakeup_listener may indeed be zero.
		 *	The intent behind this choice is to enable a scheduler to forbid an ACT
		 *		from setting up a background poll or notification.
		 *	Should a scheduler do this, it must assume the responsibility for checking back later.
		 *
		 *	\par
		 *	This choice support changing scheduling policy under various load conditions.
		 *	Under low load, outside notification is preferable, since the alternative
		 *		is a de facto polling loop, constantly running the ACT and seeing if it completes.
		 *	This simply uses up CPU.
		 *	Under high load, however, when CPU usage is near maximum, polling and notification
		 *		code simply adds overhead.
		 *	In this situation, a scheduler might skip notification and switch over to a "least
		 *		recently executed" policy for selecting the next ACT to run.
		 *	Furthermore, these policies might be blended, putting "slow" operations into
		 *		notification mode and "fast" operations into direct retry mode.
		 */
		virtual act_state run( wakeup_listener * ) =0 ;

	public:
		///
		inline act_state operator()( wakeup_listener * w )
		{
			if ( ! working() ) return internal_state() ;
			return run( w ) ;
		}

		/// Convenience zero-parameter operator() ;
		inline act_state operator()() { return operator()( 0 ) ; }
	} ;

	//-------------------------
	/**	\class act
	 *	Wrapper around polymorphic ACT child classes.
	 *	Allows hiding 'new' allocation and avoiding defective memory management.
	 *	Enables passing ACT instances by copying pointers.
	 *
	 *	\inv
	 *		the_body != 0
	 *
	 */
	class act
	{
		/// This class is a facade around this pointer to an ACT body.
		shared_ptr< basic_act > the_body ;

	public:
		/**	\pre Pointer to act_body is newly allocated.
		 *	\post
		 *	- This instance has taken responsibility for deallocation.
		 *	- the_body == 0 implies we exited by exception.
		 */
		act( basic_act * the_body ) ;

		/// 
		act( shared_ptr< basic_act > ) ;

		/// Destructor
		~act() ;

		/// Accessor to underlying ACT implementation
		shared_ptr< basic_act > the_underlying_act()
		{
			return the_body ;
		}

		/** Proxy for the body's function operator
		 */
		act_state operator()( wakeup_listener * x ) {
			if ( working() ) {
				return the_body -> operator()( x ) ;
			}
			return the_body -> internal_state() ;
		}

		/// 
		inline bool working() const { return the_body -> working() ; }

		/// 
		inline bool completed() const { return the_body -> completed() ; }

		/// 
		inline bool bad() const { return the_body -> bad() ; }
	} ;

} // end of namespace ACT

#endif	// end of inclusion protection