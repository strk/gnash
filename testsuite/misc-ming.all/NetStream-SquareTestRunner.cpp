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

#define INPUT_FILENAME "NetStream-SquareTest.swf"

#include "MovieTester.h"
#include "VM.h"
#include "string_table.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "GnashSleep.h"

#include "check.h"
#include <string>
#include <cassert>
#include "GnashSystemIOHeaders.h"

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::RcInitFile& rc = gnash::RcInitFile::getDefaultInstance();
	rc.addLocalSandboxPath(MEDIADIR);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	MovieClip* root = tester.getRootMovie();
	assert(root);

	if ( ! tester.canTestVideo() )
	{
		cout << "UNTESTED: NetStream video (not supported by this build)."
		     << endl;
		return EXIT_SUCCESS;
	}

	// On NetStatus.Play.Stop we jump to last frame and stop,
	// so this loop is about equivalent to running a
	// generic self-contained test.
	//
	// When all possible tests are implemented as self-contained, we'll
	// add tests that can't be self-contained.
	//
	VM& vm = tester.vm();

	const ObjectURI& k = getURI(vm, "startNotified");
	as_value tmp;
	while (!getObject(root)->get_member(k, &tmp) )
	{
		tester.advance();

		// sleep to give the NetStream a chance to load
        // data and trigger notifications
		// needs more analisys to find a good way for doing this..
        gnashSleep(10000); // 10 milliseconds should be enough for loading
	}

	cout << "Pressing space" << endl;
	tester.pressKey(key::SPACE);
	tester.releaseKey(key::SPACE);

    while (root->get_current_frame() < 2)
	{
		tester.advance();

		// sleep to give the NetStream a chance to
		// load data and trigger notifications
		// needs more analisys to find a good way for doing this..
        gnashSleep(10000); // 10 milliseconds should be enough for loading
	}

	// Consistency check 
	as_value eot;
	bool endOfTestFound = getObject(root)->get_member(getURI(vm, "end_of_test"),
		&eot);
	check(endOfTestFound);

}

