// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

// Linking Gnash statically or dynamically with other modules is making a
// combined work based on Gnash. Thus, the terms and conditions of the GNU
// General Public License cover the whole combination.
//
// As a special exception, the copyright holders of Gnash give you
// permission to combine Gnash with free software programs or libraries
// that are released under the GNU LGPL and with code included in any
// release of Talkback distributed by the Mozilla Foundation. You may
// copy and distribute such a system following the terms of the GNU GPL
// for all but the LGPL-covered parts and Talkback, and following the
// LGPL for the LGPL-covered parts.
//
// Note that people who make modified versions of Gnash are not obligated
// to grant this special exception for their modified versions; it is their
// choice whether to do so. The GNU General Public License gives permission
// to release a modified version without this exception; this exception
// also makes it possible to release a modified version which carries
// forward this exception.
// 

#include "URL.h"
#include "dejagnu.h"

#include <iostream>
#include <sstream>
#include <cassert>

using namespace std;
using namespace gnash;

TestState runtest;

#define check_equals_label(label, expr, expected) \
	{ \
		std::stringstream ss; \
		if ( label != "" ) ss << label << ": "; \
		if ( expr == expected ) \
		{ \
			ss << #expr << " == " << expected; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			runtest.pass(ss.str().c_str()); \
		} \
		else \
		{ \
			ss << #expr << " == '" << expr << "' (expected: " \
				<< expected << ")"; \
			ss << " [" << __FILE__ << ":" << __LINE__ << "]"; \
			runtest.fail(ss.str().c_str()); \
		} \
	}

#define check_equals(expr, expected) check_equals_label("", expr, expected)


int
main(int argc, char** argv)
{

	std::string label;


	/// Test absolute filename
	URL u("/etc/hosts");
	label = "abs fname";
	check_equals_label(label, u.protocol(), "file");
	check_equals_label(label, u.hostname(), "");
	check_equals_label(label, u.path(), "/etc/hosts");
	check_equals_label(label, u.str(), "file://etc/hosts");

	/// Test relative filename
	URL u1("passwd", u);
	label = "rel fname";
	check_equals_label(label, u1.protocol(), "file" );
	check_equals_label(label, u1.hostname(), "" );
	check_equals_label(label, u1.path(), "/etc/passwd" );
	check_equals_label(label, u1.str(), "file://etc/passwd" );

	/// Test proto-host relative filename
	URL u2("/", u);
	label = "rel path";
	check_equals_label (label, u2.protocol() , "file" );
	check_equals_label (label, u2.hostname() , "" );
	check_equals_label (label, u2.path() , "/" );
	check_equals_label (label, u2.str() , "file://" );

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
	check_equals ( "/tmp/curl.h", "/tmp/curl.h" );

}

