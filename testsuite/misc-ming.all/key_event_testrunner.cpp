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
* 
* You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "key_event_test.swf"

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
  string filename =  string(INPUT_FILENAME);
  MovieTester tester(filename);

  gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
  dbglogfile.setVerbosity(1);

  sprite_instance* root = tester.getRootMovie();
  assert(root);

  check_equals(root->get_frame_count(), 31);
  check_equals(root->get_current_frame(), 0);

  tester.advance();
  check_equals(root->get_current_frame(), 1);

  character* mc = const_cast<character*>(tester.findDisplayItemByName(*root, "mc"));
  check(mc);

  as_value tmp;

  check(root->get_member("x1", &tmp));
  check_equals(tmp.to_number(), 0);
  
  // press key 'A' and checks
  tester.pressKey(key::A);
  tester.releaseKey(key::A);

  // check that onClipKeyUp/KeyDown have been triggered
  check(root->get_member("x1", &tmp));
  check_equals(tmp.to_string(), "A");
  check(root->get_member("x2", &tmp));
  check_equals(tmp.to_number(), key::A);

  // check that user defined onKeyUp/KeyDown were not triggered
  check(root->get_member("x4", &tmp));
  check_equals(tmp.to_number(), 0);
  check(root->get_member("x5", &tmp));
  check_equals(tmp.to_number(), 0);

  for(int i=1; i<30; i++)
  {
    tester.advance();
  }

  check_equals(root->get_current_frame(), 30); // the 31th frame
  check_equals(root->get_play_state(), sprite_instance::STOP);

  // press key 'C' and checks
  tester.pressKey(key::C);
  tester.releaseKey(key::C);

  // check that onClipKeyUp/KeyDown have been triggered
  check(root->get_member("x1", &tmp));
  check_equals(tmp.to_string(), "C");
  check(root->get_member("x2", &tmp));
  check_equals(tmp.to_number(), key::C);
  
  // check that user defined onKeyUp/KeyDown have been triggered
  check(root->get_member("x4", &tmp));
  check_equals(tmp.to_string(), "C");
  check(root->get_member("x5", &tmp));
  check_equals(tmp.to_number(), key::C);
  	
  // check that user onClipKeyPress and user defined onKeyPress were not triggered
  // onClipKeyPress was not triggered because the event handler binds a invalid key code
  check(root->get_member("x3", &tmp));
  check_equals(tmp.to_number(), 0);
  // onKeyPress was not triggered because I think there is no user defined
  // KeyPress event handler at all( the defined onKeyPress is just a normal function).
  check(root->get_member("x6", &tmp));
  check_equals(tmp.to_number(), 0);
  
}
