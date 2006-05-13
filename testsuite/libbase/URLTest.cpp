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
#include <cassert>

using namespace std;
using namespace gnash;

TestState runtest;

int
main(int argc, char** argv)
{

	/// Test absolute filename
	URL u("/etc/hosts");
	if ( u.protocol() == "file" ) {
		runtest.pass ("filename proto");
	} else {
		runtest.fail ("filename proto");
	}
	if ( u.hostname() == "" ) {
		runtest.pass ("filename hostname");
	} else {
		runtest.fail ("filename hostname");
	}
	if ( u.path() == "/etc/hosts" ) {
		runtest.pass ("filename path");
	} else {
		runtest.fail ("filename path");
	}
	if ( u.str() == "file://etc/hosts" ) {
		runtest.pass ("filename str");
	} else {
		std::cerr << "Expected 'file://etc/hosts', obtained " << u.str() << std::endl;
		runtest.fail ("filename str");
	}

	/// Test relative filename
	URL u1("passwd", u);
	if ( u1.protocol() == "file" ) {
		runtest.pass ("rel filename proto");
	} else {
		runtest.fail ("rel filename proto");
	}
	if ( u1.hostname() == "" ) {
		runtest.pass ("rel filename hostname");
	} else {
		runtest.fail ("rel filename hostname");
	}
	if ( u1.path() == "/etc/passwd" ) {
		runtest.pass ("rel filename path");
	} else {
		std::cerr << "Expected '/etc/passwd', obtained " << u1.path() << std::endl;
		runtest.fail ("rel filename path");
	}
	if ( u1.str() == "file://etc/passwd" ) {
		runtest.pass ("filename str");
	} else {
		std::cerr << "Expected 'file://etc/passwd', obtained " << u1.str() << std::endl;
		runtest.fail ("filename str");
	}

	/// Test proto-host relative filename
	URL u2("/", u);
	if ( u2.protocol() == "file" ) {
		runtest.pass ("proto-host rel filename proto");
	} else {
		runtest.fail ("proto-host rel filename proto");
	}
	if ( u2.hostname() == "" ) {
		runtest.pass ("proto-host rel filename hostname");
	} else {
		runtest.fail ("proto-host rel filename hostname");
	}
	if ( u2.path() == "/" ) {
		runtest.pass ("proto-host rel filename path");
	} else {
		std::cerr << "Expected '/', obtained " << u2.path() << std::endl;
		runtest.fail ("proto-host rel filename path");
	}
	if ( u2.str() == "file://" ) {
		runtest.pass ("proto-host filename str");
	} else {
		std::cerr << "Expected 'file://', obtained " << u2.str() << std::endl;
		runtest.fail ("proto-host filename str");
	}

	/// Test https url (root path)
	URL u3("https://www.fake.it/path.swf");
	if ( u3.protocol() == "https" ) {
		runtest.pass ("https url proto");
	} else {
		runtest.fail ("https url proto");
		std::cerr << "obtained: " << u3.protocol();
	}
	if ( u3.hostname() == "www.fake.it" ) {
		runtest.pass ("https url hostname");
	} else {
		runtest.fail ("https url hostname");
	}
	if ( u3.path() == "/path.swf" ) {
		runtest.pass ("https url path");
	} else {
		std::cerr << "Expected '/path.swf', obtained " << u3.path() << std::endl;
		runtest.fail ("https url path");
	}
	if ( u3.str() == "https://www.fake.it/path.swf" ) {
		runtest.pass ("https url str");
	} else {
		std::cerr << "Expected 'https://www.fake.it/path.swf', obtained " << u3.str() << std::endl;
		runtest.fail ("https url str");
	}
}

