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
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Test for movie loop.
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 
 *  --------+---+---+---+
 *   Event  |PP |   | M |
 * 
 *  P = place (by PlaceObject2)
 *  M = move to another depth (by swapDepth)
 *
 * Description:
 *
 *  frame1: two static DisplayObjects are placed
 *  frame3: the two DisplayObjects are depth-swapped
 *
 * Expected behaviour:
 *
 *  Normally, you can see both the red and black squares overlap each
 *  other with equal time.
 *  A single instance of the two DisplayObjects is created.
 *
 * run as ./loop_test
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test.swf"

//actions in _root movie, the last frame
SWFAction  action_in_root(void);
SWFAction  action_in_root()
{
  SWFAction ac;
  ac = compileSWFActionCode(
      "var mc1_depth = movieClip1.getDepth();"
      "var mc2_depth = movieClip2.getDepth();"
      "movieClip1.swapDepths(movieClip2);"
      "_root.check_equals(movieClip1.getDepth(), mc2_depth);" 
      "_root.check_equals(movieClip2.getDepth(), mc1_depth);"
      "if ( ++runs > 5 )  {"
      "  _root.check_equals(mc1Constructed, 1);"
      "  _root.check_equals(mc2Constructed, 1);"
      "  _root.check_equals(mc1Unloaded, undefined);"
      "  _root.check_equals(mc2Unloaded, undefined);"
      "  _root.check_equals(mc1Executed, 1);"
      "  _root.check_equals(mc2Executed, 1);"
      "  totals();"
      "  stop();"
      "}"
  );
  return ac;
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, dejagnuclip;
  SWFShape  sh1,sh2;
  SWFAction ac;

  const char *srcdir=".";
  if (argc > 1) srcdir=argv[1];

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate(mo, 6);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);


  sh1 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_add(mc1, (SWFBlock)newSWFAction("_root.mc1Executed++;"));
  SWFMovieClip_nextFrame(mc1);
  
  sh2 = make_fill_square (330, 270, 120, 120, 0, 0, 0, 0, 0, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_add(mc2, (SWFBlock)newSWFAction("_root.mc2Executed++;"));
  SWFMovieClip_nextFrame(mc2); 

  SWFDisplayItem it1, it2;
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  SWFDisplayItem_setDepth(it1, 64000);
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_addAction(it1, newSWFAction("_root.mc1Constructed++;"),
		  SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it1, newSWFAction("_root.mc1Unloaded++;"),
		  SWFACTION_UNLOAD);

  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip2
  SWFDisplayItem_addAction(it2, newSWFAction("_root.mc2Constructed++;"),
		  SWFACTION_CONSTRUCT);
  SWFDisplayItem_addAction(it2, newSWFAction("_root.mc2Unloaded++;"),
		  SWFACTION_UNLOAD);
  
  SWFMovie_nextFrame(mo); 
  SWFMovie_nextFrame(mo); 

  ac =  action_in_root();
  SWFMovie_add(mo, (SWFBlock)ac);
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
