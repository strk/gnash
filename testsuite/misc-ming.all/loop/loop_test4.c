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
 * Test "Jumping backward to the middle of a DisplayObject's lifetime after removal and replacement(1)"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  --------+---+---+---+---+---+---+
 *   Event  |   |PP | * | RR| Pp| J |
 * 
 *  P = place (by PlaceObject2)
 *  p = place (by ActionScript)
 *  M = move to another depth (by swapDepth)
 *  R = remove ((by RemoveObject* tag)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: 
 *    a static DisplayObjects is placed at depth 3 (-16381) [ a red square ]
 *    a static DisplayObject is placed at depth 4 (-16380) [ a black square ]
 *  frame3: nothing new.
 *  frame4: 
 *    DisplayObject at depth 3 is removed;
 *    DisplayObject at depth 4 is removed;
 *  frame5:
 *    a static DisplayObject is placed at depth 3 (-16381) with ratio set to 2.0
 *    a dynamic DisplayObject is placed at depth 4 (-16380)
 *          
 *  frame6: jump to frame 3 and stop.
 *
 * Expected behaviour:
 *
 *   After jump back, DisplayObjects placed at frame5 get destroyed, DisplayObjects placed at frame2
 *   get re-created.
 *
 * run as ./loop_test4
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "loop_test4.swf"


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
  add_actions(mo, "mc1Constructed=0; mc2Constructed=0; mc3Constructed=0;");
  SWFMovie_nextFrame(mo); 
  
  //
  // Frame 2: 
  //   Place red static movieClip1 DisplayObject at depth 3 (-16381)
  //   Place black static movieClip1 DisplayObject at depth 4 (-16380)
  //
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
  
  SWFMovie_nextFrame(mo);  

  // Frame3: nothing new
  SWFMovie_nextFrame(mo);  
  
  // Frame4: remove "movieClip1" and "movieClip2"
  
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); 
  
  //
  // Frame5: 
  //   Place red static movieClip3 DisplayObject at depth 3 (-16381) 
  //   Place an empty dynamic movieClip4 DisplayObject at depth4 (-16380)
  //

  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip3 to the _root
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_setName(it1, "movieClip3"); //name movieClip3
  SWFDisplayItem_setRatio(it1, 1.0); // It's important to set this, don't why yet.
  SWFDisplayItem_addAction(it1, newSWFAction(
    "_root.note(this+' constructed');"
    "_root.mc3Constructed++;"
    ), SWFACTION_CONSTRUCT);
  
  add_actions(mo, "_root.createEmptyMovieClip('movieClip4', -16380);");
  
  SWFMovie_nextFrame(mo); 
  
  //
  // Frame4: check, gotoAndStop(3), check..
  //
  check_equals(mo, "typeof(movieClip1)", "'undefined'");
  check_equals(mo, "typeof(movieClip2)", "'undefined'");
  check_equals(mo, "typeof(movieClip3)", "'movieclip'");
  check_equals(mo, "typeof(movieClip4)", "'movieclip'");
  check_equals(mo, "_root.mc1Constructed", "1");
  check_equals(mo, "_root.mc2Constructed", "1");
  check_equals(mo, "_root.mc3Constructed", "1");
  
  SWFMovie_add(mo, (SWFBlock)newSWFAction( "gotoAndStop(3);"));
  
  check_equals(mo, "typeof(movieClip1)", "'movieclip'");
  check_equals(mo, "typeof(movieClip2)", "'movieclip'");
  check_equals(mo, "typeof(movieClip3)", "'undefined'");
  check_equals(mo, "typeof(movieClip4)", "'undefined'");
  check_equals(mo, "_root.mc1Constructed", "2");
  check_equals(mo, "_root.mc2Constructed", "2");
  check_equals(mo, "_root.mc3Constructed", "1");
  
  SWFMovie_add(mo, (SWFBlock)newSWFAction( "totals(); stop();" ));
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

