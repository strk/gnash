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

#define INPUT_FILENAME "morph_test1.swf"

#include "MovieTester.h"
#include "MovieClip.h"
#include "DisplayObject.h"
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
  //string filename = string(TGTDIR) + string("/") + string(INPUT_FILENAME);
  string filename = string(INPUT_FILENAME);
  MovieTester tester(filename);

  gnash::LogFile& dbglogfile = gnash::LogFile::getDefaultInstance();
  dbglogfile.setVerbosity(1);

  
  rgba red(255,0,0,255);   //start color
  rgba green(0,255,0,255); //end color

  MovieClip* root = tester.getRootMovie();
  assert(root);
  check_equals(root->get_frame_count(), 8);
  
  // FRAME 2 
  tester.advance(); 
  // fill color: 1.0*red + 0.0*green
  // center coordinates: <start_x+50, start_y+50>
  check_pixel(50, 50, 50, red, 2); // morph ratio = 0

  // FRAME 3
  tester.advance(); 
  // 0.8*red + 0.2*green
  // center coordinates: <0.8*start_x+0.2*end_x+50, 0.8*start_y+0.2*end_y+50>
  check_pixel(190, 150, 50, rgba(204,51,0,255), 2); // morph ratio = 0.2
  
  // FRAME 4
  tester.advance(); 
  // fill color: 0.6*red + 0.4*green
  check_pixel(330, 250, 50, rgba(153,102,0,255), 2); // morph ratio = 0.4

  // FRAME 5
  tester.advance(); 
  // fill color: 0.4*red + 0.6*green
  check_pixel(470, 350, 50, rgba(102,153,0,255), 2); // morph ratio = 0.6

  // FRAME 6
  tester.advance(); 
  // fill color: 0.2*red + 0.8*green
  check_pixel(610, 450, 50, rgba(51,204,0,255), 2); // morph ratio = 0.8

  // FRAME 7
  tester.advance(); 
  // fill color: 0.0*red + 1.0*green
  check_pixel(750, 550, 50, rgba(0,255,0,255), 2); // morph ratio = 1.0

  // Frame 8
  tester.advance();
  // #39989. Nothing to check: empty morph shape.
}

