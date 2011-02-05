/***********************************************************************
 *
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *
 *
 * Test DLIST tags with dynamic depth zone specified
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 
 *  --------+---+---+---+---+
 *   Event  |   |PP |TT | RR|
 * 
 *  P = place (by PlaceObject2)
 *  T = transform (by PlaceObject2 tag)
 *  R = remove(by RemoveObject2 tag)
 *
 * Description:
 *
 *  frame2: two static sprites are placed, both by PlaceObject2,
 *          one in depth 16384(0), one in depth 65535(49151)
 *  frame3: transform the sprites by PlaceObject2 tag
 *  frame4: remove the two sprites by RemoveObject2 tag
 *  frame5: place shape1 at depth 16384(0), shape2 at depth 65535(49151).
 *  frame6: replace shape1 with shape2
 *
 * Expected behaviour:
 *  DLIST tags manipulate DisplayObjects in dynamic zone as in static zone in this test.
 *
 * run as ./PlaceObject2Test
 */

#include "ming_utils.h"

#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "PlaceObject2Test.swf"

int
main(int argc, char** argv) 
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, dejagnuclip;
  SWFDisplayItem it1, it2;
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
  SWFMovie_setRate(mo, 6);
  
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo);  // frame 1
  
  
  mc1 = newSWFMovieClip();
  mc2 = newSWFMovieClip();
  
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to _root
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to _root
  SWFDisplayItem_setName(it1, "Mc1");
  SWFDisplayItem_setName(it2, "Mc2");
  // place 2 sprites DisplayObjects at dynamic zone
  SWFDisplayItem_setDepth(it1, 16384);
  SWFDisplayItem_setDepth(it2, 65535);
  
  check_equals(mo, "Mc1.getDepth()", "0");
  check_equals(mo, "Mc2.getDepth()", "49151");
  SWFMovie_nextFrame(mo); // frame 2
 
  // move sprites at dynamic zone
  SWFDisplayItem_move(it1, 0,   100);
  SWFDisplayItem_move(it2, 100, 200);
  check_equals(mo, "Mc1._y", "100");
  check_equals(mo, "Mc2._y", "200");
  SWFMovie_nextFrame(mo); // frame 3
  
  // remove sprites at dynamic zone
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  check_equals(mo, "Mc1", "undefined");
  check_equals(mo, "Mc2", "undefined");
  SWFMovie_nextFrame(mo); // frame 4
  
  // place a shape at dynamic zone
  sh1 = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  sh2 = make_fill_square (300, 300, 60, 60, 0, 255, 0, 0, 255, 0);
  it1 = SWFMovie_add(mo, (SWFBlock)sh1);
  it2 = SWFMovie_add(mo, (SWFBlock)sh2);
  SWFDisplayItem_setDepth(it1, 16384);
  SWFDisplayItem_setDepth(it2, 65535);
  SWFMovie_nextFrame(mo); // frame 5
  
  // replace the shape with another one
  if ( SWFMovie_replace(mo, it1, (SWFBlock)sh2) )
  {
    return 1;
  }
  SWFMovie_nextFrame(mo); // frame 6
  
  add_actions(mo, "totals(6); stop();");
  SWFMovie_nextFrame(mo); // frame 6
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
