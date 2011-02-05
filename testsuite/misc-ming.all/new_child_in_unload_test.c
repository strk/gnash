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
 *---case1---
 *
 * Description:
 *   (1) step1: create a dynamic child in an onClipUnload handler of static_mc1;
 *   (2) step2: define a onUnload handler for the child;
 *   (3) step3: remove static_mc1;
 *
 * Observed:
 *  after step3:
 *  (1) onClipUnload for static_mc1 is triggered.
 *  (2) user defined onUnload for the dynamic child is NOT triggered.
 *  (3) the dynamic child is destroyed even without onUnload triggered.
 *
 *---case2---
 *  (1) step1: create a dynamic child in an onClipLoad handler of static_mc2;
 *  (2) step2: define a onUnload handler for the child;
 *  (3) step3: remove static_mc2;
 *
 * Observed:
 *  after step3:
 *  (1)onClipUnload for static_mc2 is triggered.
 *  (2)user defined onUnload for the dynamic child is triggered.
 *
 * TODO:
 *   seems createEmptyMovieClip() does not work in an user defined onUnload handler,
 *   not sure why, need more inspect later. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "new_child_in_unload_test.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
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
  

  mc1 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc1);
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setName(it1, "static_mc1");
  SWFDisplayItem_addAction(it1,
    newSWFAction(
        " this.createEmptyMovieClip('dyn1', 100); "
        " _root.check_equals(dyn1.getDepth(), 100);"
        " _root.dyn1Ref = dyn1;"
        // shouldn't be executed.
        " dyn1.onUnload = function () { _root.check(false); } ;"
        "trace(this);"
    ),
    SWFACTION_UNLOAD);  
  
  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2);
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setName(it2, "static_mc2");
  SWFDisplayItem_addAction(it2,
    newSWFAction(
       " this.createEmptyMovieClip('dyn2', 200); "
       " _root.check_equals(dyn2.getDepth(), 200);"
       " dyn2.onUnload = function () {_root.dyn2testvar = 'executed'; } ;"
    ),
    SWFACTION_ONLOAD);  
  SWFDisplayItem_addAction(it2,
    newSWFAction(
      " _root.check_equals(_level0.dyn1Ref.getDepth(), 100);"
      " _level0.dyn1Ref.swapDepths(101); "
      // Check that we can still swap the new child created in onClipUnload(mc1)
      // Note mc1 is already unloaded(this is in mc2.unload).
      " _root.check_equals(_level0.dyn1Ref.getDepth(), 101);"
    ),
    SWFACTION_UNLOAD); 
  SWFMovie_nextFrame(mo); // frame2

  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); // frame3
  
  check_equals(mo, "typeof(_root.dyn1Ref)", "'movieclip'");
  check_equals(mo, "_root.dyn1Ref.valueof()", "null");
  // check that dyn2.onUnload was triggered
  check_equals(mo, "_root.dyn2testvar", "'executed'");
  SWFMovie_nextFrame(mo); // frame4

  add_actions(mo, " _root.totals(7); stop(); ");
  SWFMovie_nextFrame(mo); // frame5 
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

