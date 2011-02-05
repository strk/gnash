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
 * Test "Jumping backward to the start of a DisplayObject's lifetime after being swapped to the depth of a subsequently placed DisplayObject"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 
 *  --------+---+---+---+---+
 *   Event  |   |P* |PM | J |
 * 
 *  P = place (by PlaceObject2)
 *  M = move to another depth (by swapDepth)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: a static DisplayObjects is placed at depth 2 (-16381) [ a red square ]
 *  frame3: a static DisplayObject is placed at depth 3 (-16380) [ a black square ]
 *          the two DisplayObjects are depth-swapped
 *  frame4: jump to frame 2 and stop.
 *
 * Expected behaviour:
 *
 *  A single instance of each DisplayObjects is created.
 *  After loop-back only the DisplayObject placed in frame 3 is still alive (the black square),
 *  at depth -16381. The DisplayObject placed in frame 2 has been destroyed !
 *
 * run as ./loop_test3
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test3.swf"


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

  // Frame 1: Place dejagnu clip

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: Place red static movieClip1 DisplayObject at depth 3 (-16381)

  sh1 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_addAction(it1, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc1Constructed++;"
		), SWFACTION_CONSTRUCT);

  SWFMovie_nextFrame(mo);
  
  // Frame 3: Place black static movieClip2 DisplayObject at depth 4 (-16380), swap depths

  sh2 = make_fill_square (330, 270, 120, 120, 0, 0, 0, 0, 0, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc2); 

  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setDepth(it2, 4);
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip2
  SWFDisplayItem_addAction(it2, newSWFAction(
		"_root.note(this+' constructed');"
		"_root.mc2Constructed++;"),
		  SWFACTION_CONSTRUCT);
  

  SWFMovie_add(mo, (SWFBlock)newSWFAction(
      "check_equals(movieClip1.getDepth(), -16381);"
      "check_equals(movieClip2.getDepth(), -16380);"
      "movieClip1.secretCode = 'mc1';"
      "movieClip2.secretCode = 'mc2';"
      "movieClip1.swapDepths(movieClip2);"
      "check_equals(movieClip1.getDepth(), -16380);" 
      "check_equals(movieClip2.getDepth(), -16381);"
      "check_equals(movieClip1.secretCode, 'mc1');" 
      "check_equals(movieClip2.secretCode, 'mc2');"
      ));

  SWFMovie_nextFrame(mo);  

  // Frame4: gotoAndStop(2), check..

  SWFMovie_add(mo, (SWFBlock)newSWFAction(

      "gotoAndStop(2);"

      // Character placed in frame 3 at depth (-16380) was destroyed (placed *after* target frame)
      "check_equals(typeof(movieClip1), 'undefined');" 
      
      // Depth of DisplayObject at depth -16380 isn't restored (there's no DisplayObject at that depth)
      // (gnash fails because create new instances instead)
      "check_equals(movieClip2.getDepth(), -16381);"

      // movieClip2 is still the same instance
      // (gnash fails because create new instance instead)
      "check_equals(movieClip2.secretCode, 'mc2');"

      // Chars have been constructed only once
      // (gnash fails because create new movieClip1 instance instead)
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
