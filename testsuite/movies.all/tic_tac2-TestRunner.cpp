/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#define INPUT_FILENAME "tic_tac2.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "Button.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"
#include "TextField.h"

#include "check.h"
#include <string>
#include <cassert>
#include <sstream>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
	string filename = string(SRCDIR) + string("/") + string(INPUT_FILENAME);
	MovieTester tester(filename);

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	const MovieClip* root = tester.getRootMovie();
	assert(root);

	check_equals(root->get_frame_count(), 10);
	check_equals(root->get_current_frame(), 0);

	const MovieClip* slides = dynamic_cast<const MovieClip*>(
		tester.findDisplayItemByTarget("_level0.slides"));
	check(slides);

	const Button* button = dynamic_cast<const Button*>(
		tester.findDisplayItemByTarget("_level0.b1"));
	check(button);

	tester.advance(); 
	check_equals(root->get_current_frame(), 1);
	check_equals(slides->get_current_frame(), 0);

	// Not much happens w/out clicking on the play button
	for (int i=0; i<10; ++i) tester.advance(); 
	check_equals(root->get_current_frame(), 1);
	check_equals(slides->get_current_frame(), 0);

	// Should start now
	tester.movePointerTo(395, 301); tester.click();
	check_equals(root->get_current_frame(), 1);
	check_equals(slides->get_current_frame(), 0);

	for (unsigned int i=0; i<3; ++i) {
		std::stringstream s; s << "iteration " << i;
		tester.advance(); 
		check_equals_label(s.str(), root->get_current_frame(), 2+i);
		check_equals_label(s.str(), slides->get_current_frame(), 0);
	}

    // Tweak initial offset (dunno based on what really)
    tester.advanceClock(1000);
    tester.advance(false); 
	check_equals(root->get_current_frame(), 4);
	check_equals(slides->get_current_frame(), 0);

	for (unsigned int i=0; i<12; ++i) {
        tester.advanceClock(1000);
        tester.advance(false); 
		std::stringstream s; s << "i" << i;
	    check_equals_label(s.str(), slides->get_current_frame(), i);
        // TODO: check invalidated bounds!
    }

	check_equals(slides->get_current_frame(), 11);
	check_equals(root->get_current_frame(), 4);

    // It's stuck there
	for (int i=0; i<10; ++i) tester.advance(); 
	check_equals(slides->get_current_frame(), 11);
	check_equals(root->get_current_frame(), 4);

}

