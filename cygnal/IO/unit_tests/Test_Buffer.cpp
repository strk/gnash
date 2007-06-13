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

/// \file Test_Buffer.cpp

#include <boost/test/auto_unit_test.hpp>
#include "../Buffer.hpp"
#include <boost/shared_ptr.hpp>
using boost::shared_ptr ;

using namespace IO ;

//---------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE( Buffer_Iterator_1 )
{
	char ch ;
	char storage[ 100 ] = "abcdefghijklmnopqrstuvwxyz" ;
	result_buffer< char > buf( storage, 100 ) ;
	result_buffer< char >::character_iterator p( buf.begin() ) ;
	BOOST_CHECK( *p == 'a' ) ;
	++p ;
	ch = *p++ ;
	BOOST_CHECK_MESSAGE( ch == 'b', "Saw character '" << ch << "'. Expected to see character 'b'." ) ;
	BOOST_CHECK( *p == 'c' ) ;
	BOOST_CHECK( *++p == 'd' ) ;
}

//---------------------------------------------------------------------------
// Tests for operator< (less than)
//---------------------------------------------------------------------------
char *test_strings[] = {
	"Abe", "Abel", "Abelard", "Base"
} ;
const unsigned int n_strings = sizeof( test_strings ) / sizeof( char * ) ;
const unsigned int n_buffer_types = 1 ;
char *buffer_name[] = {
	"result_buffer"
} ;

typedef shared_ptr< Abstract_Buffer< char > > btype ;

btype make_buffer( char * s, size_t n, unsigned int buffer_type_index )
{
	switch ( buffer_type_index ) {
		case 0:
			return btype( new result_buffer<>( s, n ) ) ;
		default:
			throw std::exception( "Bad buffer type index" ) ;
	}
}

BOOST_AUTO_TEST_CASE( Buffer_Comparison_1 )
{
	for ( int j1 = 0 ; j1 < n_strings - 1 ; ++ j1 ) {
		char * jptr = test_strings[ j1 ] ;
		size_t jlen = strlen( jptr ) ;
		int j2 = 0 ;
		btype bj( make_buffer( jptr, jlen, j2 ) ) ;
		for ( int k1 = 0 ; k1 < n_strings ; ++ k1 ) {
			char * kptr = test_strings[ k1 ] ;
			size_t klen = strlen( kptr ) ;
			int k2 = 0 ;
			btype bk( make_buffer( kptr, klen, j2 ) ) ;
			if ( j1 < k1 ) {
				BOOST_CHECK_MESSAGE( * bj < * bk,
					"Expected " << buffer_name[ j2 ] << "( \"" << jptr << "\", " << jlen << " ) to be less than "
					<< buffer_name[ k2 ] << "( \"" << kptr << "\", " << klen << " )."
					) ;
			} else if ( j1 == k1 ) {
				BOOST_CHECK_MESSAGE( * bj == * bk,
					"Expected " << buffer_name[ j2 ] << "( \"" << jptr << "\", " << jlen << " ) to be equal to "
					<< buffer_name[ k2 ] << "( \"" << kptr << "\", " << klen << " )."
					) ;
			} else {
				BOOST_CHECK_MESSAGE( ! ( * bj < * bk ),
					"Expected " << buffer_name[ j2 ] << "( \"" << jptr << "\", " << jlen << " ) to be not less than "
					<< buffer_name[ k2 ] << "( \"" << kptr << "\", " << klen << " )."
					) ;
			}
		}
	}
}

//---------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE( Buffer_Comparison_2 )
{
	E_AV_Buffer< 1 > x1 ;
	E_AV_Buffer< 2 > x2 ;

	char first_half[] = "12" ;
	char second_half[] = "345" ;
	char whole[] = "12345" ;

	x1.append( whole, 5 ) ;
	x2.append( first_half, 2 ) ;
	x2.append( second_half, 3 ) ;
	BOOST_CHECK( x1 == x2 ) ;
}
