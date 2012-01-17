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

#define INPUT_FILENAME "key_event_test.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
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

  MovieClip* root = tester.getRootMovie();
  assert(root);

  check_equals(root->get_frame_count(), 24);
  check_equals(root->get_current_frame(), 0);

  tester.advance();
  tester.advance();
  // check we are stopped at frame2
  check_equals(root->get_current_frame(), 1);
  
  // provide a key press to continue the test
  tester.pressKey(key::A);
  tester.releaseKey(key::A);
  
  tester.advance();
  check_equals(root->get_current_frame(), 2);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame4
  check_equals(root->get_current_frame(), 3);
  
  // provide a key press to continue the test
  tester.pressKey(key::B);
  tester.releaseKey(key::B);
    
  tester.advance();
  check_equals(root->get_current_frame(), 4);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame6
  check_equals(root->get_current_frame(), 5);
 
  // provide a key press to continue the test
  tester.pressKey(key::C);
  tester.releaseKey(key::C);
    
  tester.advance();
  check_equals(root->get_current_frame(), 6);
  
  tester.advance();
  check_equals(root->get_current_frame(), 7);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame9
  check_equals(root->get_current_frame(), 8);
  
  // provide a key press to continue the test
  tester.pressKey(key::D);
  tester.releaseKey(key::D);
  
  // we have jumped back to frame8
  check_equals(root->get_current_frame(), 7);
  
  tester.advance();
  // and we are in frame9 again
  check_equals(root->get_current_frame(), 8);
  
  // provide a key press to continue the test
  tester.pressKey(key::E);
  tester.releaseKey(key::E);
  	
  // we have jumped forward to frame10
  check_equals(root->get_current_frame(), 9);
  
  tester.advance();
  check_equals(root->get_current_frame(), 10);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame12
  check_equals(root->get_current_frame(), 11);
  
  // provide a key press to continue the test
  tester.pressKey(key::F);
  tester.releaseKey(key::F);
  
  // we have jumped backward to frame11
  check_equals(root->get_current_frame(), 10);
  
  tester.advance();
  // and we are in frame12 again
  check_equals(root->get_current_frame(), 11);
  
  // provide a key press to continue the test
  tester.pressKey(key::G);
  tester.releaseKey(key::G);
  // we have jumped forward to frame13
  check_equals(root->get_current_frame(), 12);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame14
  check_equals(root->get_current_frame(), 13);
  
  // provide a key press to continue the test
  tester.pressKey(key::H);
  tester.releaseKey(key::H);
    
  tester.advance();
  tester.advance();
  // check we are stopped at frame15
  check_equals(root->get_current_frame(), 14);
  
  // provide a key press to continue the test
  tester.pressKey(key::I);
  tester.releaseKey(key::I);
  
  // check we have jumped to frame16
  check_equals(root->get_current_frame(), 15);
  
  for(int i=0; i<10; i++)
  {
 	 tester.advance();
  }
  // check we are stopped at frame20
  check_equals(root->get_current_frame(), 19);
  
  // provide a key press to continue the test
  tester.pressKey(key::J);
  tester.releaseKey(key::J);
  // check have jumped to frame21
  check_equals(root->get_current_frame(), 20);
  
  tester.advance();
  tester.advance();
  // check we are stopped at frame22
  check_equals(root->get_current_frame(), 21);
  
  // provide a key press to continue the test
  tester.pressKey(key::K);
  tester.releaseKey(key::K);
  tester.advance();

  // Select the text field
  tester.movePointerTo(310, 25);
  tester.click();

  // Enter 'i'
  tester.pressKey(key::i);
  tester.advance();

  // reached frame23, test finished
  check_equals(root->get_current_frame(), 23);
}
