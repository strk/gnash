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

/// \file Permutation.hpp
/// \brief A permutation generator for test cases.

#pragma once
#ifndef __Permutation_hpp__
#define __Permutation_hpp__

//--------------------------------------------------

#include <vector>
#include <string>
#include <sstream>
#include <boost/function.hpp>
#include <boost/bind.hpp>

class permutation_iterator
{
public:
	typedef std::vector< unsigned int > vector_type ;
	typedef vector_type::const_iterator const_iterator ;
	typedef boost::function< void( const_iterator, const_iterator ) > test_function_type ;
private:
	vector_type the_vector ;
	bool at_end ;
	bool at_begin ;
	test_function_type f ;

	const_iterator begin() { return at_end ? the_vector.end() : the_vector.begin() ; }
	const_iterator end() { return the_vector.end() ; }

public:
	typedef vector_type::const_iterator const_iterator ;

	/// Initialize the generator at the beginning
	permutation_iterator( unsigned int n, test_function_type f )
		: the_vector( 0 ),
		f( f ),
		at_end( false ),
		at_begin( true )
	{
		for( unsigned int j = 0 ; j < n ; ++ j ) {
			the_vector.push_back( j ) ;
		}
	}

	/// Initialize the generator at the end
	permutation_iterator( unsigned int n, test_function_type f, int )
		: the_vector( 0 ),
		f( f ),
		at_end( true ),
		at_begin( false )
	{
		// In ordinary usage, next_permutation wraps around, so the vector state after the last permutation is the first one.
		for( unsigned int j = 0 ; j < n ; ++ j ) {
			the_vector.push_back( j ) ;
		}
	}

	/// Advance the permutation by one.
	void
	operator++()
	{
		if ( at_end ) return ;
		at_end = ! next_permutation( the_vector.begin(), the_vector.end() ) ;
	}

	/// Get a printable name for the permutation
	std::string
	name() const
	{
		std::stringstream s( "" ) ;
		const_iterator j( the_vector.begin() ) ;
		const_iterator k( the_vector.end() ) ;
		while ( j != k ) {
			s << *j ;
			++ j ;
		}
		return s.str() ;
	}

	bool
	operator==( const permutation_iterator & x ) const
	{
		if ( x.at_end ) {
			return at_end ;
		}
		throw std::exception( "Can only compare against end-iterators in the present implementation." ) ;
	}

	/**	\class test_function
	 */
	boost::function< void( void ) > test_function() const
	{
		return boost::bind( f, the_vector.begin(), the_vector.end() ) ;
	}
} ;

class permutation
{
public:
	typedef permutation_iterator iterator ;
	typedef const permutation_iterator const_iterator ;
	typedef permutation_iterator::test_function_type test_function_type ;
private:
	unsigned int n ;

	test_function_type f ;

public:
	permutation( unsigned int n, test_function_type f )
		: n( n ), f( f ) {}

	permutation_iterator begin() const
	{
		return permutation_iterator( n, f ) ;
	}

	permutation_iterator end() const
	{
		return permutation_iterator( n, f, 1 ) ;
	}
} ;


#endif	// end of inclusion protection
