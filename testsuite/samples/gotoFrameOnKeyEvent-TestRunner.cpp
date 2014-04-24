/* 
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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

#define INPUT_FILENAME "gotoFrameOnKeyEvent.swf"

#include "MovieTester.h"
#include "GnashException.h"
#include "MovieClip.h"
#include "DisplayObject.h"
#include "DisplayList.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

TRYMAIN(_runtest);
int
trymain(int /*argc*/, char** /*argv*/)
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

  check_equals(root->get_frame_count(), 7);

  check_equals(root->get_current_frame(), 0);
  tester.advance();
  check_equals(root->get_current_frame(), 0);

  // press key 1 four times
  for(unsigned int i=1; i<=4; i++)
  {
    tester.pressKey(key::_1);
    tester.releaseKey(key::_1);
    check_equals(root->get_current_frame(), i);
    tester.advance();
    check_equals(root->get_current_frame(), i);
  }
  
  // press key 0 four times
  for(unsigned int i=4; i>0; --i)
  {
    tester.pressKey(key::_0);
    tester.releaseKey(key::_0);
    check_equals(root->get_current_frame(), i-1);
    tester.advance();
    check_equals(root->get_current_frame(), i-1);
  }

  // press key 1 two times
  for(unsigned int i=1; i<=2; i++)
  {
    tester.pressKey(key::_1);
    tester.releaseKey(key::_1);
    check_equals(root->get_current_frame(), i);
    tester.advance();
    check_equals(root->get_current_frame(), i);
  }
  
  // press key DOWN two times
  {
    tester.pressKey(key::DOWN);
    tester.releaseKey(key::DOWN);
    check_equals(root->get_current_frame(), 5);
    tester.advance();
    check_equals(root->get_current_frame(), 5);

    tester.pressKey(key::DOWN);
    tester.releaseKey(key::DOWN);
    check_equals(root->get_current_frame(), 6);
    tester.advance();
    check_equals(root->get_current_frame(), 6);
  }

  // press key UP two times
  {
    tester.pressKey(key::UP);
    tester.releaseKey(key::UP);
    check_equals(root->get_current_frame(), 5);
    tester.advance();
    check_equals(root->get_current_frame(), 5);

    tester.pressKey(key::UP);
    tester.releaseKey(key::UP);
    check_equals(root->get_current_frame(), 2);
    tester.advance();
    check_equals(root->get_current_frame(), 2);
  }

  // press key 0 two times, now should be back to the first frame
  for(unsigned int i=2; i>0; --i)
  {
    tester.pressKey(key::_0);
    tester.releaseKey(key::_0);
    check_equals(root->get_current_frame(), i-1);
    tester.advance();
    check_equals(root->get_current_frame(), i-1);
  }
  return 0;
}

