/* 
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010,
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 
/*
 * test ActionGetTimer, getTimer returns the time in milliseconds.
 *
 * TODO: use AS method "ifFrameLoaded" to ensure that all frames are loaded before
 * testing getTimer().  
 *
 * Ming seems do not support "ifFrameLoaded" yet:(
 * Take care that gprocessor does not support frame-rate-control!
 * Take care that this test might fail if it is loaded from a network 
 *   and the loading speed is too slow!
*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "getTimer_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip   dejagnuclip;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo);  // frame1
  
  // get current time in frame2
  add_actions(mo, "x1 = getTimer();");
  SWFMovie_nextFrame(mo); // frame2

  // just delay some time here
  add_actions(mo, " for(i=0; i<1000; i++) {} ");
  // get current time in frame3 
  add_actions(mo, "x2 = getTimer();");
  SWFMovie_nextFrame(mo); // frame3

  // check that the timer is working
  check(mo, "x1 > 0");
  check(mo, "x2 > x1" );
  // this is dependent on frame rate(current setting is 1 second per frame)
  // check(mo, "x2 > 1000");
  check(mo, "x2 < 6000");
  // check that "getTimer" return a intergral number
  check(mo, "x2 == Math.ceil(x2)");
  check(mo, "x2 == Math.floor(x2)");
  SWFMovie_nextFrame(mo); // frame4        

  add_actions(mo, "_root.totals(); stop();");
  SWFMovie_nextFrame(mo); // frame5       
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
