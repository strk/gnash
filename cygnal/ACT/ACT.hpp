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

/**	\file ACT.hpp
 *	\brief Asynchronous Computation Task
 */

#pragma once
#ifndef __ACT_hpp___
#define __ACT_hpp___

#include <boost/functional.hpp>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr ;

namespace ACT {
	//-------------------------
	/** \class ACT_State
	 *	\brief Generically state of an ACT as a scheduler and other actions see it.
	 *
	 *	This enumeration is at the core of the control flow idiom of the ACT environment.
	 *	When a scheduler or another action calls an action, the action may complete its call before it completes its embodied operation.
	 *	The states break down as follows:
	 *	- Action is not finished.
	 *		In this case, the operation is not yet finished and is still working.
	 *		Further calls to this action will be necessary to get to a finished state,
	 *			which will not happen autonomously.
	 *		This state comes in two varieties.
	 *		- Ready. 
	 *			The action spontaneously yielded control.
	 *			An immediate call to this action would immediately continue.
	 *		- Would_Block.
	 *			The action returned because it would block otherwise.
	 *			An immediate call to this action is unlikely to do anything else.
	 *	- Action has finished.
	 *		One way or another, this action has finished its processing.
	 *		Further calls to this action do nothing.
	 *		- Completed.  
	 *			The action has finished and its ordinary postconditions may be relied upon.
	 *		- Bad.  
	 *			The action has entered an abnormal state and its ordinary postconditions need not be true.
	 *			As a rule, actions in this state terminated early.
	 *
	 *	An important aspect of the Waiting state is that an action that returns this state
	 *		has accepted responsibility for setting in motion its own later wake-up.
	 *	The scheduler does not do this, since it doesn't deal in specifics.
	 */
	class ACT_State
	{
	public:
		/// Symbolic names for the return states of an action.
		enum state {
			/// Not finished.  Ready to continue immediately.
			Ready,
			/// Not finished.  Presumed not ready.
			Would_Block,
			/// Finished normally.
			Completed,
			/// Abnormal termination; now finished.
			Bad
		} ;

	private:
		/// This class is a wrapper around this state.
		state x ;

	public:
		/// Ordinary constructor
		ACT_State( state x ) : x( x ) {} ;

		/// Ordinary assignment
		ACT_State & operator=( state y ) { x = y ; return * this ; }

		/// Convenience test.
		inline bool ready() const { return x == Ready ; }

		/// Convenience test.
		inline bool would_block() const { return x == Would_Block ; }

		/// Convenience test.
		inline bool completed() const { return x == Completed ; }

		/// Convenience test.
		inline bool bad() const { return x == Bad ; }

		/// Test whether state is an unfinished (i.e. still working) state.
		/// If true, it's either Ready or Waiting.
		inline bool working() const { return x <= Would_Block ; }
	} ;

	//-------------------------
	// Forward
	class wakeup_listener ;
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
		ACT_State the_state ;

	protected:
		/// Protected constructor for child classes.
		basic_act() 
			: the_state( ACT_State::Ready )
		{}

		/// Protected setter for subclass implementation.
		inline ACT_State set_ready() { return the_state = ACT_State::Ready ; }

		/// Protected setter for subclass implementation.
		inline ACT_State set_would_block() { return the_state = ACT_State::Would_Block ; }

		/// Protected setter for subclass implementation.
		inline ACT_State set_completed() { return the_state = ACT_State::Completed ; }

		/// Protected setter for subclass implementation.
		inline ACT_State set_bad() { return the_state = ACT_State::Bad ; }

		/// Protected arbitrary setter for subclasses that filter.
		inline ACT_State set_state( ACT_State x ) { return the_state = x ; }

	public:
		///
		inline ACT_State internal_state() { return the_state ; }

		/// 
		inline bool ready() const { return the_state.ready() ; }

		/// 
		inline bool would_block() const { return the_state.would_block() ; }

		/// Return is whether this action is in an unfinished state (either Ready or Waiting)
		inline bool working() const { return the_state.working() ; }

		/// 
		inline bool completed() const { return the_state.completed() ; }

		/// 
		inline bool bad() const { return the_state.bad() ; }

		///
		virtual ACT_State operator()( void ) =0 ;

		///
		virtual ACT_State operator()( wakeup_listener * ) =0 ;

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
		 *	- the_state is working.
		 *		(Note: operator(), acting as a facade, ensures this predicate.)
		 *	\post
		 *	- the_state == Working implies success conditions are not yet met nor are they yet precluded.
		 *	- return == internal_state().
		 */
		virtual ACT_State run( void ) =0 ;

	public:
		/**	\brief operator() is a facade for the body of an ACT.
		 *
		 *	\post
		 *	- the_state == Working implies success conditions are not yet met nor are they yet precluded.
		 *	- return == internal_state().
		 */
		inline ACT_State operator()( void )
		{
			if ( ! working() ) return internal_state() ;
			return run() ;
		}

		inline ACT_State operator()( wakeup_listener * )
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
		 *	The postcondition is not a guarantee that a wakeup_listener <b>will</b> be called,
		 *		but rather that a reliance upon the caller that this instance <b>may</b> call
		 *		the listener (assuming it's not null).
		 *	Certain ACT's may provide such a guarantee that they will call.
		 *	They might do this by, for example, registering with the OS for a notification
		 *		when I/O is available.
		 *
		 *	\note
		 *	The input parameter wakeup_listener may indeed be zero.
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
		 *
		 *	\sa
		 *	ACT_State: For more detailed information on the meaning of the return value.
		 */
		virtual ACT_State run( wakeup_listener * ) =0 ;

	public:
		///
		inline ACT_State operator()( wakeup_listener * w )
		{
			if ( ! working() ) return internal_state() ;
			return run( w ) ;
		}

		/// Convenience zero-parameter operator() ;
		inline ACT_State operator()() { return operator()( 0 ) ; }
	} ;

	//-------------------------
	/**	\class act
	 *	Wrapper around polymorphic ACT child classes.
	 *	Allows hiding 'new' allocation and avoiding defective memory management.
	 *	Enables passing ACT instances by copying pointers.
	 *
	 *	\invariant
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
		ACT_State operator()( wakeup_listener * x ) {
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
