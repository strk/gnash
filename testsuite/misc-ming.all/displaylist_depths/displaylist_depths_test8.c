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
 * Sandro Santilli, strk@keybit.net
 *
 * Test "Jumping backward to the start of a DisplayObject's lifetime after dynamic transformation"
 *
 * run as ./displaylist_depths_test8
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   |Pt*|   | T |   |   | J |
 * 
 *  P = place (by PlaceObject2)
 *  T = transform matrix (by PlaceObject2)
 *  t = transform matrix (by ActionScript)
 *  M = move to another (or same) depth (by swapDepth)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: DisplayObject placed at depth -16381 at position (10,200);
 *          increment _y += 2 using ActionScript.
 *  frame4: try to transform the DisplayObject to the right (50,200)
 *  frame7: jump back to frame 2 
 * 
 * Expected behaviour:
 * 
 *  * In frame 2 the instance is positioned at 10,202
 *    ActionScript matrix transform in frame2 prevent subsequent static transformations to apply:
 *  * In frames 4,5,6 and 7 the instance is still positioned at 10,202 
 *    (static transformations are disabled as the instance has been transformed by script in frame 2)
 *  * After the jump we have the same instances at depth -16381, still positioned at 10,202
 *  * A single instance has been constructed in total.
 *  * Actions in jump-target frame are executed after actions following the gotoAndPlay() call.
 *   
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "displaylist_depths_test8.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
  SWFShape sh;
  SWFMovieClip mc;
  SWFDisplayItem it;

  sh = make_fill_square (-(width/2), -(height/2), width, height, 255, 0, 0, 255, 0, 0);
  mc = newSWFMovieClip();
  SWFMovieClip_add(mc, (SWFBlock)sh);

  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setDepth(it, depth); 
  SWFDisplayItem_moveTo(it, x, y); 
  SWFDisplayItem_setName(it, name);

  return it;
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFDisplayItem it1;


  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      //fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      //return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 2);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: Add a static movieclip at depth 3 with origin at 10,200, then change it's _y to 202 with AS code.
  it1 = add_static_mc(mo, "static3", 3, 10, 200, 20, 20);
  add_actions(mo,
    "check_equals(static3.getDepth(), -16381);" 
    "static3.myThing = 'guess';"
    // dynamically transform it
    "static3._y += 2;"
    );
  SWFMovie_nextFrame(mo); 

  // Frame 3: nothing new, just checks, and end of test on second run
  SWFMovie_nextFrame(mo); 
  add_actions(mo,
    "if ( typeof(_root.runs) == 'undefined' ) {"
    " check_equals(static3._y, 202);" 
    " _root.runs=1;"
    "} else {"
    " check_equals(static3._y, 204);"  
    " totals();"
    " stop();"
    "}"
    );

  // Frame 4: move DisplayObject at depth 3 to position 50,200
  SWFDisplayItem_moveTo(it1, 50, 200); 
  add_actions(mo,
    // immune to MOVE after _y set by AS
    "check_equals(static3._x, 10);" 
    "check_equals(static3.getDepth(), -16381);" 
    );
  SWFMovie_nextFrame(mo); 

  // Frame 5: nothing new
  SWFMovie_nextFrame(mo); 

  // Frame 6: nothing new
  SWFMovie_nextFrame(mo); 

  // Frame 7: go to frame 2  and playe (movie will end on second execution of frame 3 actions)
  add_actions(mo,

    "   gotoAndPlay(2); " // The movie will be ended in frame 5
    
    // Static3 refers to same instance
    "check_equals(static3.myThing, 'guess');" 
    "check_equals(static3._x, 10);" // Probably PlaceObject was a no-op...

    // actions in frame 2 would be executed again, making this a 204, but only *after* this action block terminates..
    "check_equals(static3._y, 202);" 

    "check_equals(static3.getDepth(), -16381);" 
    );
  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
