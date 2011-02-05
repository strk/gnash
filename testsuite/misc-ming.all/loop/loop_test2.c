/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Test "Jumping backward to the start of two DisplayObject's lifetime after depth swap"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 
 *  --------+---+---+---+---+
 *   Event  |   |PP*| M | J |
 * 
 *  P = place (by PlaceObject2)
 *  M = move to another depth (by swapDepth)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: two static DisplayObjects are placed
 *  frame3: the two DisplayObjects are depth-swapped
 *  frame4: jump to frame 2 and stop.
 *
 * Expected behaviour:
 *
 *  A single instance of each DisplayObjects is created.
 *  First time in frame1 depths are the original ones,
 *  second time depths are swapped.
 *
 * run as ./loop_test2
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test2.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, dejagnuclip;
  SWFShape  sh1,sh2;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      //fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      //return 1;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate(mo, 2);

  // Frame 1: Place dejagnu clip

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: Place two static DisplayObjects

  sh1 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  sh2 = make_fill_square (330, 270, 120, 120, 0, 0, 0, 0, 0, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc2); 

  SWFDisplayItem it1, it2;
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_addAction(it1, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc1Constructed++;"
		), SWFACTION_CONSTRUCT);

  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip2
  SWFDisplayItem_addAction(it2, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc2Constructed++;"),
		  SWFACTION_CONSTRUCT);
  
  SWFMovie_nextFrame(mo);
  
  // Frame3: swap depths

  SWFMovie_add(mo, (SWFBlock)newSWFAction(
      "var mc1_depth = movieClip1.getDepth();"
      "var mc2_depth = movieClip2.getDepth();"
      "movieClip1.secretCode = 'mc1';"
      "movieClip2.secretCode = 'mc2';"
      "movieClip1.swapDepths(movieClip2);"
      "check_equals(movieClip1.getDepth(), mc2_depth);" 
      "check_equals(movieClip2.getDepth(), mc1_depth);"
      "check_equals(movieClip1.secretCode, 'mc1');" 
      "check_equals(movieClip2.secretCode, 'mc2');"
      ));

  SWFMovie_nextFrame(mo);  

  // Frame4: gotoAndStop(2), check..

  SWFMovie_add(mo, (SWFBlock)newSWFAction(

      "gotoAndStop(2);"

      // Depths have not be restored
      // (gnash fails because create new instnaces instead)
      "check_equals(movieClip1.getDepth(), mc2_depth);" 
      "check_equals(movieClip2.getDepth(), mc1_depth);"

      // They are still the same instance
      // (gnash fails because create new instnaces instead)
      "check_equals(movieClip1.secretCode, 'mc1');" 
      "check_equals(movieClip2.secretCode, 'mc2');"

      // Chars have not been reconstructed
      // (gnash fails because create new instnaces instead)
      "check_equals(mc1Constructed, 1);"
      "check_equals(mc2Constructed, 1);"

      "totals();"

	));
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
