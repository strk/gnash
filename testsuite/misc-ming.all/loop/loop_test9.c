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
 *   Event  | * |PP |   | J |
 * 
 *  P = place (by PlaceObject2)
 *  M = move to another depth (by swapDepth)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: two static DisplayObjects are placed, both by PlaceObject2,
 *          one in the static zone, one in the dynamic zone
 *  frame4: jump to frame 1 and stop.
 *
 * Expected behaviour:
 *
 * The static DisplayObjects placed out of the static depth zone
 * get NOT removed on jump back. 
 *
 * run as ./loop_test9
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test9.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, mc3, dejagnuclip;
  SWFShape  sh1, sh2, sh3;
  SWFDisplayItem it1, it2, it3;

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

  sh3 = make_fill_square (330, 270, 120, 180, 0, 255, 0, 0, 255, 0);
  mc3 = newSWFMovieClip();
  SWFMovieClip_add(mc3, (SWFBlock)sh3);
  SWFMovieClip_nextFrame(mc3); 

  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_addAction(it1, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc1Constructed++;"
		), SWFACTION_CONSTRUCT);
  SWFDisplayItem_setDepth(it1, 3); // depth of movieClip1 is 3 (-16381)

  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip2
  SWFDisplayItem_addAction(it2, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc2Constructed++;"),
		  SWFACTION_CONSTRUCT);
  SWFDisplayItem_setDepth(it2, 30000); // depth of movieClip2 is 30000 (13616)

  it3 = SWFMovie_add(mo, (SWFBlock)mc3);  //add movieClip3 to the _root
  SWFDisplayItem_setName(it3, "movieClip3"); //name movieClip3
  SWFDisplayItem_addAction(it3, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc3Constructed++;"),
		  SWFACTION_CONSTRUCT);
  SWFDisplayItem_setDepth(it3, 30001); // depth of movieClip3 is 30001 (13617)

  SWFMovie_add(mo, (SWFBlock)newSWFAction(
      "check_equals(movieClip3.getDepth(), 13617);"
  ));
  
  SWFMovie_nextFrame(mo);
  
  // Frame4: RemoveObject(mc3) - one of those out of static depth zone

  SWFDisplayItem_remove(it3);

  // Frame4: gotoAndStop(1), check..

  SWFMovie_add(mo, (SWFBlock)newSWFAction(

      "var mc1_depth = movieClip1.getDepth();"
      "var mc2_depth = movieClip2.getDepth();"
      "movieClip1.secretCode = 'mc1';"
      "movieClip2.secretCode = 'mc2';"

      "check_equals(movieClip1.getDepth(), -16381);" 
      "check_equals(movieClip2.getDepth(), 13616);"
      // movieClip3 was removed, despite it's depth is out of the static zone
      "check_equals(typeof(movieClip3), 'undefined');"

      "gotoAndStop(1);"

      // MC1 (in static depth zone) has been removed
      "check_equals(typeof(movieClip1), 'undefined');"
      // MC2 (in dynamic depth zone) has been kept
      "check_equals(typeof(movieClip2), 'movieclip');"

      // The kept instance wasn't detroyed
      "check_equals(movieClip2.secretCode, 'mc2');"

      // Both chars have been constructed once (consistency check)
      "check_equals(mc1Constructed, 1);"
      "check_equals(mc2Constructed, 1);"

      "totals(9);"

	));
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
