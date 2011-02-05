/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Test "Jumping backward to the end of a DisplayObject's lifetime (with events: onConstruct, onUnload)"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  --------+---+---+---+---+---+---+
 *   Event  |   | P | R | * | J |   |
 * 
 *  P = place (by PlaceObject2)
 *  R = remove (by RemoveObject* tag)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: a static DisplayObjects is placed at depth 3 (-16381) [ a red square ]
 *          Both onConstruct and onUnload event handlers defined.
 *  frame3: DisplayObject at depth 3 (-16381) removed
 *  frame5: jump back to frame 4 and stop
 *
 * Expected behaviour:
 *
 *   After jump back, the onConstruct event handler for the red square has been invoked twice.
 *
 * run as ./loop_test7
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test7.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, dejagnuclip;
  SWFDisplayItem it1;
  SWFShape sh1;

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
  add_actions(mo, "mc1Constructed=0; mc2Constructed=0; mc3Constructed=0; mc4Constructed=0;");
  SWFMovie_nextFrame(mo); 
  
  //
  // Frame 2: 
  //   Place red static movieClip1 DisplayObject at depth 3 (-16381)
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
  SWFDisplayItem_addAction(it1, newSWFAction(
    "_root.note(this+' unloaded');"
    "_root.mc1Unloaded++;"
    ), SWFACTION_UNLOAD);

  check_equals(mo, "typeof(movieClip1)", "'movieclip'");
  check_equals(mo, "_root.mc1Constructed", "1");

  SWFMovie_nextFrame(mo);  

  // Frame3: Remove red square
  SWFDisplayItem_remove(it1);
  // After compile, the RemoveObject2 tag is *after* the DoAction tag which 
  // contains the following check. So it's not surprise that we can still access
  // "movieClip1" here when considering the gloabal ActionQueue model! If the 
  // RemoveObject2 is *before* the DoAction, then typeof(movieClip1) will reurn 'undefined'.
  // So Gnash fails here because of action execution order!
  // TODO: add testcase for this(RemoveObject2 placed *before* DoAction within the same frame).
  check_equals(mo, "typeof(movieClip1)", "'movieclip'"); // kept alive for calling onUnload!
  check_equals(mo, "_root.mc1Constructed", "1");
  SWFMovie_nextFrame(mo);  
  
  // Frame4: nothing new
  SWFMovie_nextFrame(mo); 
  
  // Frame5: check, gotoAndStop(4), check..

  check_equals(mo, "typeof(movieClip1)", "'undefined'");
  SWFMovie_add(mo, (SWFBlock)newSWFAction( "gotoAndStop(4);"));
  check_equals(mo, "typeof(movieClip1)", "'movieclip'");

  // onConstruct is called twice
  check_equals(mo, "_root.mc1Constructed", "2");

  // this is due to action execution order, it's called twice, but
  // the second time it's called *after* the end of *this* DOACTION block ..
  check_equals(mo, "_root.mc1Unloaded", "1");

  SWFMovie_add(mo, (SWFBlock)newSWFAction( "totals(); stop();" ));
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

