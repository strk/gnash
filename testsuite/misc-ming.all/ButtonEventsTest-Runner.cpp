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

void
test_mouse_activity(MovieTester& tester, const character* text, const character* text2)
{
	// roll over the middle of the square, this should change
	// the textfield value.
	tester.movePointerTo(60, 60);
	check_equals(string(text->get_text_value()), string("MouseOver"));
	check_equals(string(text2->get_text_value()), string("RollOver"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is yellow !

	// press the mouse button, this should change
	// the textfield value.
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseDown"));
	check_equals(string(text2->get_text_value()), string("Press"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is green !

	// depress the mouse button, this should change
	// the textfield value.
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseUp"));
	check_equals(string(text2->get_text_value()), string("Release"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is yellow !

	// roll off the square, this should change
	// the textfield value.
	tester.movePointerTo(39, 60);
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check_equals(string(text2->get_text_value()), string("RollOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// press the mouse button, this should not change anything
	// as we're outside of the button.
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check_equals(string(text2->get_text_value()), string("RollOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// depress the mouse button, this should not change anything
	// as we're outside of the button.
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseOut"));
	check_equals(string(text2->get_text_value()), string("RollOut"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// Now press the mouse inside and release outside
	tester.movePointerTo(60, 60);
	check_equals(string(text->get_text_value()), string("MouseOver"));
	check_equals(string(text2->get_text_value()), string("RollOver"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is yellow !
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), string("MouseDown"));
	check_equals(string(text2->get_text_value()), string("Press"));
	check(tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is green !
	tester.movePointerTo(39, 60);
	// The following might be correct, as the character still catches releaseOutside events
	//check(tester.isMouseOverMouseEntity());
	tester.depressMouseButton();
	xcheck_equals(string(text->get_text_value()), string("MouseUpOutside"));
	xcheck_equals(string(text2->get_text_value()), string("ReleaseOutside"));
}

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = INPUT_FILENAME;
	MovieTester tester(filename);

	std::string idleString = "Idle";

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 3);

	check_equals(root->get_current_frame(), 0);

	const character* mc1 = tester.findDisplayItemByName(*root, "square1");
	check(mc1);
	check_equals(mc1->get_depth(), 2+character::staticDepthOffset);

	const character* text = tester.findDisplayItemByName(*root, "textfield");
	check(text);

	const character* text2 = tester.findDisplayItemByName(*root, "textfield2");
	check(text2);

	const character* text3 = tester.findDisplayItemByName(*root, "textfield3");
	check(text3);

	check_equals(string(text->get_text_value()), idleString);
	check_equals(string(text2->get_text_value()), idleString);
	check_equals(string(text3->get_text_value()), idleString);
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	for (size_t fno=0; fno<root->get_frame_count(); fno++)
	{
		const character* square_back = tester.findDisplayItemByDepth(*root, 1+character::staticDepthOffset);
		const character* square_front = tester.findDisplayItemByDepth(*root, 3+character::staticDepthOffset);

		switch (fno)
		{
			case 0:
				check(!square_back);
				check(!square_front);
				break;
			case 1:
				check(square_back);
				check(!square_front);
				break;
			case 2:
				check(square_back);
				check(square_front);
				break;
		}

		check_equals(root->get_current_frame(), fno);

		info (("testing mouse activity in frame %d", root->get_current_frame()));
		test_mouse_activity(tester, text, text2);

		// TODO: test key presses !
		//       They seem NOT to trigger immediate redraw

		tester.advance();

	}

	// last advance should restart the loop...
	check_equals(root->get_current_frame(), 0);

}

