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

#include "Random_Permutation.hpp"

/// Ordinary constructor
random_permutation_iterator::
random_permutation_iterator( const random_permutation * owner, test_function_type f, unsigned int set_size, unsigned int number_of_iterations )
	: owner( owner ), f( f ), set_size( set_size ), number_of_iterations( number_of_iterations ),
	generator( engine_type( 39650193 ), distribution_type( 0, set_size ) )
{
	for ( unsigned int j = 0 ; j < set_size ; ++ j ) {
		the_vector.push_back( j ) ;
	}
	randomize() ;
}

void
random_permutation_iterator::
operator++()
{
	randomize() ;
	++ number_of_iterations ;
}

void
random_permutation_iterator::
randomize()
{
	std::random_shuffle( the_vector.begin(), the_vector.end() ) ;
}

bool
random_permutation_iterator::
operator==( const random_permutation_iterator & x ) const
{
	return owner == x.owner
		&& number_of_iterations == x.number_of_iterations ;
}

std::string 
random_permutation_iterator::
name() const
{
	std::stringstream s( "__" ) ;
	const_iterator j( the_vector.begin() ) ;
	const_iterator k( the_vector.end() ) ;
	while ( j != k ) {
		s << *j << "_" ;
		++ j ;
	}
	s << "_" ;
	return s.str() ;
}
