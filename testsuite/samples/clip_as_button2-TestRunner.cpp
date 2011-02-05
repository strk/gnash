/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010,
 *   2011 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "clip_as_button2.swf"

#include "MovieTester.h"
#include "GnashException.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "TextField.h"
#include "DisplayList.h"
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
	auto_ptr<MovieTester> t;

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	try
	{
		t.reset(new MovieTester(filename));
	}
	catch (const GnashException& e)
	{
		std::cerr << "Error initializing MovieTester: " << e.what() << std::endl;
		exit(EXIT_FAILURE);
	}
	
	MovieTester& tester = *t;

	// TODO: check why we need this !!
	//       I wouldn't want the first advance to be needed
	tester.advance();

	const MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 1);

	string msg_empty;
	string msg_topleft = "movie clip pressed";
	string msg_topright = "button pressed";
	string msg_botleft = "big movie clip pressed";
	string msg_botright = "small movie clip pressed";

	rgba yellow(255, 255, 102, 255);
	rgba gray(159, 159, 159, 255);
	rgba dark_cyan(25,160,133,255);
	rgba white(255, 255, 255, 255);
	rgba blue(51, 0, 204, 255);
	rgba cyan(0, 255, 204, 255);
	rgba green(0,255,102,255);

	const TextField* text =
		dynamic_cast<const TextField*>(
			tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset));
	check(text);
	check_equals(string(text->get_text_value()), msg_empty);
	check(!tester.isMouseOverMouseEntity());

	// click in top-left movie clip circle
	tester.movePointerTo(176, 92);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is yellow
	check_pixel(176, 92, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// TODO: check invalidated bounds !
	// check color under the pixel is gray
	check_pixel(176, 92, 2, gray, 1);
	check_equals(string(text->get_text_value()), msg_topleft);
	tester.depressMouseButton();
	// check color under the pixel is yellow
	check_pixel(176, 92, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click "near" the top-left clip
	// (ie: inside it's boundin box, but not on the shape)
	tester.movePointerTo(143, 60);
	check(!tester.isMouseOverMouseEntity());
	// check color under the pixel is white
	check_pixel(143, 60, 2, white, 1);
	tester.pressMouseButton();
	// check color under the pixel is white
	check_pixel(143, 60, 2, white, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();

	// click in top-right movie clip circle
	tester.movePointerTo(427, 98);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is blue
	check_pixel(427, 98, 2, blue, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is green
	check_pixel(427, 98, 2, green, 1);
	check_equals(string(text->get_text_value()), msg_topright);
	tester.depressMouseButton();
	// check color under the pixel is blue
	check_pixel(427, 98, 2, blue, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click "near" the top-right clip
	// (ie: inside it's boundin box, but not on the shape)
	tester.movePointerTo(385, 56);
	check(!tester.isMouseOverMouseEntity());
	// check color under the pixel is white
	check_pixel(385, 56, 2, white, 1);
	tester.pressMouseButton();
	// check color under the pixel is white
	check_pixel(385, 56, 2, white, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();

	// click in bottom-left movie clip square
	tester.movePointerTo(68, 281);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is light blue
	check_pixel(68, 281, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is light blue
	check_pixel(68, 281, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	// check color under the pixel is light blue
	check_pixel(68, 281, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click near the bottom-left movie clip circle
	tester.movePointerTo(140, 251);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is light blue
	check_pixel(140, 251, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is light blue
	check_pixel(140, 251, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	// check color under the pixel is light blue
	check_pixel(140, 251, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click in the bottom-left movie clip circle
	tester.movePointerTo(168, 283);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is yellow
	check_pixel(168, 283, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is yellow
	check_pixel(168, 283, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_botleft);
	tester.depressMouseButton();
	// check color under the pixel is yellow
	check_pixel(168, 283, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click in bottom-right movie clip square
	tester.movePointerTo(330, 284);
	check(!tester.isMouseOverMouseEntity());
	// check color under the pixel is light blue
	check_pixel(330, 284, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is light blue
	check_pixel(330, 284, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();
	// check color under the pixel is light blue
	check_pixel(330, 284, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click near the bottom-right movie clip circle
	check(!tester.isMouseOverMouseEntity());
	tester.movePointerTo(404, 252);
	// check color under the pixel is light blue
	check_pixel(404, 252, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// check color under the pixel is light blue
	check_pixel(404, 252, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.depressMouseButton();
	// check color under the pixel is light blue
	check_pixel(404, 252, 2, cyan, 1);
	check_equals(string(text->get_text_value()), msg_empty);

	// click in the bottom-right movie clip circle
	tester.movePointerTo(434, 291);
	check(tester.isMouseOverMouseEntity());
	// check color under the pixel is yellow
	check_pixel(434, 291, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);
	tester.pressMouseButton();
	// TODO: check invalidated bounds !
	// check color under the pixel is dark cyan
	check_pixel(434, 291, 2, dark_cyan, 1);
	check_equals(string(text->get_text_value()), msg_botright);
	tester.depressMouseButton();
	// check color under the pixel is yellow
	check_pixel(434, 291, 2, yellow, 1);
	check_equals(string(text->get_text_value()), msg_empty);

}

