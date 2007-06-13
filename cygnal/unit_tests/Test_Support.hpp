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

/// \file Test_Support.hpp
/// \brief Generic Support for Unit Tests

#pragma once
#ifndef __Test_Support_hpp__
#define __Test_Support_hpp__

#include <boost/test/auto_unit_test.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
using boost::shared_ptr ;

//--------------------------------------------------
/** \class generator_base
 *	\brief Base class for template concrete_generator.
 *	It abstracts the iterator-pair functionality away from the class representation.
 */
class generator_base
{
protected:
	/// Increment operator, implemented in subclass.
	virtual void increment_begin_iterator() =0 ;

	/// End-of-sequence predicate, implemented in subclass.
	virtual bool begin_equals_end() const =0 ;

public:
	/// The function object for the test associated with the current value of the begin iterator.
	virtual boost::function< void( void ) > test_function() const =0 ;

	/// The name of the test associated with the current value of the begin iterator.
	virtual std::string name() const =0 ;

	/// Increment operator is passed off to implementation class.
	inline void advance() { increment_begin_iterator() ; }

	/// Whether the begin iterator equals the end iterator
	inline bool done() const { return begin_equals_end() ; }
} ;

//--------------------------------------------------
/**	\class concrete_generator
 *	\brief An instance of this generic class stores an iterator-pair of some particular test generator.
 *		It adapts their behavior to generate a sequence of test functions and names.
 *
 *	Instances of this class are dynamically allocated to initialize the generator that the test framework sees.
 */
template< class Iter >
class concrete_generator
	: public generator_base 
{
	/// Beginning of sequence at construction; first item in remaining sequence at all times.
	Iter begin ;

	/// End marker
	const Iter end ;

	/// Increment operator uses the native increment operation of the iterator
	void increment_begin_iterator()
	{
		++ begin ;
	}

	bool begin_equals_end() const 
	{
		return begin == end ;
	}

	std::string name() const
	{
		return begin.name() ;
	}

	boost::function< void( void ) > test_function() const
	{
		return begin.test_function() ;
	}

public:
	/// Ordinary constructor
	concrete_generator( Iter begin, Iter end )
		: begin( begin ), end( end )
	{}
} ;

//--------------------------------------------------
/**	\class auto_generator
 *	\brief An adapter that converts the iterator style generator used within to a test generator as the unit test framework sees it.
 */
class auto_generator
	: public boost::unit_test::test_unit_generator
{
	mutable shared_ptr< generator_base > g ;
	mutable bool done ;
	std::string basename ;

public:
	auto_generator( shared_ptr< generator_base > g, std::string basename )
		: g( g ), done( false ), basename( basename )
	{}

	/// Wrapper around a function object because the test framework doesn't support function objects directly.
	class test
	{
		boost::function< void( void ) > f ;
	public:
		test( boost::function< void( void ) > f ) : f( f ) {}
		void run() { f() ; }
	} ;

	/// Instance of virtual function declared in test_unit_generator.
	boost::unit_test::test_case * next() const ;
} ;

//--------------------------------------------------
/**	\fn make_generator
 *	\brief Adapter function allowing use of type inference to initialize test generator.
 */
template< class Gen >
auto_generator make_generator( Gen generator_instance, std::string name )
{
	shared_ptr< concrete_generator< Gen::iterator > > 
		g( new concrete_generator< Gen::iterator >( generator_instance.begin(), generator_instance.end() ) ) ;
	return auto_generator( g, name ) ;
}

//--------------------------------------------------
// The macro used in the test source to automatically register all the generated tests.
#define BOOST_AUTO_TEST_GENERATOR( name, gen ) \
	static auto_generator BOOST_AUTO_TC_UNIQUE_ID( name )( make_generator( gen, BOOST_STRINGIZE( name ) ) ) ; \
BOOST_AUTO_TC_REGISTRAR( name )( BOOST_AUTO_TC_UNIQUE_ID( name ) ) ;

#endif	// end of inclusion protection
