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
 * Test  "Jump backward to start of lifetime after removal and placement at same depth"
 *
 * run as ./displaylist_depths_test11
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   |P* | R | P | J |   |   |
 * 
 *  P = place (by PlaceObject2)
 *  R = Remove (by RemoveObject2 tag)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: DisplayObject 1 placed at depth -16381 
 *  frame3: remove DisplayObject -16381 
 *  frame4: DisplayObject 1 placed at depth -16381 (same DisplayObject id)
 *  frame5: jump back to frame 2 and stop
 * 
 * Expected behaviour:
 * 
 *  Two instances have been constructed in total, the second instance
 *  is NOT removed at time of jump-back.
 */

#include "ming_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "displaylist_depths_test11.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height);
SWFMovieClip get_static_mc(int width, int height);

SWFMovieClip
get_static_mc(int width, int height)
{
  SWFShape sh;
  SWFMovieClip mc;

  sh = make_fill_square (-(width/2), -(height/2), width, height, 255, 0, 0, 255, 0, 0);
  mc = newSWFMovieClip();
  SWFMovieClip_add(mc, (SWFBlock)sh);

  SWFMovieClip_nextFrame(mc);

  return mc;

}

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth, int x, int y, int width, int height)
{
  SWFMovieClip mc;
  SWFDisplayItem it;

  mc = get_static_mc(width, height);

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
  SWFMovieClip dejagnuclip, static3;
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

  static3 = get_static_mc(20, 20);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  // Frame 2: place a static DisplayObject at depth 3 (-16381)
  it1 = SWFMovie_add(mo, (SWFBlock)static3);
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_setName(it1, "static1");
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipConstruct');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);
  SWFMovie_nextFrame(mo); 

  // Frame 3: remove DisplayObject at depth -16381 
  SWFDisplayItem_remove(it1);
  add_actions(mo, "check_equals(typeof(static1), 'undefined');"); // the replacement failed
  SWFMovie_nextFrame(mo); 
 
  // Frame 4: place same static DisplayObject at depth 3 (-16381)
  it1 = SWFMovie_add(mo, (SWFBlock)static3);
  SWFDisplayItem_setDepth(it1, 3);
  SWFDisplayItem_setName(it1, "static2");
  SWFDisplayItem_addAction(it1, newSWFAction(
			"_root.note(this+' onClipConstruct');"
			" _root.check_equals(typeof(_root), 'movieclip');"
		        " if ( isNaN(_root.depth3Constructed) ) {"
			"	_root.depth3Constructed=1; "
			" } else {"
			"	_root.depth3Constructed++;"
			" }"
			" _root.note('_root.depth3Constructed set to '+_root.depth3Constructed);"
			), SWFACTION_CONSTRUCT);

  // Frame 5: jump to frame 2, stop and check
  add_actions(mo,
    "check_equals(typeof(static1), 'undefined');"
    "check_equals(typeof(static2), 'movieclip');"

    "gotoAndStop(2); " 

    // two instances were placed in total, the second instance is not
    // supposed to be removed on jump back, being on a depth supposed
    // to contain a timeline instance at that time
    // Gnash fails here by removing the instance placed in a later frame
    "check_equals(_root.depth3Constructed, 2);"

    // Gnash fails here by removing the instance placed in a later frame
    "check_equals(typeof(static1), 'undefined');"

    // Gnash fails here by removing the instance placed in a later frame
    // (thus placing a new instance)
    "check_equals(typeof(static2), 'movieclip');"

    "totals();"
    );
  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
