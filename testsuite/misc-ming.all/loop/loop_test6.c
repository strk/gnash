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
 * Test "Jumping backward to the end of a DisplayObject's lifetime (with events: onConstruct)"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  --------+---+---+---+---+---+---+
 *   Event  |   | PP| RR| * | J |   |
 * 
 *  P = place (by PlaceObject2)
 *  R = remove (by RemoveObject* tag)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 *  frame2: (1)a static DisplayObject(movieclip1) is placed at depth 3 (-16381) [ a red square ]
 *             onInitialize and onConstruct event handlers defined, onUnload event handler NOT defined.
 *          (2)a static DisplayObject(movieclip2) is placed at depth 4 [a green square]
 *             onInitialize and onConstruct event handlers defined, onUnload event handler also defined.
 *  frame3: (1)DisplayObject at depth 3 (-16381) removed.
 *          (2)DisplayObject at depth 4 removed.
 *  frame5: jump back to frame 4 and stop
 *
 * Expected behaviour:
 *
 *   After jump back, 
 *   (1)the onInitialize and onConstruct event handler for the red square has been 
 *      invoked only once.
 *   (2)the onInitialize and onConstruct event handlers for the green square has been 
 *      invoked twice!
 *
 * run as ./loop_test6
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "loop_test6.swf"


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
  add_actions(mo, "mc1Initialized=0; mc1Constructed=0; mc2Initialized=0; mc2Constructed=0; ");
  SWFMovie_nextFrame(mo); 
  
  //
  // Frame 2: 
  //   Place red static movieClip1 DisplayObject at depth 3 (-16381)
  //   Place green static movieClip2 DisplayObject at depth 4 (-16380)
  //
  sh1 = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  sh2 = make_fill_square (300, 400, 60, 60, 255, 0, 0, 0, 255, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2); 
  SWFMovieClip_nextFrame(mc2);

  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_addAction(it1, newSWFAction(
    "_root.note(this+' constructed');"
    "_root.mc1Constructed++;"
    ), SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it1, newSWFAction(
    "_root.note(this+' initialized');"
    "_root.mc1Initialized++;"
    ), SWFACTION_INIT);
  
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setDepth(it2, 4);
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip1
  SWFDisplayItem_addAction(it2, newSWFAction(
    "_root.note(this+' constructed');"
    "_root.mc2Constructed++;"
    ), SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it2, newSWFAction(
    "_root.note(this+' initialized');"
    "_root.mc2Initialized++;"
    ), SWFACTION_INIT);
  SWFDisplayItem_addAction(it2, newSWFAction(
    "_root.note(this+' unloaded');"
    ), SWFACTION_UNLOAD);
    
  check_equals(mo, "typeof(movieClip1)", "'movieclip'");
  check_equals(mo, "_root.mc1Constructed", "1");
  check_equals(mo, "typeof(movieClip2)", "'movieclip'");
  check_equals(mo, "_root.mc1Constructed", "1");
  SWFMovie_nextFrame(mo);  

  // Frame3: Remove movieclip1 and movieclip2
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  check_equals(mo, "typeof(movieClip1)", "'undefined'");
  check_equals(mo, "_root.mc1Constructed", "1");
  SWFMovie_nextFrame(mo);  
  
  // Frame4: nothing new
  SWFMovie_nextFrame(mo); 
  
  // Frame5: check, gotoAndStop(4), check..

  SWFMovie_add(mo, (SWFBlock)newSWFAction( "gotoAndStop(4);"));
  check_equals(mo, "typeof(movieClip1)", "'undefined'");
  check_equals(mo, "typeof(movieClip2)", "'movieclip'");

  // onClipConstruct invoked or not during jumping back is dependent 
  // on whether the onClipUnload has been defined.
  // Gnash fails by calling onClipConstruct again without considering onClipUnload!!
  check_equals(mo, "_root.mc1Constructed", "1");
  check_equals(mo, "_root.mc1Initialized", "1");

  check_equals(mo, "_root.mc2Constructed", "2");
  check_equals(mo, "_root.mc2Initialized", "2");
  SWFMovie_add(mo, (SWFBlock)newSWFAction( "totals(); stop();" ));
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
