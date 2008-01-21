/* 
 *   Copyright (C) 2005, 2006, 2007, 2008 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "DefineEditTextVariableNameTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = INPUT_FILENAME;
	MovieTester tester(filename);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 8);
	check_equals(root->get_play_state(), sprite_instance::PLAY);
	check_equals(root->get_current_frame(), 0);

	const character* mc1 = tester.findDisplayItemByName(*root, "mc1");
	check(mc1);

	const character* mc2 = tester.findDisplayItemByName(*root, "mc2");
	check(mc2);

	const character* mc3 = tester.findDisplayItemByName(*root, "mc3");
	check(mc3);

	check_equals(root->get_current_frame(), 0);
	for (unsigned f=root->get_current_frame(); f<root->get_frame_count()-1; ++f)
	{
		check_equals(root->get_current_frame(), f);
		check_equals(root->get_play_state(), sprite_instance::PLAY);
		tester.advance();
	}

	// does stop() on last frame
	check_equals(root->get_play_state(), sprite_instance::STOP);
	check_equals(root->get_current_frame(), root->get_frame_count()-1);

}

