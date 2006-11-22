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
test_mouse_activity(MovieTester& tester, const character* text)
{
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

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = INPUT_FILENAME;
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 3);

	check_equals(root->get_current_frame(), 0);

	const character* mc1 = tester.findDisplayItemByName(*root, "square1");
	check(mc1);
	check_equals(mc1->get_depth(), 2);

	const character* text = tester.findDisplayItemByName(*root, "textfield");
	check(text);

	const character* square_back = tester.findDisplayItemByDepth(*root, 1);
	check(!square_back);
	const character* square_front = tester.findDisplayItemByDepth(*root, 3);
	check(!square_front);

	check_equals(string(text->get_text_value()), string("Play with the button"));
	check(!tester.isMouseOverMouseEntity());
	// TODO: check that pixel @ 60,60 is red !

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();
	check_equals(root->get_current_frame(), 0);

	for (int fno=0; fno<root->get_frame_count(); fno++)
	{
		check_equals(root->get_current_frame(), fno);

		info (("testing mouse activity in frame %d", root->get_current_frame()));
		test_mouse_activity(tester, text);

		// TODO: check why we need this !!
		//       I wouldn't want the first advance to be needed
		tester.advance();

	}

	// last advance should restart the loop...
	check_equals(root->get_current_frame(), 0);

}

