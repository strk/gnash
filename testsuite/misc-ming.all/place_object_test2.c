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
 *
 * observed behaviour(SWF6,7,8):
 *   if the given depth is occupied, PlaceObjec2(PLACE) tag won't replace the orginal one.
 * 
 * observed behaviour(SWF5):
 *   too odd to understand :(
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "place_object_test2.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFShape  sh1, sh2;
  SWFDisplayItem it1, it2;
  
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
  SWFMovie_setRate (mo, 12.0);
  
  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, "testvar1 = 0; testvar2 = 0;");
  SWFMovie_nextFrame(mo); // frame1
  
  //
  // Define movieClips
  //
  mc1 = newSWFMovieClip();
  sh1 = make_fill_square (100, 100, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh1);  
  SWFMovieClip_nextFrame(mc1);
  
  mc2 = newSWFMovieClip();
  sh2 = make_fill_square (300, 100, 60, 60, 255, 0, 0, 0, 255, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc2);
    
  SWFMovie_nextFrame(mo); // frame2

  //
  // Place mc1 at depth 3, place mc1 at depth3 again with a different name;
  // Observed behaviour:  later place does not create a new DisplayObject
  //
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 3); 
  SWFDisplayItem_setName(it1, "static_mc1");
  SWFDisplayItem_addAction(it1,
    newSWFAction(" _root.testvar1++; trace(this); trace(_root.testvar1); "),
    SWFACTION_INIT | SWFACTION_CONSTRUCT | SWFACTION_ONLOAD);  
  
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "static_mc2");
  SWFDisplayItem_addAction(it2,
    newSWFAction(" _root.testvar2++; trace(this); trace(_root.testvar2); "),
    SWFACTION_INIT | SWFACTION_CONSTRUCT | SWFACTION_ONLOAD); 
  
  check_equals(mo, "typeof(static_mc1)", "'movieclip'");
  if(OUTPUT_VERSION > 5)
  {
    // check that "static_mc2" doesn't get placed
    check_equals(mo, "typeof(static_mc2)", "'undefined'");
  }
  else
  {
    // check that "static_mc2" does get placed
    check_equals(mo, "typeof(static_mc2)", "'movieclip'");
  }
  SWFMovie_nextFrame(mo); // frame3
  
  if(OUTPUT_VERSION > 5)
  {
    check_equals(mo, "testvar1", "3");
    check_equals(mo, "testvar2", "0");
  }
  else
  {
    // swf5 does not support CONSTRUCT event
    check_equals(mo, "testvar1", "2");
    check_equals(mo, "testvar2", "2");
  }
  
  //
  // Place mc2 at depth 3 again.
  // Observed behaviour: no new DisplayObject gets created
  //
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "static_mc3");
  
  check_equals(mo, "typeof(static_mc1)", "'movieclip'");
  if(OUTPUT_VERSION > 5)
  {
    // check that "static_mc3" doesn't get placed
    check_equals(mo, "typeof(static_mc3)", "'undefined'");
  }
  else
  {
    // check that "static_mc3" does get placed
    check_equals(mo, "typeof(static_mc3)", "'movieclip'");
  }
  SWFMovie_nextFrame(mo); // frame4
  
  //
  // Place mc1 at depth 3 again with ratio set to 0.2
  // Observed behaviour: no new DisplayObject get placed(created).
  //
  it2 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "static_mc4");
  SWFDisplayItem_setRatio(it2, 0.2); 
  
  check_equals(mo, "typeof(static_mc1)", "'movieclip'");
  if(OUTPUT_VERSION > 5)
  {
    // check that "static_mc4" doesn't get placed.
    check_equals(mo, "typeof(static_mc4)", "'undefined'");
  }
  else
  {
    // check that "static_mc4" does get placed.
    check_equals(mo, "typeof(static_mc4)", "'movieclip'");
  }
  SWFMovie_nextFrame(mo); // frame5
  
  //
  // Place mc2 at depth 3 again with ratio set to 0.2
  // Observed behaviour: no new DisplayObject get placed(created).
  //
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "static_mc5");
  SWFDisplayItem_setRatio(it2, 0.2); 
  
  check_equals(mo, "typeof(static_mc1)", "'movieclip'");
  if(OUTPUT_VERSION > 5)
  {
    // check that "static_mc5" doesn't get placed.
    check_equals(mo, "typeof(static_mc5)", "'undefined'");
  }
  else
  {
    // check that "static_mc5" does get placed.
    check_equals(mo, "typeof(static_mc5)", "'movieclip'");
  }
  SWFMovie_nextFrame(mo); // frame6
  
  
  //
  // Odd, where are the movieclips now?
  // Note that all those movieclips are defined above but not now.
  //
  if(OUTPUT_VERSION <= 5)
  {
    add_actions(mo, 
      "check_equals(typeof(static_m1), 'undefined');"
      "check_equals(typeof(static_m2), 'undefined');"
      "check_equals(typeof(static_m3), 'undefined');"
      "check_equals(typeof(static_m4), 'undefined');"
      "check_equals(typeof(static_m5), 'undefined');"
      );
  }
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); // frame7  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

