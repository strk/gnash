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
 * all frame numbers are 1-based and reference the _root.
 *
 * expected order(or behaviour) is like this(player version higher than 4):
 *
 * (1) execute actions in 2nd frame, since "check"!=1, then do nothing;
 * (2) set "check" to 1, jump back to the second frame; 
 * (3) execute actions in 2nd frame, since "check"==1 now, jump to the 4th frame; 
 * (4) "mc1" is placed in the 4th frame, frame1 actions of mc1 get executed,
 *     which is the "_root.gotoAndStop(6);" 
 * (5) As "mc1" is removed in the 5th frmae, and "mc2" is removed in the 6th frame, 
 *     the 6th frame of _root should be visually empty.
 *
 * expected behaviour of player4:
 *
 * (1)(2)(3) are the same as the above;
 * (4) "mc1" is placed in the 4th frame, frame1 actions get executed
 *     but has no effect(player4 does not support mc.gotoAndStop). _root
 *     still advances to the 5th frame.  
 * (5) "mc1" is removed in the 5th frame. mc2 is placed in the 5th frame and 
 *     frame1 actions of mc2 get executed. Then the message 'your player version 
 *     is lower than 5' is printed on the screen.
 *
 *  TODO: generate SWF4 and SWF5 targets to automate test for both !
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define DEF_OUTPUT_VERSION 6
#define OUTPUT_FILENAME_FMT "action_execution_order_test8-v%d.swf"


int
main(int argc, char *argv[])
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFDisplayItem it1, it2;
  int outputVersion = DEF_OUTPUT_VERSION;
  
  const char *srcdir = ".";
  if (argc > 1) 
  {
    srcdir = argv[1];
    if (argc > 2) outputVersion = strtol(argv[2], NULL, 0);
  }
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  mo = newSWFMovieWithVersion(outputVersion);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 12.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); /* 1st frame */

  add_actions(mo,
		  " if(check == 1) "
		  " {"
		  "  _root.note('root frame '+_root._currentframe+'. About to call gotoAndPlay(4)');"
		  "  gotoAndPlay(4);"
		  " } else {"
		  "  _root.note('root frame '+_root._currentframe);"
		  " }");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  add_actions(mo, " _root.note('root frame '+_root._currentframe+'. About to call gotoAndPlay(2).');");
  add_actions(mo, " check = 1; gotoAndPlay(2); ");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  
  mc1 = newSWFMovieClip(); // will only exist in frame4
  add_clip_actions(mc1, 
  	" _root.note('about to invoke _root.gotoAndStop(6)');"
    " _root.gotoAndStop(6);"
    " _root.note('mc1 actions still running after _root.gotoAndStop(6), _root is '+_root);"
    " _root.x = 100; " );
  SWFMovieClip_nextFrame(mc1);
  
  /* add mc1 to _root and name it as "mc1" */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 3); 
  SWFDisplayItem_setName(it1, "mc1"); 
  add_actions(mo, "_root.note('root frame '+_root._currentframe);");
  check_equals(mo,"typeof(_root.x)", "'undefined'");
  add_actions(mo, "_root.x = 200; ");

  SWFMovie_nextFrame(mo); /* 4th frame */


  /* Remove mc1 in frame 4 (from depth 3) */
  SWFDisplayItem_remove(it1);
  
  mc2 = newSWFMovieClip();

  /* these actions are expected to be skipped with SWF version higher then 4 */
  add_clip_actions(mc2, " _root.note('mc2 frame '+this._currentframe);");
  add_clip_actions(mc2, " _root.xfail('This actions should not be executed with SWF5+');"); 
  SWFMovieClip_nextFrame(mc2);

  /* Place mc2 in frame 4 (at depth 3) */
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "mc2"); 

  add_actions(mo, " _root.note('root frame '+_root._currentframe);");
  add_actions(mo, " stop(); ");  
   
  SWFMovie_nextFrame(mo); /* 5th frame */

  /* Remove mc2 in frame 5 (from depth 3) */
  SWFDisplayItem_remove(it2);

  check_equals(mo, "_root.x", "200");
  add_actions(mo, " _root.note('root frame '+_root._currentframe);");
  add_actions(mo, " _root.totals(); stop(); ");

  SWFMovie_nextFrame(mo); /* 6th frame */
  //Output movie
  char outputFilename[256];
  snprintf(outputFilename, 255, OUTPUT_FILENAME_FMT, outputVersion);
  printf("Saving %s\n", outputFilename );
  SWFMovie_save(mo, outputFilename);

  return 0;
}



