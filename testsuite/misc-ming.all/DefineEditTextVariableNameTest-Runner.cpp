/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Linking Gnash statically or dynamically with other modules is making a
 * combined work based on Gnash. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.
 *
 * As a special exception, the copyright holders of Gnash give you
 * permission to combine Gnash with free software programs or libraries
 * that are released under the GNU LGPL and with code included in any
 * release of Talkback distributed by the Mozilla Foundation. You may
 * copy and distribute such a system following the terms of the GNU GPL
 * for all but the LGPL-covered parts and Talkback, and following the
 * LGPL for the LGPL-covered parts.
 *
 * Note that people who make modified versions of Gnash are not obligated
 * to grant this special exception for their modified versions; it is their
 * choice whether to do so. The GNU General Public License gives permission
 * to release a modified version without this exception; this exception
 * also makes it possible to release a modified version which carries
 * forward this exception.
 *
 */ 

#define INPUT_FILENAME "DefineEditTextVariableNameTest.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
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

	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 3);
	check_equals(root->get_play_state(), movie_interface::PLAY);
	check_equals(root->get_current_frame(), 0);

	const character* mc1 = tester.findDisplayItemByName(*root, "mc1");
	check(mc1);

	const sprite_instance* mc1_sp = \
		dynamic_cast<const sprite_instance*>(mc1);
	assert(mc1_sp);

	check_equals(mc1->get_height(), 16*20);
	check_equals(mc1->get_width(), 136*20);
	check_equals(mc1_sp->get_height(), 16*20);
	check_equals(mc1_sp->get_width(), 136*20);

	const character* textfield = tester.findDisplayItemByName(*mc1_sp,
			"textfield");
	check(textfield);

	check_equals(string(textfield->get_text_value()), string("Hello World"));
	check_equals(textfield->get_height(), 16*20);
	check_equals(textfield->get_width(), 136*20);

	tester.advance();

	check_equals(root->get_play_state(), movie_interface::PLAY);
	check_equals(root->get_current_frame(), 0);
	check_equals(tester.findDisplayItemByName(*root, "mc1"), mc1_sp);
	check_equals(tester.findDisplayItemByName(*mc1_sp, "textfield"),
		textfield);
	check_equals(string(textfield->get_text_value()), string("Hello"));

	tester.advance();

	check_equals(root->get_play_state(), movie_interface::PLAY);
	check_equals(root->get_current_frame(), 1);
	check_equals(tester.findDisplayItemByName(*root, "mc1"), mc1_sp);
	check_equals(tester.findDisplayItemByName(*mc1_sp, "textfield"),
		textfield);
	check_equals(string(textfield->get_text_value()), string("World"));

}

