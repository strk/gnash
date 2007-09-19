/* 
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "key_event_test2.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "container.h"
#include "log.h"
#include "VM.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
  string filename =  string(INPUT_FILENAME);
  MovieTester tester(filename);

  gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
  dbglogfile.setVerbosity(1);

  sprite_instance* root = tester.getRootMovie();
  assert(root);

  check_equals(root->get_frame_count(), 3);
  check_equals(root->get_current_frame(), 0);

  tester.advance();
  check_equals(root->get_current_frame(), 1);

  tester.advance();
  check_equals(root->get_current_frame(), 2);

  character* mc = const_cast<character*>(tester.findDisplayItemByName(*root, "mc"));
  check(mc);

  as_value tmp;
  string_table& st = VM::get().getStringTable();
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 0);
  
  // press key 'A' and checks
  tester.pressKey(key::A);
  tester.releaseKey(key::A);

  // check that KeyDown have been triggered
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 1);

  
  tester.advance(); // loop back to frame1
  // press key 'A' and checks
   tester.pressKey(key::A);
   tester.releaseKey(key::A);
  // check that no KeyDown was triggered(no key event handler at frame1);
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 0);

  tester.advance(); // advance to frame2
  tester.advance(); // advance to frame3
  check_equals(root->get_current_frame(), 2);

  // press key 'A' and checks
  tester.pressKey(key::A);
  tester.releaseKey(key::A);

  // check that KeyDown have been triggered
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 1); 

  tester.advance(); // loop back to frame1 again
  // press key 'A' and checks
  tester.pressKey(key::A);
  tester.releaseKey(key::A);
  // check that no KeyDown was triggered(no key event handler at frame1);
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 0);

  tester.advance(); // advance to frame2
  tester.advance(); // advance to frame3
  check_equals(root->get_current_frame(), 2);

  // press key 'A' and checks
  tester.pressKey(key::A);
  tester.releaseKey(key::A);

  // check that KeyDown have been triggered
  check(root->get_member(st.find("x1"), &tmp));
  check_equals(tmp.to_number(), 1);
  
  return 0; 
}
