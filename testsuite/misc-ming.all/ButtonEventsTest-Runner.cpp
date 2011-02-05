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

#define INPUT_FILENAME "ButtonEventsTest.swf"

#include "MovieTester.h"
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

void
test_mouse_activity(MovieTester& tester, const TextField* text, const TextField* text2, bool covered, bool enabled)
{
	rgba red(255,0,0,255);
	rgba dark_red(128,0,0,255);
	rgba covered_red(127,126,0,255); // red, covered by 50% black
	rgba covered_dark_red(64,120,0,255); // dark red, covered by 50% black
	rgba yellow(255,255,0,255);
	rgba dark_yellow(128,128,0,255);
	rgba covered_yellow(128,255,0,255); // yellow, covered by 50% black
	rgba covered_dark_yellow(64,184,0,255); // dark yellow, covered by 50% black
	rgba green(0,255,0,255);

	// roll over the middle of the square, this should change
	// the textfield value, if enabled
	tester.movePointerTo(60, 60);
	if ( enabled ) {
		check(tester.usingHandCursor());
		check_equals(string(text->get_text_value()), string("MouseOver"));
		check_equals(string(text2->get_text_value()), string("RollOver"));
		check(tester.isMouseOverMouseEntity());
		if ( covered )
		{
			// check that pixel @ 60,60 is yellow (covered)
			check_pixel(60, 60, 2, covered_yellow, 2); 
			// check that pixel @ 72,64 is dark_yellow (covered)
			check_pixel(72, 64, 2, covered_dark_yellow, 2); 
		}
		else
		{
			// check that pixel @ 60,60 is yellow 
			check_pixel(60, 60, 2, yellow, 2); 
			// check that pixel @ 72,64 is dark_yellow 
			check_pixel(72, 64, 2, dark_yellow, 2); 
		}
	} else {
		check(!tester.usingHandCursor());
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
		check(!tester.isMouseOverMouseEntity());
		if ( covered )
		{
			// check that pixel @ 60,60 is red  (covered)
			check_pixel(60, 60, 2, covered_red, 2); 
			// check that pixel @ 72,64 is dark_red  (covered)
			check_pixel(72, 64, 2, covered_dark_red, 2); 
		}
		else
		{
			// check that pixel @ 60,60 is red 
			check_pixel(60, 60, 2, red, 2); 
			// check that pixel @ 72,64 is dark_red 
			check_pixel(72, 64, 2, dark_red, 2); 
		}
	}

	// press the mouse button, this should change
	// the textfield value, if enabled.
	tester.pressMouseButton();
	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseDown"));
		check_equals(string(text2->get_text_value()), string("Press"));
		check(tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is green !
		check_pixel(60, 60, 2, green, 2);
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
		check(!tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is red !
		if ( covered ) { check_pixel(60, 60, 2, covered_red, 2);  }
		else { check_pixel(60, 60, 2, red, 2);  }
	}

	// depress the mouse button, this should change
	// the textfield value, if enabled
	tester.depressMouseButton();
	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseUp"));
		check_equals(string(text2->get_text_value()), string("Release"));
		check(tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is yellow !
		if ( covered ) { check_pixel(60, 60, 2, covered_yellow, 2);  }
		else { check_pixel(60, 60, 2, yellow, 2);  }
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
		check(!tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is red !
		if ( covered ) { check_pixel(60, 60, 2, covered_red, 2);  }
		else { check_pixel(60, 60, 2, red, 2);  }
	}

	// roll off the square, this should change
	// the textfield value, if enabled
	tester.movePointerTo(39, 60);
	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseOut"));
		check_equals(string(text2->get_text_value()), string("RollOut"));
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
	}
	check(!tester.isMouseOverMouseEntity());
	// check that pixel @ 60,60 is red !
	if ( covered ) { check_pixel(60, 60, 2, covered_red, 2); }
	else { check_pixel(60, 60, 2, red, 2); }

	// press the mouse button, this should not change anything
	// as we're outside of the button.
	tester.pressMouseButton();
	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseOut"));
		check_equals(string(text2->get_text_value()), string("RollOut"));
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
	}
	check(!tester.isMouseOverMouseEntity());
	// check that pixel @ 60,60 is red !
	if ( covered ) { check_pixel(60, 60, 2, covered_red, 2); }
	else { check_pixel(60, 60, 2, red, 2); }

	// depress the mouse button, this should not change anything
	// as we're outside of the button.
	tester.depressMouseButton();
	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseOut"));
		check_equals(string(text2->get_text_value()), string("RollOut"));
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
	}
	check(!tester.isMouseOverMouseEntity());
	// check that pixel @ 60,60 is red !
	if ( covered ) { check_pixel(60, 60, 2, covered_red, 2); }
	else { check_pixel(60, 60, 2, red, 2); }

	// Now press the mouse inside and release outside

	tester.movePointerTo(60, 60); 

	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseOver"));
		check_equals(string(text2->get_text_value()), string("RollOver"));
		check(tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is yellow !
		if ( covered ) { check_pixel(60, 60, 2, covered_yellow, 2);  }
		else { check_pixel(60, 60, 2, yellow, 2);  }
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
		check(!tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is red !
		if ( covered ) { check_pixel(60, 60, 2, covered_red, 2); }
		else { check_pixel(60, 60, 2, red, 2); }
	}
	
	tester.pressMouseButton();

	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseDown"));
		check_equals(string(text2->get_text_value()), string("Press"));
		check(tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is green !
		check_pixel(60, 60, 2, rgba(0,255,0,255), 2);
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
		check(!tester.isMouseOverMouseEntity());
		// check that pixel @ 60,60 is red !
		if ( covered ) { check_pixel(60, 60, 2, covered_red, 2); }
		else { check_pixel(60, 60, 2, red, 2); }
	}

	tester.movePointerTo(39, 60);

	// The following might be correct, as the DisplayObject still catches releaseOutside events
	//check(tester.isMouseOverMouseEntity());
	tester.depressMouseButton();

	if ( enabled ) {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
	} else {
		check_equals(string(text->get_text_value()), string("MouseUpOutside"));
		check_equals(string(text2->get_text_value()), string("ReleaseOutside"));
	}
}

int
main(int /*argc*/, char** /*argv*/)
{
	//string filename = INPUT_FILENAME;
	string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	std::string idleString = "Idle";

	MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 7);

	check_equals(root->get_current_frame(), 0);

	const TextField* text = dynamic_cast<const TextField*>(
		tester.findDisplayItemByName(*root, "textfield"));
	check(text);

	const TextField* text2 = dynamic_cast<const TextField*>(
		tester.findDisplayItemByName(*root, "textfield2"));
	check(text2);

	const TextField* text3 = dynamic_cast<const TextField*>(
		tester.findDisplayItemByName(*root, "textfield3"));
	check(text3);

	check_equals(string(text->get_text_value()), idleString);
	check_equals(string(text2->get_text_value()), idleString);
	check_equals(string(text3->get_text_value()), idleString);

	tester.advance();
	check_equals(root->get_current_frame(), 1);

	const DisplayObject* mc1 = tester.findDisplayItemByName(*root, "square1");
	check(mc1);
	check_equals(mc1->get_depth(), 2+DisplayObject::staticDepthOffset);

	check(!tester.isMouseOverMouseEntity());
	check(!tester.usingHandCursor());

	// check that pixel @ 60,60 is red !
	rgba red(255,0,0,255);
	check_pixel(60, 60, 2, red, 2);

	tester.advance();
	check_equals(root->get_current_frame(), 2);
	tester.advance();
	check_equals(root->get_current_frame(), 2); // need to roll out

	tester.movePointerTo(60, 60); // roll over the square
	check(tester.isMouseOverMouseEntity());
	check(tester.usingHandCursor());
	check_equals(root->get_current_frame(), 2); // need to roll out

	tester.movePointerTo(0, 0); // roll out, should go to next frame
	check(!tester.isMouseOverMouseEntity());
	check(!tester.usingHandCursor());
	check_equals(root->get_current_frame(), 3); 

	for (size_t fno=root->get_current_frame(); fno<root->get_frame_count(); fno++)
	{
		const DisplayObject* square_back = tester.findDisplayItemByDepth(*root, 1+DisplayObject::staticDepthOffset);
		const DisplayObject* square_front = tester.findDisplayItemByDepth(*root, 3+DisplayObject::staticDepthOffset);

		switch (fno)
		{
			case 3:
				check(!square_back);
				check(!square_front);
				break;
			case 4:
				check(square_back);
				check(!square_front);
				break;
			case 5:
				check(square_back);
				check(square_front);
				break;
		}

		check_equals(root->get_current_frame(), fno);

		info (("testing mouse activity in frame %d", root->get_current_frame()));
		test_mouse_activity(tester, text, text2, square_front!=NULL, fno != root->get_frame_count()-1);

		// TODO: test key presses !
		//       They seem NOT to trigger immediate redraw

		tester.advance();

	}

	// last advance should not restart the loop (it's in STOP mode)
        check_equals(root->getPlayState(), MovieClip::PLAYSTATE_STOP);
	check_equals(root->get_current_frame(), 6);

}

