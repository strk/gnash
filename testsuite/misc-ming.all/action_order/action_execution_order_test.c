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
 * Test the execution order of actions within different movieClips
 *
 * set a variable with a new value under _root, then set it again under anther movieClip.
 *   the variable will only record the later assignment. Checking the final result of the
 *   variable will tell us which action(assignment) is executed first.
 *
 * Expected result is:
 *   In the frame placing mc_red,  actions in mc_red is executed *after* actions in _root
 *   In subsequent frames, actions in mc_red is executed *before* actions in _root
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red, mc_blu, dejagnuclip;
  SWFShape  sh_red, sh_blu;
  SWFDisplayItem it_red, it_blu;

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
  add_actions(mo, " trace('as in frame0 of root');"); // can't use 'note' here, as it's not been defined yet
  SWFMovie_nextFrame(mo);  /* 1st frame */

  mc_blu = newSWFMovieClip();
  sh_blu = make_fill_square (20, 320, 20, 20, 0, 0, 255, 0, 0, 255);
  SWFMovieClip_add(mc_blu, (SWFBlock)sh_blu);  
  add_clip_actions(mc_blu, " onUnload = function () { "
		  "_root.note('onUnload of mc_blu'); "
		  "_root.mc_unload_executed++; "
		  "_root.x3 = 'as_in_mc_blu_unload'; "
		  "};");
  add_clip_actions(mc_blu, " _root.note('as in frame1 of mc_blu'); _root.x1 = \"as_in_mc_blu\"; ");
  SWFMovieClip_nextFrame(mc_blu); /* 1st frame */
  add_clip_actions(mc_blu, " _root.note('as in frame2 of mc_blu'); _root.x2 = \"as_in_mc_blu\"; stop(); ");
  SWFMovieClip_nextFrame(mc_blu); /* 2nd frame */
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  /* Add mc_blu to mc_red and name it as "mc_blu" */
  it_blu = SWFMovieClip_add(mc_red, (SWFBlock)mc_blu);  
  SWFDisplayItem_setDepth(it_blu, 3); 
  SWFDisplayItem_setName(it_blu, "mc_blu");
  add_clip_actions(mc_red, " _root.note('as in frame1 of mc_red'); _root.x1 = \"as_in_mc_red\"; ");
  add_clip_actions(mc_red, " func = function() {}; ");
  SWFMovieClip_nextFrame(mc_red); /* 1st frame */
  add_clip_actions(mc_red, " _root.note('as in frame2 of mc_red'); _root.x2 = 'as_in_mc_red'; _root.x3 = 'as_in_mc_red_frame2'; stop(); ");
  SWFDisplayItem_remove(it_blu);
  SWFMovieClip_nextFrame(mc_red); /* 2nd frame */
  
  /* Add mc_red to _root and name it as "mc_red" */
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  
  add_actions(mo, " note('as in frame1 of root'); var x1 = \"as_in_root\"; ");
  /*
   * Check that the DisplayList is initialized deep to the mc_blue level
   * Even if their actions are not expected to be executed yet
   */
  check_equals(mo, "typeOf(_root.mc_red)", "'movieclip'");
  check_equals(mo, "typeOf(_root.mc_red.func)", "'undefined'");
  check_equals(mo, "typeOf(_root.mc_red.mc_blu)", "'movieclip'");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  add_actions(mo, " note('as in frame2 of root'); _root.x2 = \"as_in_root\"; ");
  check_equals(mo, "typeOf(_root.mc_red.func)", "'function'");
  SWFMovie_nextFrame(mo); /* 3rd frame */

  /* In the frame placing mc_red,  actions in mc_red is executed *after* actions in _root */
  check_equals(mo, "_root.x1", "'as_in_mc_blu'");
  /* In subsequent frames, actions in mc_red is executed *before* actions in _root */
  check_equals(mo, "_root.x2", "'as_in_root'");
  check_equals(mo, "_root.x3", "'as_in_mc_blu_unload'");
  check_equals(mo, "_root.mc_unload_executed", "1");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 4th frame */

  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



