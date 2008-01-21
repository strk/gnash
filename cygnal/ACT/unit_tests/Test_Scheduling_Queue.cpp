// 
// Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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

/// \file Test_Scheduling_Queue.cpp
/// \brief Tests of the Scheduling Queue

#include "unit_tests/cygnal_test.hpp"

#include "unit_tests/Test_Support.hpp"
#include "unit_tests/Permutation.hpp"
#include "unit_tests/Random_Permutation.hpp"


/** A test item to put into a scheduling queue.
 */
class test_item {
	size_t it ;
public:
	explicit test_item( size_t x )
		: it( x ) {}

	bool operator<( const test_item & x ) const { return it < x.it ; }
	inline bool operator==( unsigned int x ) const { return it == x ; }
	size_t get() { return it ; }
	void set( size_t n ) { it = n ; }
} ;

// Explicit instantiation
#include "ACT/Scheduling_Queue.cpp"
#include "ACT/Scheduler.T.cpp"
template class ACT::wakeup_listener_allocated< ACT::Basic_Scheduler<> > ;
template class ACT::Scheduling_Queue< test_item, ACT::wakeup_listener_allocated< ACT::Basic_Scheduler<> > > ;

typedef ACT::Scheduling_Queue< test_item, ACT::wakeup_listener_allocated< ACT::Basic_Scheduler<> > > queue_type ;
typedef queue_type::pointer pointer ;

using namespace ACT ;

//--------------------------------------------------
BOOST_AUTO_TEST_CASE( simple_queue_exercise )
{
	queue_type q ;

	test_item a( 1 ) ;
	test_item b( 2 ) ;
	test_item c( 3 ) ;

	q.push( a, 0 ) ;
	q.push( b, 0 ) ;
	q.push( c, 0 ) ;

	pointer x( q.top_ptr() ) ;
	test_item it = * x ;
	BOOST_CHECK( it == 3 ) ;
	BOOST_CHECK( q.top() == 3 ) ;

	q.pop() ;
	BOOST_CHECK( q.top() == 2 ) ;

	q.pop() ;
	BOOST_CHECK( q.top() == 1 ) ;
}

//--------------------------------------------------
void
add_then_test_order( permutation_iterator::const_iterator begin, permutation_iterator::const_iterator end )
{
	queue_type q ;
	size_t permutation_size = end - begin ;

	size_t j ;
	for ( j = 0 ; j < permutation_size ; ++ j ) {
		q.push( test_item( * begin ++ ), 0 ) ;
	}
	for ( j = 0 ; j < permutation_size ; ++ j ) {
		pointer p( q.top_ptr() ) ;
		test_item it = * p ;
		size_t n = permutation_size - 1 - j ;
		BOOST_CHECK( it.get() == n ) ;
		size_t x = q.top().get() ;
		BOOST_CHECK_MESSAGE( x == n, 
			"Found top value " << x << ". Expected value " << n << ". Size = " << permutation_size << "." ) ;
		q.pop() ;
	}
	BOOST_CHECK( q.empty() ) ;
}

// The permutation has exponential growth with its parameter.
// Pushing it past 5 means significant wait times during testing.
// At n=9 you'll start wondering if it's hung.
//BOOST_AUTO_TEST_GENERATOR( ordering_all_, permutation( 5, add_then_test_order ) )

// This permutation test 
// [EH 2007-06-04] Hey, this test identified a critical defect.  Go unit test!
BOOST_AUTO_TEST_GENERATOR( ordering_random1_, random_permutation( add_then_test_order, 200, 20 ) )

//BOOST_AUTO_TEST_GENERATOR( ordering_random2_, random_permutation( add_then_test_order, 20000, 1 ) )

//--------------------------------------------------
BOOST_AUTO_TEST_CASE( another_simple_queue_exercise )
{
	queue_type q ;

	test_item a( 4 ) ;
	test_item b( 5 ) ;
	test_item c( 6 ) ;

	q.push( a, 0 ) ;
	q.push( b, 0 ) ;
	q.push( c, 0 ) ;

	pointer x( q.top_ptr() ) ;
	test_item it = * x ;
	BOOST_CHECK( it == 6 ) ;
	BOOST_CHECK( q.top() == 6 ) ;

	x = q.top_ptr() ;
	x -> set( 3 ) ;
	q.reorder( x ) ;
	BOOST_CHECK( q.top() == 5 ) ;

	x = q.top_ptr() ;
	x -> set( 2 ) ;
	q.reorder( x ) ;
	BOOST_CHECK( q.top() == 4 ) ;

	x = q.top_ptr() ;
	x -> set( 1 ) ;
	q.reorder( x ) ;
	BOOST_CHECK( q.top() == 3 ) ;

	q.pop() ;
	BOOST_CHECK( q.top() == 2 ) ;

	q.pop() ;
	BOOST_CHECK( q.top() == 1 ) ;

	q.pop() ;
	BOOST_CHECK( q.empty() ) ;
}

//--------------------------------------------------
typedef std::vector< unsigned int > vector_type ;
typedef vector_type::iterator iterator ;
typedef vector_type::const_iterator const_iterator ;

/*	The body of the test is separated out so that it might be used from a generator.
 *
 *	\precondition
 *	- [begin,middle) is a permutation of the integers [0, middle - begin)
 *	- [middle,end) is a permutation of the integers [middle - begin, end - begin)
 *	- middle - begin <= end - middle
 *
 *	This test will actually work in other circumstance, but they're harder to generate.
 *	The idea is that priority changes must be within a window of size equal to the number of entries already present.
 */
void
add_then_reorder( const iterator begin, iterator middle, iterator end )
{
	queue_type q ;

	size_t n = middle - begin ;
	size_t n2 = end - middle ;
	size_t max = n + n2 - 1 ;
	size_t j ;
	iterator i = middle ;

	/* Add elements to the queue from middle to end
	 */
	for ( j = 0 ; j < n2 ; ++ j ) {
		q.push( test_item( * i ++ ), 0 ) ;
	}
	i = begin ;
	for ( j = 0 ; j < n ; ++ j ) {
		pointer x( q.top_ptr() ) ;
		size_t y = x -> get() ;
		BOOST_CHECK_MESSAGE( y == max,
			"Found top item " << y << ". Expected item " << max << ". Phase 1." ) ; 
		-- max ;
		x -> set( * i ++ ) ; 
		q.reorder( x ) ;
	}
	for ( j = 0 ; j < n2 ; ++ j ) {
		BOOST_CHECK_MESSAGE( q.top().get() == max,
			"Found top item " << q.top().get() << ". Expected item " << max << ". Phase 2." ) ;
		-- max ;
		q.pop() ;
	}
	BOOST_CHECK( q.empty() ) ;
}

/*	This test exercises the use pattern that an ACT at the top of the queue is executed then rescheduled
 *	at lower priority, generally while waiting for I/O to complete.
 */
BOOST_AUTO_TEST_CASE( parametric_add_permuted_reorder_all )
{
	vector_type v ;
	const size_t upper = 100 ;
	const size_t lower = 100 ;

	for ( size_t j = 0 ; j < upper + lower ; ++ j ) {
		v.push_back( j ) ;
	}

	iterator x1( v.begin() ) ;
	iterator x2( x1 + lower ) ;
	iterator x3( x2 + upper ) ;

	std::random_shuffle( x1, x2 ) ;
	std::random_shuffle( x2, x3 ) ;

	add_then_reorder( x1, x2, x3 ) ;
}

//--------------------------------------------------
void
add_then_wakeup( const iterator begin, const iterator middle, const iterator end )
{
	queue_type q ;
	const size_t number_of_elements = middle - begin ;
	size_t j ;
	iterator i( begin ) ;
	std::vector< pointer > pp ;

	while ( i != middle ) {
		pp.push_back( q.push( test_item( * i ++ ), 0 ) ) ;
	}
	BOOST_REQUIRE( i == begin + number_of_elements ) ;

	const size_t number_of_wakeups = end - middle ;
	for ( j = number_of_elements ; j < number_of_wakeups + number_of_elements ; ++ j ) {
		size_t k = * i ++ ;
		pp[ k ] -> set( j ) ;		// j is the current highest priority
		q.reorder( pp[ k ] ) ;
		size_t found = q.top().get() ;
		BOOST_CHECK( found == j ) ;
	}
}

/*	This test exercises the use pattern that an ACT somewhere in the queue wakes up and becomes ready for execution.
 */
BOOST_AUTO_TEST_CASE( add_permuted_wakeup_all )
{
	vector_type v ;
	const size_t n = 100 ;
	size_t j ;

	// First segment initializes a set of distinct objects
	for ( j = 0 ; j < n ; ++ j ) {
		v.push_back( j ) ;
	}
	std::random_shuffle( v.begin(), v.end() ) ;
	iterator begin = v.begin() ;

	// Second segment is a sequence of objects that wake up
	typedef unsigned int result_type ;
	typedef boost::mt19937 engine_type ;
	typedef boost::uniform_int< result_type > distribution_type ;
	boost::variate_generator< engine_type, distribution_type > generator( engine_type( 39650193 ), distribution_type( 0, 100 ) ) ;

	for ( j = 0 ; j < n ; ++ j ) {
		v.push_back( generator() ) ;
	}

	iterator x1( v.begin() ) ;
	iterator x2( x1 + n ) ;
	iterator x3( x2 + n ) ;

	add_then_wakeup( x1, x2, x3 ) ;
}

