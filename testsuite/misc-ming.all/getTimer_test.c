/* 
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 
/*
 * test ActionGetTimer, getTimer returns the time in milliseconds
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
  SWFMovie_setRate (mo, 10.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo);  // frame1
  
  add_actions(mo, "x1 = getTimer();");
  // check that the timer was properly initialized
  check(mo, "x1 > 0" );
  SWFMovie_nextFrame(mo); // frame2
  
  add_actions(mo, "x2 = getTimer();");
  
  // check that the timer is working
  check(mo, "x2 > x1" );

  // this is dependent on frame rate(current setting is 100ms per frame)
  // check(mo, "x2 > 100");
  check(mo, "x2 < 400");

  // check that "getTimer" return a intergral number
  check(mo, "x2 == Math.ceil(x2)");
  check(mo, "x2 == Math.floor(x2)");
  SWFMovie_nextFrame(mo); // frame3        

  add_actions(mo, "_root.totals(); stop();");
  SWFMovie_nextFrame(mo);        
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
