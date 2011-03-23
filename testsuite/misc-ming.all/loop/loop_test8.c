/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * zou lunkai  zoulunkai@gmail.com
 *
 * Test " Jumping backward to the middle of a DisplayObject's lifetime after removal and replacement(3)"
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 |
 *  --------+---+---+---+---+---+---+
 *   Event  |   |PPP| * | RR|PP | J |
 * 
 *  P = Place (by PlaceObject2)
 *  R = Remove (by RemoveObject)
 *  J = jump
 *  * = jump target
 *
 * Description:
 *
 * frame2: 
 *    a static movieclip is placed at depth 3 (-16381) with ratio value 0.001 [ a red square ]
 *    a static movieclip is placed at depth 4 (-16380) with ratio value 0.001 [ a green square ]
 *    a static movieclip is placed at depth 5 (-16379) with ratio value 0.001 [ a blue square ]
 * frame3: do nothing
 * frame4: 
 *   remove movieclip at depth 4;
 *   remove movieclip at depth 5;
 * frame5:
 *   a static movieclip is placed at depth4 again with ratio value 0.001 [a yellow square]
 *   a static movieclip is placed at depth5 agian with ratio value 0.003 [a black square]
 * frame6: gotoAndStop frame3
 *
 * Expected behaviour:
 *   movieclip in depth4 placed at frame5 kept alive;
 *   movieclip in depth5 placed at frame5 get destroyed;
 * 
 *  run as ./loop_test8
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test8.swf"

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, const char* color, int depth, float ratio);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, const char* color, int depth, float ratio)
{
  SWFShape sh;
  SWFMovieClip mc;
  SWFDisplayItem it;

  if( strcmp("red", color) == 0 ) 
  {
    // red square
    sh = make_fill_square (300, 0, 60, 60, 255, 0, 0, 255, 0, 0);
  }
  else if( strcmp("green", color) == 0 )
  {
    // green square
    sh = make_fill_square (300, 100, 60, 60, 0, 255, 0, 0, 255, 0);
  }
  else if( strcmp("blue", color) == 0 )
  {
    // blue square
    sh = make_fill_square (300, 200, 60, 60, 0, 0, 255, 0, 0, 255);
  }
  else if( strcmp("yellow", color) == 0 )
   {
    // yellow square
    sh = make_fill_square (300, 300, 60, 60, 255, 255, 0, 255, 255, 0);
  }
  else
  {
    // black square
    sh = make_fill_square (300, 400, 60, 60, 0, 0, 0, 0, 0, 0);
  } 
    
  mc = newSWFMovieClip();
  SWFMovieClip_add(mc, (SWFBlock)sh); 
  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setDepth(it, depth); 
  SWFDisplayItem_setName(it, name);
  SWFDisplayItem_setRatio(it, ratio);

  return it;
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;
  SWFDisplayItem it1, it2, it3, it4, it5;
  
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
  // Frame1: initialize testing variables
  add_actions(mo, " mc1Constructed=0; mc2Constructed=0; mc3Constructed=0; "
                  " mc4Constructed=0; mc5Constructed=0; "
                  " mc1Unloaded=0; mc2Unloaded=0; mc3Unloaded=0; ");
  SWFMovie_nextFrame(mo); // 1st frame 

  // Frame2: 
  //  add a static movieclip at depth3 with ratio value 0.001
  //  add a static movieclip at depth4 with ratio value 0.001 
  //  add a static movieclip at depth5 with ratio value 0.001 
  it1 = add_static_mc(mo, "mc1", "red", 3, 0.001);   
  it2 = add_static_mc(mo, "mc2", "green", 4, 0.001); 
  it3 = add_static_mc(mo, "mc3", "blue", 5, 0.001);  
  
  SWFDisplayItem_addAction(it1, 
    newSWFAction("_root.note(this+' constructed'); _root.mc1Constructed++;"), 
    SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it1, newSWFAction(""), SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it1, 
    newSWFAction("_root.note(this + ' unloaded'); _root.mc1Unloaded = 1;"), 
    SWFACTION_UNLOAD);
  
  SWFDisplayItem_addAction(it2, 
    newSWFAction("_root.note(this+' constructed'); _root.mc2Constructed++;"), 
    SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it2, newSWFAction(""), SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it2, 
    newSWFAction("_root.note(this + ' unloaded'); _root.mc2Unloaded = 1;"),
     SWFACTION_UNLOAD);
  
  SWFDisplayItem_addAction(it3, 
    newSWFAction("_root.note(this+' constructed'); _root.mc3Constructed++;"), 
    SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it3, newSWFAction(""), SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it3, 
    newSWFAction("_root.note(this + ' unloaded'); _root.mc3Unloaded = 1;"), 
    SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo); // 2nd frame 
 
  // Frame3: Do nothing
  SWFMovie_nextFrame(mo); // 3rd frame 

  // Frame4: Remove mc2(at depth4) and mc3(at depth5)
  SWFDisplayItem_remove(it2);
  SWFDisplayItem_remove(it3);
  SWFMovie_nextFrame(mo); // 4th frame 
  
  // Frame5:
  //   add mc4 with the same depth and ratio as mc2
  //   add mc5 with the same depth but different ratio as mc3
  it4 = add_static_mc(mo, "mc4", "yellow", 4, 0.001);
  it5 = add_static_mc(mo, "mc5", "black", 5, 0.003);
  
  SWFDisplayItem_addAction(it4, 
    newSWFAction("_root.note(this+' constructed'); _root.mc4Constructed++;"), 
    SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it4, newSWFAction(""), SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it4, newSWFAction("_root.note(this+ 'unloaded');"), SWFACTION_UNLOAD);
  
  SWFDisplayItem_addAction(it5, 
    newSWFAction("_root.note(this+' constructed'); _root.mc5Constructed++;"), 
    SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it5, newSWFAction(""), SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it5, newSWFAction("_root.note(this+ 'unloaded');"), SWFACTION_UNLOAD);
  SWFMovie_nextFrame(mo); // 5th frame 
   
  // Frame6: Checks, goto frame3 and stop, checks
  check_equals(mo, "typeof(mc1)", "'movieclip'");
  check_equals(mo, "typeof(mc2)", "'undefined'");
  check_equals(mo, "typeof(mc3)", "'undefined'");
  check_equals(mo, "typeof(mc4)", "'movieclip'");
  check_equals(mo, "typeof(mc5)", "'movieclip'");
  check_equals(mo, "mc1Constructed", "1");
  check_equals(mo, "mc2Constructed", "1");
  check_equals(mo, "mc3Constructed", "1");
  check_equals(mo, "mc4Constructed", "1");
  check_equals(mo, "mc5Constructed", "1");
  check_equals(mo, "mc1Unloaded", "0");
  check_equals(mo, "mc2Unloaded", "1");
  check_equals(mo, "mc3Unloaded", "1");
  
  add_actions(mo, "gotoAndStop(3);");
  
  check_equals(mo, "typeof(mc1)", "'movieclip'");
  check_equals(mo, "typeof(mc2)", "'undefined'"); 
  check_equals(mo, "typeof(mc3)", "'movieclip'"); 
  check_equals(mo, "typeof(mc4)", "'movieclip'"); 
  check_equals(mo, "typeof(mc5)", "'movieclip'"); // Gnash fails because of action execution order 
  check_equals(mo, "mc1Constructed", "1");
  check_equals(mo, "mc2Constructed", "1"); 
  check_equals(mo, "mc3Constructed", "2"); 
  check_equals(mo, "mc4Constructed", "1");
  check_equals(mo, "mc5Constructed", "1");
  check_equals(mo, "mc1Unloaded", "0");
  check_equals(mo, "mc2Unloaded", "1");
  check_equals(mo, "mc3Unloaded", "1");
  
  add_actions(mo, "totals();");
  SWFMovie_nextFrame(mo); // 6th frame 
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
