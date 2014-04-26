/* 
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
 *   Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "PrototypeEventListeners.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"
#include "string_table.h"
#include "GnashKey.h" // gnash::key::code

#include "check.h"
#include <string>
#include <cassert>
#include <sstream>

using namespace gnash;
using namespace gnash::geometry;
using namespace std;

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	// for variables lookup (consistency checking)
	VM& vm = getVM(*getObject(root));

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 0);

	// first frame is just Dejagnu clip...
	tester.advance();

	check_equals(root->get_current_frame(), 1);

	tester.pressKey(gnash::key::A); // Should do nothing.

	tester.click();

	tester.click();
	
	tester.pressKey(gnash::key::A);

	// Consistency check !!
	as_value eot;
	// It's an swf6, so lowercase 'ENDOFTEST'
	bool endOfTestFound = getObject(root)->get_member(getURI(vm, "endoftest"), &eot);
	check(endOfTestFound);

	if ( endOfTestFound )
	{
		cerr << eot << endl;
		check_equals(eot.to_bool(8), true);
	}
	else
	{
		cerr << "Didn't find ENDOFTEST... dumping all members" << endl;
		// root->dump_members();
	}

	return 0;
}

