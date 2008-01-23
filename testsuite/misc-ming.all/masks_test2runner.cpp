/* 
 *   Copyright (C) 2007, 2008 Free Software Foundation, Inc.
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
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 */ 

#define INPUT_FILENAME "masks_test2.swf"

#include "MovieTester.h"
#include "sprite_instance.h"
#include "character.h"
#include "dlist.h"
#include "log.h"

#include "check.h"
#include <string>
#include <cassert>

using namespace gnash;
using namespace std;

int
main(int /*argc*/, char** /*argv*/)
{
  //string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
  string filename = string(INPUT_FILENAME);
  MovieTester tester(filename);

  gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
  dbglogfile.setVerbosity(1);

  
  rgba red(255,0,0,255);  
  rgba white(255, 255, 255, 255);

  sprite_instance* root = tester.getRootMovie();
  assert(root);
  check_equals(root->get_frame_count(), 3);

  // FRAME 2
  tester.advance(); 

  check_pixel(15, 15, 30, red, 2); 
  // visual check succeeds with AGG pixel format RGB24
  // don't know why this check fails with AGG pixel format AGG_RGB555
  // it works with cairo.
  xcheck_pixel(40, 40, 10, white, 3); 

  tester.movePointerTo(118, 118);
  xcheck( ! tester.isMouseOverMouseEntity() ); // not visible in its mask
  tester.movePointerTo(10, 10);
  check( tester.isMouseOverMouseEntity() ); // visible in its mask
  
  // FRAME 3 
  tester.advance(); 
  check_pixel(105, 105, 210, red, 2); 
  check_pixel(220, 220, 10, white, 2); 
}
