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
 *
 */ 

#define INPUT_FILENAME "ButtonEventsTest.swf"

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

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();

	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);

	const character* mc1 = tester.findDisplayItemByName(*root, "square1");
	check(mc1);

	const character* text = tester.findDisplayItemByName(*root, "textfield");
	check(text);

	check_equals(string(text->get_text_value()), string("Play with the button"));
	check(!tester.isMouseOverMouseEntity());

	// TODO: check that pixel @ 60,60 is red !

	// roll over the middle of the square, this should change
	// the textfield value.
	tester.movePointerTo(60, 60);
	check_equals(string(text->get_text_value()), string("MouseOver"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is yellow !

	// press the mouse button, this should change
	// the textfield value.
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseDown"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is green !

	// depress the mouse button, this should change
	// the textfield value.
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseUp"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is yellow !

	// roll off the square, this should change
	// the textfield value.
	tester.movePointerTo(39, 60);
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// press the mouse button, this should not change anything
	// as we're outside of the button.
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// depress the mouse button, this should not change anything
	// as we're outside of the button.
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !
}

