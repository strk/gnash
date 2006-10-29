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

#define INPUT_FILENAME "clip_as_button2.swf"

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
	string filename = string(SRCDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();

	dbglogfile.setVerbosity(1);

	sprite_instance* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);

	string msg_empty;
	string msg_topleft = "movie clip pressed";
	string msg_topright = "button pressed";
	string msg_botleft = "big movie clip pressed";
	string msg_botright = "small movie clip pressed";

	const character* text = tester.findDisplayItemByDepth(*root, 3);
	check(text);
	check_equals(string(text->get_text_value()), msg_empty);
	check(!tester.isMouseOverMouseEntity());

	// click in top-left movie clip circle
	tester.movePointerTo(176, 92);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is gray
	check_equals(string(text->get_text_value()), msg_topleft);
	tester.depressMouseButton();
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);

	// click "near" the top-left clip
	// (ie: inside it's boundin box, but not on the shape)
	tester.movePointerTo(143, 60);
	check(!tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is white
	tester.pressMouseButton();
	//TODO: check color under the pixel is white
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();

	// click in top-right movie clip circle
	tester.movePointerTo(427, 98);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is green
	check_equals(string(text->get_text_value()), msg_topright);
	tester.depressMouseButton();
	//TODO: check color under the pixel is blue
	check_equals(string(text->get_text_value()), msg_empty);

	// click "near" the top-right clip
	// (ie: inside it's boundin box, but not on the shape)
	tester.movePointerTo(385, 56);
	check(!tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is white
	tester.pressMouseButton();
	//TODO: check color under the pixel is white
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();

	// click in bottom-left movie clip square
	tester.movePointerTo(68, 281);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);

	// click near the bottom-left movie clip circle
	tester.movePointerTo(140, 251);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);

	// click in the bottom-left movie clip circle
	tester.movePointerTo(168, 283);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);

	// click in bottom-right movie clip square
	tester.movePointerTo(330, 284);
	check(!tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);

	// click near the bottom-right movie clip circle
	check(!tester.isMouseOverMouseEntity());
	tester.movePointerTo(404, 252);
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();
	//TODO: check color under the pixel is light blue
	check_equals(string(text->get_text_value()), msg_empty);

	// click in the bottom-right movie clip circle
	tester.movePointerTo(434, 291);
	check(tester.isMouseOverMouseEntity());
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_botright);
	tester.depressMouseButton();
	//TODO: check color under the pixel is yellow
	check_equals(string(text->get_text_value()), msg_empty);

}

