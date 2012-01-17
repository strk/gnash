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

#define INPUT_FILENAME "intervalTest.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"

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

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	as_value tmp;

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 0);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_PLAY);

	tester.advanceClock(50); // "sleep" 50 milliseconds
	tester.advance(); // execute actions in second frame frame

	check_equals(root->get_current_frame(), 1);
	check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);

	// Now timers are set and counters initialized

	VM& vm = tester.vm();
	const ObjectURI& thisCounter = getURI(vm, "this_counter");
	const ObjectURI& thatCounter = getURI(vm, "that_counter");
	const ObjectURI& pushedArgs = getURI(vm, "pushed_args");
	const ObjectURI& testCompleted = getURI(vm, "test_completed");

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 0);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 0);

	tester.advanceClock(500); // "sleep" 500 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 1);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 0);

	tester.advanceClock(600); // "sleep" 500 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 2);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 1);

	tester.advanceClock(500); // "sleep" 500 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 3);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 1);

	tester.advanceClock(520); // "sleep" 520 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 4);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 2);

	tester.advanceClock(1020); // "sleep" 1020 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 4);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 3);

	tester.advanceClock(1020); // "sleep" 1020 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 4);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 4);

	tester.advanceClock(520); // "sleep" 520 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(thisCounter, &tmp);
	check_equals(toNumber(tmp, vm), 5);
	getObject(root)->get_member(thatCounter, &tmp);
	check_equals(toNumber(tmp, vm), 4);

	getObject(root)->get_member(pushedArgs, &tmp);
	as_environment env(vm); // needed for proper to_string()
	check_equals(tmp.to_string(), std::string("8,9,10"));

	tester.advanceClock(100); // "sleep" another 100 milliseconds
	tester.advance(); // run expired timers

	getObject(root)->get_member(testCompleted, &tmp);
	check_equals(toNumber(tmp, vm), 1);

}

