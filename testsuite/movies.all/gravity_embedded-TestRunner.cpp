/* 
 *   Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
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

#define INPUT_FILENAME "gravity-embedded.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"
#include "VM.h"
#include "TextField.h"

#include "check.h"
#include <string>
#include <cassert>
#include <unistd.h> // for usleep

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

	gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
	dbglogfile.setVerbosity(1);

	const MovieClip* root = tester.getRootMovie();
	assert(root);

	//const DisplayList& dl = root->getDisplayList();
	//dl.dump(std::cout);

	check_equals(root->get_frame_count(), 1);

	// give loader time to load the actual gravity.swf movie 
	usleep(5000);
	tester.advance(); // have load processed

	// used to get members
	as_value tmp;

	const DisplayObject* loaded = tester.findDisplayItemByDepth(*root, 0); // depends on getNextHighestDepth
	check(loaded);
	check_equals(loaded->get_parent(), root);

	string_table& st = VM::get().getStringTable();
	string_table::key xscale = st.find("_xscale");
	string_table::key yscale = st.find("_yscale");
	// we need a const_cast as get_member *might* eventually
	// change the DisplayObject (fetching _x shouldn't change it though)
	check(getObject(const_cast<DisplayObject*>(loaded))->get_member(xscale, &tmp));
	check(tmp.strictly_equals(50));

	check_equals(loaded->getBounds().height(), 2056);
	check_equals(loaded->getBounds().width(), 2056);

	const TextField* text = 
            dynamic_cast<const TextField*>(
                tester.findDisplayItemByDepth(
                    *root, 7 +DisplayObject::staticDepthOffset));
	check(text);
	
	check_equals(string(text->get_text_value()), "50");
	check(!tester.isMouseOverMouseEntity());

	// click some on the "smaller" button
    as_object* obj = getObject(const_cast<DisplayObject*>(loaded));
	tester.movePointerTo(474, 18);
	check(tester.isMouseOverMouseEntity());
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "50");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "48");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 48);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 48);
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "48");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "46");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 46);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 46);
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "46");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "44");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 44);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 44);

	// click some on the "larger" button
	tester.movePointerTo(580, 18);
	check(tester.isMouseOverMouseEntity());
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "44");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "46");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 46);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 46);
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "46");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "48");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 48);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 48);
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "48");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "50");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 50);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 50);
	tester.pressMouseButton();
	check_equals(string(text->get_text_value()), "50");
	tester.depressMouseButton();
	check_equals(string(text->get_text_value()), "52");
	check(obj->get_member(xscale, &tmp));
	check_equals(round(tmp.to_number()), 52);
	check(obj->get_member(yscale, &tmp));
	check_equals(round(tmp.to_number()), 52);


}

