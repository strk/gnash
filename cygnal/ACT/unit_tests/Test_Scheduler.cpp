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

/// \file Test_Scheduler.cpp
/// \brief Tests of the Scheduler

#include "unit_tests/cygnal_test.hpp"

#include "ACT/Scheduler.hpp"
#include "ACT/test_support/Simple_Actions.hpp"
#include "ACT/test_support/Listening_Actions.hpp"
#include "ACT/test_support/Supplied_Service.hpp"
#include "ACT/Pause_Demon.hpp"

// Include implementations for explicit instantiation.
#include "ACT/Handle.cpp"
#include "ACT/Scheduler.T.cpp"
#include "ACT/Scheduling_Queue.cpp"

namespace ACT_Test {
	//--------------------------------------------------
	// White box test of Handle using aspects.
	//--------------------------------------------------
	using namespace ACT ;

	//---------------
	// This is the template definition of the base aspect class.
	// Specializations of this class provide the actual aspect classes compiled in to the classes under test.

	/// \brief Generic template declaration to be specialized for particular class aspects.
	template< class, class > class test_aspect {} ;
	/// \brief Generic template declaration to be specialized for particular class aspects.
	template< class, class, class > class test_aspect_2 {} ;

	//---------------
	/// \brief Test aspect for Handle_Registry_Leader
	template< class T >
	class test_aspect< T, Handle_Registry_Leader< T, test_aspect > >
		: public aspect::Null_Aspect_1< T, Handle_Registry_Leader< T, aspect::Null_Aspect_1 > >
	{
		// We need the execution trace in this class because it's constructed statically.
		// If we had two static classes that required a common trace, we'd have to use a third class, a singleton, to put this in.
		execution_trace tr ;

	public:
		execution_trace & trace() { return tr ; }
		std::string result() const { return tr.result() ; }

		void add_in_new_place() { tr.add( "N" ) ; }
		void add_in_old_place() { tr.add( "O" ) ; }
	} ;

	//---------------
	/// \brief Test aspect for Handle_Registry_Follower.
	template< class T, class Leader >
	class test_aspect_2< T, Leader, Handle_Registry_Follower< T, Leader, test_aspect_2 > >
		: public aspect::Null_Aspect_2< T, Leader, Handle_Registry_Follower< T, Leader, aspect::Null_Aspect_2 > >
	{
		// See reason above for why the execution trace is here
		execution_trace tr ;
	public:
		execution_trace & trace() { return tr ; }
		std::string result() const { return tr.result() ; }

		void access_from_existing_slot() { tr.add( "A" ) ; }
		void enlarge_size_of_vector() { tr.add( "X" ) ; }
	} ;

	//---------------
	// Forward
	class test ;
	//---------------
	/// \brief Test aspect for Handled.
	template< class T >
	class test_aspect< T, Handled< T, test_aspect > >
		: public Null_Aspect_Handled< T >,
		public aspect::Aspect_Has_Access_To_Owner< Handled< T, test_aspect > >
	{
		/// The explicit namespace qualification distinguishes the generic declaration from this specialization.
		typedef Handled< test, ACT_Test::test_aspect > owner_type ;

		///
		typedef aspect::Aspect_Has_Access_To_Owner< Handled< T, ACT_Test::test_aspect > > access_base_type ;

	public:
		std::string result() const {
			return access_base_type::owner() -> our_registry.aspect.result() ;
		}
	} ;

	//---------------
	/// \brief A test class for \c Handled, deriving from it in order to exercise it.
	class test
		: public Handled< test, test_aspect >
	{
		typedef Handled< test, test_aspect > Handled_Base ;
	public:
		test() : Handled_Base( this ) {}

		std::string result() const { return aspect.result() ; }
	} ;

	//---------------
	BOOST_AUTO_UNIT_TEST( handle_one )
	{
		// We declare variables within blocks to invoke their destructors.
		{
			test x ;
			BOOST_CHECK( test::registry_at( x.handle() ) == & x ) ;
			BOOST_CHECK( x.result() == "N" ) ;
		}
		// The second time, there should be an existing entry in the free list to allocate from
		{
			test x ;
			BOOST_CHECK( test::registry_at( x.handle() ) == & x ) ;
			BOOST_CHECK( x.result() == "NO" ) ;
		}
		{
			test x ;
			BOOST_CHECK( test::registry_at( x.handle() ) == & x ) ;
			BOOST_CHECK( x.result() == "NOO" ) ;
			// Another object after the previous test.
			test y ;
			BOOST_CHECK( test::registry_at( y.handle() ) == & y ) ;
			BOOST_CHECK( y.result() == "NOON" ) ;
			// Since the execution trace should be in a common class registry, results from both objects should match.
			BOOST_CHECK( y.result() == x.result() ) ;
		}
	}

	//---------------
	BOOST_AUTO_UNIT_TEST( handle_two )
	{
		test x ;
		Handle_Registry_Follower< int, test, test_aspect_2 > follower ;
		std::string found( follower.aspect.result() ) ;
		std::string expected( "" ) ;
		BOOST_CHECK( found == "" ) ;

		follower[ x.handle() ] = 1 ;
		found = follower.aspect.result() ;
		expected = "XA" ;		// Expand follower registry, then add
		BOOST_CHECK_MESSAGE( found == expected,
			"Found result " << found << ". Expected result " << expected << "." ) ;

		test y ;
		follower[ y.handle() ] = 2 ;
		found = follower.aspect.result() ;
		expected = "XAXA" ;		// Expand again, then add
		BOOST_CHECK_MESSAGE( found == expected,
			"Found result " << found << ". Expected result " << expected << "." ) ;

		x.~test() ;		// invoke destructor to free up existing handle
		test z ;		// should issue the handle that 'x' had.
		follower[ z.handle() ] = 3 ;
		found = follower.aspect.result() ;
		expected = "XAXAA" ;		// No expansion this time, just add
		BOOST_CHECK_MESSAGE( found == expected,
			"Found result " << found << ". Expected result " << expected << "." ) ;
	}

	//--------------------------------------------------
	// Basic exercises for the scheduler.
	//--------------------------------------------------

	/// \brief Body of test aspect for \c Basic_Scheduler; aspect itself is a pointer to this class.
	class scheduler_aspect_body
	{
		size_t execution_count ;
		size_t execution_bound ;
	public:
		scheduler_aspect_body()
			: execution_bound( 0 ) {}

		void run_begin() { execution_count = 0 ; }

		bool run_guard()
		{
			if ( execution_count >= execution_bound ) return false ;
			++ execution_count ;
			return true ;
		}

		void set_execution_bound( size_t n ) { execution_bound = n ; }

		bool finished_within_bound() { return execution_count <= execution_bound ; }

		bool finished_at_bound() { return execution_count == execution_bound ; }

	} ;

	template< class Owner > class scheduler_aspect ;

	/// \brief test aspect for \c Basic_Scheduler
	template<>
	class scheduler_aspect< Basic_Scheduler< scheduler_aspect > >
		: public Basic_Scheduler_Null_Aspect,
		public aspect::Aspect_Has_Access_To_Owner< Basic_Scheduler< scheduler_aspect > >
	{
		scheduler_aspect_body * body ;

	public:
		scheduler_aspect()
			: body( 0 ) {}

		scheduler_aspect( scheduler_aspect_body * b )
			: body( b ) {}

		void run_begin() { if ( body ) body -> run_begin() ; }

		bool run_guard() { return ( body ) ? body -> run_guard() : true ; }

		void set_execution_bound( size_t n ) { if ( body ) body -> set_execution_bound( n ) ; }

		bool finished_within_bound() { return ( body ) ? body -> finished_within_bound() : false ; }

		bool finished_at_bound() { return ( body ) ? body -> finished_at_bound() : false ; }

	} ;

	typedef scheduler_aspect< Basic_Scheduler< scheduler_aspect > > Test_Scheduler_Aspect ;
	typedef Basic_Scheduler< scheduler_aspect > Test_Scheduler ;

	// Explicit instantiation of our Test_Scheduler
	template Test_Scheduler ;

	//--------------------------------------------------
	// This test checks that the execution guard functions correctly to limit the total number of execution passes.
	BOOST_AUTO_UNIT_TEST( guard_functions )
	{
		scheduler_aspect_body a ;
		Test_Scheduler b = Test_Scheduler( Test_Scheduler_Aspect( & a ) ) ;

		/* N_to_completion( k ) causes the execution of '2k' actions:
		 *	- 'k' actions of the N_to_completion task
		 *	- 'k' actions of the listening monitor service.
		 * N_to_completion( 6 ) causes 12 action invocations.
		 * We run them 4 at a time.
		 * After the last one, the queue should be empty.
		 */
		b.add_task( act( new N_to_completion( 6, 0 ) ) ) ;
		a.set_execution_bound( 4 ) ;
		b() ;
		BOOST_CHECK( a.finished_within_bound() ) ;
		BOOST_CHECK( ! b.empty() ) ;
		b() ;
		BOOST_CHECK( a.finished_within_bound() ) ;
		BOOST_CHECK( ! b.empty() ) ;
		b() ;
		BOOST_CHECK( a.finished_at_bound() ) ;
		BOOST_CHECK( b.empty() ) ;
	}

	BOOST_AUTO_UNIT_TEST( some_single_actions )
	{
		scheduler_aspect_body a ;
		Test_Scheduler b = Test_Scheduler( Test_Scheduler_Aspect( & a ) ) ;

		execution_trace tr ;
		b.add_task( act( new no_action( new simple_tracker( tr, "N" ) ) ) ) ;
		b.add_task( act( new single_action( new simple_tracker( tr, "A" ) ) ) ) ;
		b.add_task( act( new single_action( new simple_tracker( tr, "B" ) ) ) ) ;
		b.add_task( act( new single_action( new simple_tracker( tr, "C" ) ) ) ) ;

		a.set_execution_bound( 100 ) ;
		b() ;
		std::string expected( "ABC" ) ;
		BOOST_CHECK_MESSAGE( tr.result() == expected,
			"Found result " << tr.result() << ". Expected result " << expected << "." ) ;
		BOOST_CHECK( a.finished_within_bound() ) ;
		BOOST_CHECK( b.empty() ) ;
	}

	BOOST_AUTO_UNIT_TEST( act_n_interleaved )
	{
		scheduler_aspect_body a ;
		Test_Scheduler b = Test_Scheduler( Test_Scheduler_Aspect( & a ) ) ;

		execution_trace tr ;
		b.add_task( act( new N_to_completion( 2, new simple_tracker( tr, "A" ) ) ) ) ;
		b.add_task( act( new N_to_completion( 3, new simple_tracker( tr, "B" ) ) ) ) ;
		b.add_task( act( new N_to_completion( 5, new simple_tracker( tr, "C" ) ) ) ) ;
		
		a.set_execution_bound( 100 ) ;
		b() ;
		std::string expected( "ABCABCBCCC" ) ;
		BOOST_CHECK_MESSAGE( tr.result() == expected,
			"Found result " << tr.result() << ". Expected result " << expected << "." ) ;
		BOOST_CHECK( b.empty() ) ;
	}

	//--------------------------------------------------
	// Same as act_n_interleaved, but using a Supplied_Service

	BOOST_AUTO_UNIT_TEST( act_n_service )
	{
		scheduler_aspect_body a ;
		Test_Scheduler b = Test_Scheduler( Test_Scheduler_Aspect( & a ) ) ;

		execution_trace tr ;
		Supplied_Service * ss = new Supplied_Service( b, new simple_tracker( tr, "S" ) ) ;
		ss -> add_task( shared_ptr< basic_act >( new N_to_completion( 2, new simple_tracker( tr, "A" ) ) ) ) ;
		ss -> add_task( shared_ptr< basic_act >( new N_to_completion( 3, new simple_tracker( tr, "B" ) ) ) ) ;
		ss -> add_task( shared_ptr< basic_act >( new N_to_completion( 5, new simple_tracker( tr, "C" ) ) ) ) ;

		b.add_service( act( ss ) ) ;
		ss -> shutdown() ;
		a.set_execution_bound( 100 ) ;
		b() ;
		// First "S" is to enter the new tasks into the queue.
		// Second "S" is to allow the service to return Completed and leave the scheduling queue.
		std::string expected( "SABCSABCBCCC" ) ;
		BOOST_CHECK_MESSAGE( tr.result() == expected,
			"Found result " << tr.result() << ". Expected result " << expected << "." ) ;
	}

	//--------------------------------------------------
	BOOST_AUTO_UNIT_TEST( pause_action )
	{
		scheduler_aspect_body a ;
		Test_Scheduler b = Test_Scheduler( Test_Scheduler_Aspect( & a ) ) ;
		BOOST_REQUIRE( b.empty() ) ;

		Pause_Demon * pause( new Pause_Demon( & b ) ) ;
		b.add_task( act( new no_action( 0 ) ) ) ;
		// This pause action should immediately return, because there's a pending action in the scheduler.
		( * pause )() ;

		a.set_execution_bound( 2 ) ;
		b.add_service( act( pause ) ) ;
		b() ;
		BOOST_CHECK( a.finished_at_bound() ) ;	// Pause demon should execute once, no_action once.
		BOOST_CHECK( ! b.empty() ) ;			// demon should still be in queue.
	}

} // end namespace ACT_test
