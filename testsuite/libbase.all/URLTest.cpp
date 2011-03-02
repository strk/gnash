// 
//   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software
//   Foundation, Inc
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

#ifdef HAVE_CONFIG_H
#include "gnashconfig.h"
#endif

#include "check.h"
#include "URL.h"
#include "log.h"
#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;
using namespace gnash;

int
main(int /*argc*/, char** /*argv*/)
{
	//gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	//dbglogfile.setVerbosity(2);

	std::string label;


	/// Test absolute filename
	URL u("/etc/hosts");
	label = "abs fname";
	check_equals_label(label, u.protocol(), "file");
	check_equals_label(label, u.hostname(), "");
	check_equals_label(label, u.path(), "/etc/hosts");
	check_equals_label(label, u.str(), "file:///etc/hosts");

	/// Test relative filename
	URL u1("passwd", u);
	label = "rel fname";
	check_equals_label(label, u1.protocol(), "file" );
	check_equals_label(label, u1.hostname(), "" );
	check_equals_label(label, u1.path(), "/etc/passwd" );
	check_equals_label(label, u1.str(), "file:///etc/passwd" );

	/// Test proto-host relative filename
	URL u2("/", u);
	label = "rel path";
	check_equals_label (label, u2.protocol() , "file" );
	check_equals_label (label, u2.hostname() , "" );
	check_equals_label (label, u2.path() , "/" );
	check_equals_label (label, u2.str() , "file:///" );

	/// Test https url 
	URL u3("https://www.fake.it/path.swf");

	label = "https url";
	check_equals_label(label, u3.protocol(), "https");
	check_equals_label(label, u3.hostname(), "www.fake.it");
	check_equals_label(label, u3.path(), "/path.swf");
	check_equals_label(label, u3.str(), "https://www.fake.it/path.swf");

	/// Test http url with root path
	URL u4("http://www.fake.it/");
	label = "http root path";
	check_equals_label (label, u4.protocol() , "http" );
	check_equals_label (label, u4.hostname() , "www.fake.it" );
	check_equals_label (label, u4.path() , "/" );
	check_equals_label (label, u4.str() , "http://www.fake.it/" );

	/// Test path-absolute proto-host-relative http url 
	URL u5("/index.html", u4);
	label = "rel root path";
	check_equals_label (label, u5.protocol(), "http" );
	check_equals_label (label, u5.hostname(), "www.fake.it" );
	check_equals_label (label, u5.path(), "/index.html" );
	check_equals_label (label, u5.str(), "http://www.fake.it/index.html");

	/// Test back-seek path
	URL u6("/usr/local/include/curl.h");
	check_equals(u6.protocol() , "file" );
	check_equals( u6.path() , "/usr/local/include/curl.h" );

	URL u7("../../include/curl.h", u6);
	check_equals( u7.protocol() , "file" );
	check_equals( u7.path() , "/usr/include/curl.h" );

	URL u8("../..//../../../../tmp//curl.h", u6);
	check_equals ( u8.protocol() , "file" );
	check_equals ( u8.path() , "/tmp/curl.h" );

	/// Test path normalization 
	check_equals (URL("/hello/world/../file").path(), "/hello/file");
	check_equals (URL("/dir/./file").path(), "/dir/file");
	check_equals (URL("/dir/./1/2/3/../../../...file").path(), "/dir/...file");

	/// Test url with anchors
	URL u9("/the/path#the_anchor");
	check_equals (u9.path(), "/the/path");
	check_equals (u9.anchor(), "the_anchor");
	check_equals (u9.str(), "file:///the/path#the_anchor");
	URL u10("http://host/the/path#the_anchor");
	check_equals (u10.hostname(), "host");
	check_equals (u10.path(), "/the/path");
	check_equals (u10.anchor(), "the_anchor");
	check_equals (u10.str(), "http://host/the/path#the_anchor");
	URL u11("#another_anchor", u10);
	check_equals (u11.hostname(), "host");
	check_equals (u11.path(), "/the/path");
	check_equals (u11.anchor(), "another_anchor");
	check_equals (u11.str(), "http://host/the/path#another_anchor");
	URL u12("#", u10);
	check_equals (u12.hostname(), "host");
	check_equals (u12.path(), "/the/path");
	check_equals (u12.anchor(), "");
	check_equals (u12.str(), "http://host/the/path");

	/// Test url with QUERY STRING
	URL u13("http://localhost/?M=A");
	check_equals (u13.hostname(), "localhost");
	check_equals (u13.path(), "/");
	check_equals (u13.protocol(), "http");
	check_equals (u13.anchor(), "");
	check_equals (u13.querystring(), "M=A");
	check_equals (u13.str(), "http://localhost/?M=A");
	URL u14("/?M=A&C=D");
	check_equals (u14.querystring(), "M=A&C=D");
	check_equals (u14.str(), "file:///?M=A&C=D");
	URL u15("/?M=A&C=D#anchor");
	check_equals (u15.querystring(), "M=A&C=D");
	check_equals (u15.anchor(), "anchor");
	check_equals (u15.str(), "file:///?M=A&C=D#anchor");
	URL u16("/my/path/?option1=23&option2=65#anchor");
	check_equals (u16.querystring(), "option1=23&option2=65");
	check_equals (u16.anchor(), "anchor");
	check_equals (u16.str(), "file:///my/path/?option1=23&option2=65#anchor");
	URL u17("/test?.swf");
	check_equals (u17.protocol() , "file" );
	check_equals (u17.hostname() , "" );
	check_equals (u17.path() , "/test" );
	check_equals (u17.str() , "file:///test?.swf" );
	check_equals (u17.querystring() , ".swf" );

	// Test that this doesn't crash.
	URL u18("file:///loadMovieTest.swf");
	URL u19("file://../../test.swf", u18);

	// Test query_string parsing
	map<string, string> qs;
	URL::parse_querystring(u13.querystring(), qs);
	check_equals (qs["M"], "A");
	check_equals (qs["C"], "");
	URL::parse_querystring(u14.querystring(), qs);
	check_equals (qs["M"], "A");
	check_equals (qs["C"], "D");
	URL::parse_querystring(u16.querystring(), qs);
	check_equals (qs["option1"], "23");
	check_equals (qs["option2"], "65");

	qs.clear();
	URL::parse_querystring("\n&inurl_check=3", qs);
	check_equals(qs.size(), 2);
	check_equals(qs["inurl_check"], "3");
	check(qs.find("\n") != qs.end());
	check(qs.find("\n&inurl_check") == qs.end());

	// Test query string with embedded path to an .swf 
	// Broken by:
	// htp://cvs.savannah.gnu.org/viewvc/gnash/libbase/URL.cpp?root=gnash&r1=1.26&r2=1.27
	//
	URL u20("http://www.gnu.org/~gnash/movie.swf?arg1=600&arg2=path/to/file.swf&arg3=320x200");
	check_equals(u20.protocol(), "http");
	check_equals(u20.hostname(), "www.gnu.org");
	check_equals(u20.path(), "/~gnash/movie.swf");
	check_equals(u20.querystring(), "arg1=600&arg2=path/to/file.swf&arg3=320x200");
	qs.clear();
	URL::parse_querystring(u20.querystring(), qs);
	check_equals(qs["arg1"], "600");
	check_equals(qs["arg2"], "path/to/file.swf");
	check_equals(qs["arg3"], "320x200");

	// Test relative resolution when query string contains slashes
	URL u21("movie2.swf", u20);
	check_equals (u21.protocol(), "http");
	check_equals (u21.querystring(), "");
	check_equals (u21.path(), "/~gnash/movie2.swf");
	check_equals (u21.hostname(), "www.gnu.org");

	// Test url with mixed query string and anchor
	URL u22("http://localhost/?query#anchor"); // simple case
	check_equals (u22.querystring(), "query");
	check_equals (u22.anchor(), "anchor");
	check_equals (u22.hostname(), "localhost");
	check_equals (u22.path(), "/");
	URL u23("http://localhost/?query#questions?"); 
	check_equals (u23.querystring(), "query");
	check_equals (u23.anchor(), "questions?");
	check_equals (u23.hostname(), "localhost");
	check_equals (u23.path(), "/");
	URL u24("http://localhost/?query#questions?yes"); 
	check_equals (u24.querystring(), "query");
	check_equals (u24.anchor(), "questions?yes");
	check_equals (u24.hostname(), "localhost");
	check_equals (u24.path(), "/");
	URL u25("http://localhost/#anchor?query");
	check_equals (u25.querystring(), "");
	check_equals (u25.anchor(), "anchor?query");
	check_equals (u25.hostname(), "localhost");
	check_equals (u25.path(), "/");
	URL u26("http://localhost/?query1?query2?query3");
	check_equals (u26.querystring(), "query1?query2?query3");
	check_equals (u26.anchor(), "");
	check_equals (u26.hostname(), "localhost");
	check_equals (u26.path(), "/");
	URL u27("http://localhost/?query1?query2#anchor?query3");
	check_equals (u27.querystring(), "query1?query2");
	check_equals (u27.anchor(), "anchor?query3");
	check_equals (u27.hostname(), "localhost");
	check_equals (u27.path(), "/");

    URL u28("rtmp://pms.youtube.com:443/");
    check_equals (u28.protocol(), "rtmp");
    check_equals (u28.hostname(), "pms.youtube.com");
    check_equals (u28.port(), "443");
    check_equals (u28.path(), "/");

    URL u29("rtmp://pms.youtube.com:443");
    check_equals (u29.protocol(), "rtmp");
    check_equals (u29.hostname(), "pms.youtube.com");
    check_equals (u29.port(), "443");
    check_equals (u29.path(), "/");

    { // Relative url when base url has a port number
       URL u("rtmp://pms.youtube.com:443");
       URL u2("/newpath", u);
       check_equals (u2.protocol(), "rtmp");
       check_equals (u2.hostname(), "pms.youtube.com");
       check_equals (u2.port(), "443");
       check_equals (u2.path(), "/newpath");
    }

    { // Relative url with port number 
       URL u("rtmp://pms.youtube.com:443");
       URL u2("http://newhost:554", u);
       check_equals (u2.protocol(), "http");
       check_equals (u2.hostname(), "newhost");
       check_equals (u2.port(), "554");
       check_equals (u2.path(), "/");
    }

    { // Relative url with query string
       URL u("http://pms.youtube.com:443/index.php?query=1");
       URL u2("/?query", u);
       check_equals (u2.protocol(), "http");
       check_equals (u2.hostname(), "pms.youtube.com");
       check_equals (u2.port(), "443");
       check_equals (u2.path(), "/");
       check_equals (u2.querystring(), "query");
       check_equals (u2.anchor(), "");
    }

    { // Relative url with anchor
       URL u("http://pms.youtube.com:443/index.php?query=1");
       URL u2("/#anchor", u);
       check_equals (u2.protocol(), "http");
       check_equals (u2.hostname(), "pms.youtube.com");
       check_equals (u2.port(), "443");
       check_equals (u2.path(), "/");
       check_equals (u2.querystring(), "");
       check_equals (u2.anchor(), "anchor");
    }

    { // HTTP URL with url in query string
       URL u("http://anurl.com/tmp/easy.swf?url=http://url.it/there");
       check_equals (u.protocol(), "http");
       check_equals (u.hostname(), "anurl.com");
       check_equals (u.port(), "");
       check_equals (u.path(), "/tmp/easy.swf");
       check_equals (u.querystring(), "url=http://url.it/there");
       check_equals (u.anchor(), "");
    }

    { // File URL with url in query string (see bug #32625)
       URL u("/tmp/easy.swf?url=http://url.it/there");
       xcheck_equals (u.protocol(), "file");
       xcheck_equals (u.hostname(), "");
       check_equals (u.port(), "");
       xcheck_equals (u.path(), "/tmp/easy.swf");
       xcheck_equals (u.querystring(), "url=http://url.it/there");
       check_equals (u.anchor(), "");
    }

    bool threw = false;
    try
    { // pseudo-url from Mozilla
        URL u("about:blank");
    }
    catch (const std::exception& e)
    {
       threw = true;
    }
    check(threw);


	// TODO: Samba paths
}

