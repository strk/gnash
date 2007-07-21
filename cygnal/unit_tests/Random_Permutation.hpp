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

/// \file Random_Permutation.hpp
/// \brief A permutation generator for test cases.

#pragma once
#ifndef __Random_Permutation_hpp__
#define __Random_Permutation_hpp__

#include "Permutation.hpp"

#include <vector>
#include <boost/random/variate_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>


class random
{
	typedef unsigned int result_type ;
	typedef boost::mt19937 engine_type ;
	typedef boost::uniform_int< result_type > distribution_type ;

	boost::variate_generator< engine_type, distribution_type > generator ;
public:
	random()
		: generator( engine_type( 39650193 ), distribution_type( 0, 9 ) )
	{}
	
	result_type operator()()
	{
		return generator() ;
	}
} ;

// Forward declaration
class random_permutation ;

class random_permutation_iterator
{
public:
	typedef std::vector< unsigned int > vector_type ;
	typedef vector_type::const_iterator const_iterator ;
	typedef boost::function< void( const_iterator, const_iterator ) > test_function_type ;
private:
	test_function_type f ;

	unsigned int set_size ;

	unsigned int number_of_iterations ;

	vector_type the_vector ;

	typedef unsigned int result_type ;
	typedef boost::mt19937 engine_type ;
	typedef boost::uniform_int< result_type > distribution_type ;

	/// uniform_int is documented as having a second operator()( IntType ) that would allow it to be
	/// used to generate random permutations.  As of Boost 1.33, such an operator did not exist.
	/// The upshot is that this generator is not currently used.
	/// It's still here as a stub for future implementation.
	boost::variate_generator< engine_type, distribution_type > generator ;

	/// owner of iterator required for equality testing
	const random_permutation * owner ;

	void randomize() ;

public:
	boost::function< void( void ) > test_function() const
	{
		return boost::bind( f, the_vector.begin(), the_vector.end() ) ;
	}

	/// Ordinary constructor
	random_permutation_iterator( const random_permutation *, test_function_type f, unsigned int set_size, unsigned int number_of_iterations ) ;

	void operator++() ;

	bool operator==( const random_permutation_iterator & ) const ;

	std::string name() const ;
} ;

class random_permutation
{
public:
	typedef random_permutation_iterator iterator ;
	typedef const random_permutation_iterator const_iterator ;
	typedef random_permutation_iterator::test_function_type test_function_type ;
private:
	unsigned int n ;

	unsigned int number_of_iterations ;

	test_function_type f ;

public:
	random_permutation( test_function_type f, unsigned int n, unsigned int number_of_iterations )
		: f( f ), n( n ), number_of_iterations( number_of_iterations ) {}

	iterator begin() const
	{
		return iterator( this, f, n, 0 ) ;
	}

	iterator end() const
	{
		return iterator( this, f, n, number_of_iterations ) ;
	}
} ;


#endif	// end of inclusion protection

