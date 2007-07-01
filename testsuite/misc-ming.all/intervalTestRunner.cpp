/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>
#include <unistd.h>

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

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 2);
	check_equals(root->get_current_frame(), 0);
	check_equals(root->get_play_state(), sprite_instance::PLAY);

	tester.advance(); // execute actions in second frame frame

	check_equals(root->get_current_frame(), 1);
	check_equals(root->get_play_state(), sprite_instance::STOP);

	// Now timers are set and counters initialized

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 0);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 0);

	usleep(500000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 1);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 0);

	usleep(500000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 2);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 1);

	usleep(500000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 3);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 1);

	usleep(500000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 4);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 2);

	usleep(1000000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 4);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 3);

	usleep(1000000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 4);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 4);

	usleep(500000); tester.advance(); // run expired timers

	root->get_member("this_counter", &tmp);
	check_equals(tmp.to_number(), 5);
	root->get_member("that_counter", &tmp);
	check_equals(tmp.to_number(), 4);

	root->get_member("pushed_args", &tmp);
	as_environment env; // needed for proper to_string()
	check_equals(tmp.to_string(&env), std::string("8,9,10"));

	root->get_member("test_completed", &tmp);
	check_equals(tmp.to_number(), 1);


}

