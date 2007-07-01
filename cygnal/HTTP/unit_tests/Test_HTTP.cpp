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

/// \file Test_HTTP.cpp

#include <boost/test/auto_unit_test.hpp>

#include "../HTTP_Parse.hpp"
#include "HTTP/HTTP_Behavior.hpp"
#include "IO/test_support/String_Device.hpp"

using namespace cygnal ;
using namespace cygnal::HTTP ;
using std::string ;


//-------------------------
// Character class table defined in HTPP_Behavior.cpp
namespace cygnal { namespace HTTP {
	extern character_class ch_class[ 256 ] ;
}}

BOOST_AUTO_TEST_CASE( Character_Classes )
{
	unsigned int counts[ n_character_classes ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } ;

	unsigned int j ;
	for ( j = 0 ; j <= 255 ; ++ j ) {
		++ counts[ ch_class[ j ] ] ;
	}
	// This sum verifies that all character classes are in the right numeric range.
	unsigned int count = 0 ;
	for ( j = 0 ; j < n_character_classes ; ++ j ) {
		BOOST_CHECK( 0 <= j && j < n_character_classes ) ;
		count += counts[ j ] ;
	}
	BOOST_CHECK( count == 256 ) ;

	BOOST_CHECK( counts[ ctl ] == 30 ) ;
	BOOST_CHECK( counts[ CR ] == 1 ) ;
	BOOST_CHECK( counts[ LF ] == 1 ) ;
	BOOST_CHECK( counts[ HT ] == 1 ) ;
	BOOST_CHECK( counts[ sep ] == 14 ) ;
	BOOST_CHECK( counts[ SP ] == 1 ) ;
	BOOST_CHECK( counts[ cln ] == 1 ) ;
	BOOST_CHECK( counts[ DQ ] == 1 ) ;
	BOOST_CHECK( counts[ bsl ] == 1 ) ;
	BOOST_CHECK( counts[ txt ] == 205 ) ;

	BOOST_CHECK( ch_class[ '(' ] == sep ) ;
	BOOST_CHECK( ch_class[ ')' ] == sep ) ;
	BOOST_CHECK( ch_class[ '<' ] == sep ) ;
	BOOST_CHECK( ch_class[ '>' ] == sep ) ;
	BOOST_CHECK( ch_class[ '@' ] == sep ) ;
	BOOST_CHECK( ch_class[ ',' ] == sep ) ;
	BOOST_CHECK( ch_class[ ';' ] == sep ) ;
	BOOST_CHECK( ch_class[ '/' ] == sep ) ;
	BOOST_CHECK( ch_class[ '[' ] == sep ) ;
	BOOST_CHECK( ch_class[ ']' ] == sep ) ;
	BOOST_CHECK( ch_class[ '?' ] == sep ) ;
	BOOST_CHECK( ch_class[ '=' ] == sep ) ;
	BOOST_CHECK( ch_class[ '{' ] == sep ) ;
	BOOST_CHECK( ch_class[ '}' ] == sep ) ;

	BOOST_CHECK( ch_class[ '\r' ] == CR ) ;
	BOOST_CHECK( ch_class[ '\n' ] == LF ) ;
	BOOST_CHECK( ch_class[ '\t' ] == HT ) ;
	BOOST_CHECK( ch_class[ ' ' ] == SP ) ;
	BOOST_CHECK( ch_class[ ':' ] == cln ) ;
	BOOST_CHECK( ch_class[ '\"' ] == DQ ) ;
	BOOST_CHECK( ch_class[ '\\' ] == bsl ) ;

	for ( j = 0 ; j < 32 ; ++ j ) {
		if ( j != '\r' && j != '\n' && j != '\t' ) {
			BOOST_CHECK( ch_class[ j ] == ctl ) ;
		}
	}
	BOOST_CHECK( ch_class[ 127 ] == ctl ) ;
}

//BOOST_AUTO_TEST_CASE( Character_Categories ) {}


//---------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE( Single_Header_Ordinary_1 )
{
	static const char s[] = "Name: Value\r\n" ;
	IO::String_Device device( s ) ;
	IO::Source * source( & device ) ;

	cygnal::HTTP::read_single_header act( source ) ;
	act( 0 ) ;
	BOOST_CHECK( act.completed() ) ;
	BOOST_CHECK( act.result_message_header_size() == strlen( s ) - 2 ) ;
	IO::buffer x( act.result_field_name() ) ;
	BOOST_CHECK( x.size() == 4 ) ;

	BOOST_REQUIRE( x.begin().get() != 0 ) ;
	BOOST_CHECK( 0 == strncmp( "Name", x.begin().get(), x.size() ) ) ;
}

BOOST_AUTO_TEST_CASE( Single_Header_Ordinary_2 )
{
	static const char s[] = "Name: Value\r\n more\r\n" ;
	IO::String_Device device( s ) ;
	IO::Source * source( & device ) ;

	cygnal::HTTP::read_single_header act( source ) ;
	act( 0 ) ;
	BOOST_CHECK( act.completed() ) ;
	BOOST_CHECK_MESSAGE( act.result_message_header_size() == strlen( s ) - 2,
		"Read " << act.result_message_header_size() << " characters.  Expected to read " << strlen( s ) - 2 << " characters." ) ;
	IO::buffer x( act.result_field_name() ) ;
	BOOST_CHECK( x.size() == 4 ) ;
	BOOST_REQUIRE( x.begin().get() != 0 ) ;
	BOOST_CHECK( 0 == strncmp( "Name", x.begin().get(), x.size() ) ) ;
}

BOOST_AUTO_TEST_CASE( Single_Header_Abnormal_1 )
{
	IO::String_Device device( ":" ) ;
	IO::Source * source( & device ) ;

	cygnal::HTTP::read_single_header act( source ) ;
	act( 0 ) ;
	BOOST_CHECK( act.bad() ) ;
}

//---------------------------------------------------------------------------
// read_message_headers_action

void check_read_message_headers( const char * msg ) {
	IO::String_Device device( msg ) ;
	IO::Source * source( & device ) ;

	read_message_headers_action action( source ) ;
	action( 0 ) ;
	BOOST_CHECK( ! action.bad() ) ;
	BOOST_CHECK( action.completed() ) ;
}

BOOST_AUTO_TEST_CASE( Read_Message_Headers_1 )
{
	static const char basic_headers_1[] =
		"Name: Value\r\n"
		"\r\n" ;
	check_read_message_headers( basic_headers_1 ) ;
}

BOOST_AUTO_TEST_CASE( Read_Message_Headers_2 )
{
	static const char basic_headers_2[] =
		"Name2: Value\r\n"
		"Name3: Value\r\n"
		"   Value\r\n"
		"\r\n" ;
	check_read_message_headers( basic_headers_2 ) ;
}

//---------------------------------------------------------------------------
void verify_result( const string input, IO::buffer result, const std::string expected, size_t expected_length )
{
	if ( expected_length == 0 ) {
		return ;
	}

	char * result_begin = result.begin().get() ;
	size_t result_length = result.size() ;

	if ( result_begin == 0 ) {
		BOOST_CHECK_MESSAGE( result_begin != 0, 
			"Found null pointer result. Expected result \"" << expected << "\"."
			<< " Input is \"" << input << "\"."
			) ;
	} else {
		BOOST_CHECK_MESSAGE( 0 == strncmp( result_begin, expected.c_str(), expected_length ),
			"Expected \"" << expected << "\" for " << expected_length << " characters.  Found \"" << string( result_begin, expected_length ) << "\"."
			<< " Input is \"" << input << "\"."
			) ;
	}
	BOOST_CHECK_MESSAGE( result_length == expected_length,
		"Expected result length " << expected_length << ". Found length " << result_length << "." 
		<< " The result expected is \"" << expected << "\"."
		<< " Input is \"" << input << "\"."
		) ;
}

void verify_result( const string input, IO::buffer result, const std::string expected )
{
	verify_result( input, result, expected, expected.length() ) ;
}

//---------------------------------------------------------------------------
bool check_valid_URI
	( const string input, size_t input_length, bool flag_asterisk, 
	bool use_scheme = false, const string scheme = "", 
	bool use_authority = false, const string authority = "", 
	bool use_path = false, const string path = "", 
	bool use_query = false, const string query = "", 
	bool use_fragment = false, const string fragment = "" )
{
	IO::String_Device device( input ) ;
	URI_Scanner action( & device ) ;
	action( 0 ) ;
	if ( action.bad() ) {
		BOOST_CHECK_MESSAGE( ! action.bad(), "Parse failed on input \"" << input << "\"." ) ;
		return false ;
	}
	BOOST_CHECK( action.completed() ) ;
	Request_URI_Data uri = action.result_uri() ;

	BOOST_CHECK( uri.is_asterisk() == flag_asterisk ) ;

	verify_result( input, uri.result(), input, input_length ) ;
	verify_result( input, uri.result_scheme(), scheme ) ;
	verify_result( input, uri.result_authority(), authority ) ;
	verify_result( input, uri.result_path(), path ) ;
	verify_result( input, uri.result_query(), query ) ;
	verify_result( input, uri.result_fragment(), fragment ) ;

	return true ;
}

BOOST_AUTO_TEST_CASE( Good_URI_direct )
{
	check_valid_URI( "*", 1, true ) ;
}

//---------------------------------------------------------------------------
// Multiplier: 2
//		test: with space as terminator
//		test: with EOF as terminator
void check_valid_URI
	( const string scheme, const string authority, const string path = "", 
	bool use_query = false, const string query = "", 
	bool use_fragment = false, const string fragment = "" )
{
	std::string input( "" ) ;
	if ( scheme.length() > 0 ) {
		input += scheme ;
		input += ":" ;
	}
	if ( authority.length() > 0 ) {
		input += "//" ;
		input += authority ;
	}
	input += path ;
	if ( use_query ) {
		input += "?" ;
		input += query ;
	}
	if ( use_fragment ) {
		input += "#" ;
		input += fragment ;
	}
	size_t input_length = input.length() ;

	// Check again with terminator being an internal character.
	bool x = check_valid_URI( input + " ", input_length, false, 
		true, scheme, 
		true, authority, 
		true, path, 
		use_query, query, 
		use_fragment, fragment ) ;
	// Don't try again if parse failed.
	if ( ! x ) return ;

	// Check with EOF as terminator
	check_valid_URI( input, input_length, false, 
		true, scheme, 
		true, authority, 
		true, path, 
		use_query, query, 
		use_fragment, fragment ) ;
}

//---------------------------------------------------------------------------
// Local Multiplier: 5
//		test: no fragment
//		test: "#" alone
//		test: "#frag-one"
//		test: "#frag-one#"
//		test: "#frag-one#frag-two"
// Total Multiplier: 10
void check_valid_URI_from_query( 
	const string scheme, const string authority, const string path, 
	bool use_query, string query = ""
	)
{
	check_valid_URI( scheme, authority, path, use_query, query, false ) ;
	string fragment( "" ) ;
	check_valid_URI( scheme, authority, path, use_query, query, true, fragment ) ;
	fragment += "frag-one" ;
	check_valid_URI( scheme, authority, path, use_query, query, true, fragment ) ;
	fragment += "#" ;
	check_valid_URI( scheme, authority, path, use_query, query, true, fragment ) ;
	fragment += "frag-two" ;
	check_valid_URI( scheme, authority, path, use_query, query, true, fragment ) ;
}

// Local Multiplier: 3
// Total Multiplier: 30
void check_valid_URI_from_path_nz( const string scheme, const string authority, const string path )
{
	string query( "" ) ;
	check_valid_URI_from_query( scheme, authority, path, true, query ) ;
	query += "x=1" ;
	check_valid_URI_from_query( scheme, authority, path, true, query ) ;
	query += "?y=2" ;
	check_valid_URI_from_query( scheme, authority, path, true, query ) ;
}

// Local Multiplier: 4
// Total Multiplier: 40
void check_valid_URI_from_path( const string scheme, const string authority, const string path )
{
	check_valid_URI_from_query( scheme, authority, path, false ) ;
	check_valid_URI_from_path_nz( scheme, authority, path ) ;
}

// Local Multiplier: 5
// Total Multiplier: 200
void check_valid_URI_from_authority_nz( const string scheme, const string authority )
{
	string path( "/" ) ;
	check_valid_URI_from_path( scheme, authority, path ) ;
	path += "a" ;
	check_valid_URI_from_path( scheme, authority, path ) ;
	path += "/" ;
	check_valid_URI_from_path( scheme, authority, path ) ;
	path += "b" ;
	check_valid_URI_from_path( scheme, authority, path ) ;
	path += "/" ;
	check_valid_URI_from_path( scheme, authority, path ) ;
}

// Local Multiplier: 6
// Total Multiplier: 240
void check_valid_URI_from_authority( const string scheme, const string authority )
{
	string path( "" ) ;
	check_valid_URI_from_path( scheme, authority, "" ) ;
	check_valid_URI_from_authority_nz( scheme, authority ) ;
}

// Local Multiplier: 4
// Total Multiplier: 960
void check_valid_URI_from_scheme_nz( const string scheme )
{
	check_valid_URI_from_authority( scheme, "foo.com" ) ;
	check_valid_URI_from_authority( scheme, "foo.com:80" ) ;
	check_valid_URI_from_authority( scheme, "user@foo.com" ) ;
	check_valid_URI_from_authority( scheme, "user@foo.com:80" ) ;
}

// Local Multiplier: 5
// Total Multiplier: 1200
void check_valid_URI_from_scheme( const string scheme )
{
	check_valid_URI_from_authority( scheme, "" ) ;
	check_valid_URI_from_scheme_nz( scheme ) ;
}

BOOST_AUTO_TEST_CASE( Good_URI )
{	
	check_valid_URI_from_scheme( "http" ) ;			// 1200 test vectors
	check_valid_URI_from_scheme_nz( "" ) ;			// 960 test vectors
	check_valid_URI_from_authority_nz( "", "" ) ;	// 200 test vectors
	check_valid_URI_from_path_nz( "", "", "" ) ;	// 30 test vectors
}

/*
 * [EH] I'd like there to be an equally exhaustive Bad_URI test suite.
 * That would mean, ideally, adding error reporting so that the parser could give us an initially valid prefix.
 * Each test would then not only verify that a URI candidate was bad, but that it went bad in the expected location.
 */

//---------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE( Request_1 )
{
	const char our_request[] =
		"GET"
		" "
		"/software/gnash/tests/flvplayer2.swf?file=http://localhost:4080/software/gnash/tests/lulutest.flv"
		" "
		"HTTP/1.1" 
		"\r\n" ;
	const char the_method[] = "GET" ;
	const char the_request_URI[] = "/software/gnash/tests/flvplayer2.swf?file=http://localhost:4080/software/gnash/tests/lulutest.flv" ;
	const char the_HTTP_version[] = "HTTP/1.1" ;

	IO::String_Device device( our_request ) ;
	Request_Scanner action( & device ) ;
	action( 0 ) ;
	BOOST_REQUIRE( ! action.bad() ) ;
	BOOST_CHECK_MESSAGE( action.completed(), "Action is still working but should have finished." ) ;
	verify_result( our_request, action.method(), the_method ) ;
	verify_result( our_request, action.request_URI(), the_request_URI ) ;
	verify_result( our_request, action.HTTP_version(), the_HTTP_version ) ;
	BOOST_CHECK( action.HTTP_version_major_number() == 1 ) ;
	BOOST_CHECK( action.HTTP_version_minor_number() == 1 ) ;
}

static const char basic_request[] = 
	"GET /software/gnash/tests/flvplayer2.swf?file=http://localhost:4080/software/gnash/tests/lulutest.flv HTTP/1.1" "\r\n"
	"User-Agent: Opera/9.01 (X11; Linux i686; U; en)" "\r\n"
	"Host: localhost:4080" "\r\n"
	"Accept: text/html, application/xml;q=0.9, application/xhtml+xml, image/png, image/jpeg, image/gif, image/x-xbitmap, */*;q=0.1" "\r\n"
	"Accept-Language: en" "\r\n"
	"Accept-Charset: iso-8859-1, utf-8, utf-16, *;q=0.1" "\r\n"
	"Accept-Encoding: deflate, gzip, x-gzip, identity, *;q=0" "\r\n"
	"Referer: http://localhost/software/gnash/tests/" "\r\n"
	"Connection: Keep-Alive, TE" "\r\n"
	"TE: deflate, gzip, chunked, identity, trailers" "\r\n"
	 "\r\n" ;

//---------------------------------------------------------------------------
void check_bad_request( const char * x )
{
	IO::String_Device device( x ) ;
	Request_Scanner action( & device ) ;
	action( 0 ) ;
	BOOST_CHECK_MESSAGE( action.bad(), 
		( action.completed() ? "Parse unexpectedly completed." : "Parse did not finish." )
		<< " Input = \"" << x << "\"." 
	) ;
}

BOOST_AUTO_TEST_CASE( Bad_Requests )
{
	check_bad_request( "" ) ;		// Nothing is not a valid request
	check_bad_request( "\r\n" ) ;	// The empty line is not a valid request
	check_bad_request( "GET\r\n" ) ;	// A single word is not a valid request
	check_bad_request( "GET \r\n" ) ;	// A single word with a space is not a valid request
	check_bad_request( "GET *\r\n" ) ;		// Two words is not a valid request
	check_bad_request( "GET * \r\n" ) ;		// Two words and a space is not a valid request
	check_bad_request( " *\r\n" ) ;			// A missing initial words is not a valid request
	check_bad_request( "GET  HTTP/1.1\r\n" ) ;		// An initial word and a missing second one is not a valid request
	check_bad_request( "GET * HTTP/1.1 \r\n" ) ;	// Three words and a space is not a valid request
}

//---------------------------------------------------------------------------
BOOST_AUTO_TEST_CASE( Invalid_Version_Request )
{
	char request[] = "GET * HTTP/1.0\r\n" ;
	IO::String_Device device( request ) ;
	char response[] = 
		"HTTP/1.1 505 HTTP Version Not Supported" "\r\n"
		"Connection: close" "\r\n" ;

	cygnal::HTTP::HTTP_Behavior behavior( & device ) ;
	behavior() ;
	BOOST_CHECK( behavior.completed() ) ;
	BOOST_CHECK( device.string_written() == string( response ) ) ;
}