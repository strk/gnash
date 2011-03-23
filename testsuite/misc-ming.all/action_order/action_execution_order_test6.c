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
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * To verify that:
 *    onClipConstruct, onClipLoad and onClipUnload are triggerd for passing-by MovieClips.
 *
 * 1st frm of _root
 *     place dejagunClip;
 * 2nd frm of _root: 
 *    gotoAndPlay(9);
 * 3rd frm of _root:
 *    place mc1 and  mc2;
 * 4th frm of _root:
 *    record the order of all triggered events and goto the 10th frame
 * 5th frm of _root
 *    gotoAndPlay(4);
 *    remove mc1 and mc2; 
 * 6th frm of _root
 *    place mc3;
 * 7th frm of _root
 *    do nothing;
 * 8th frm of _root
 *    remove mc3;
 * 9th frm of _root
 *    gotoAndPlay(5);
 * 10th frm of _root
 *    stop and check
 *
 * expected actions order:
 * At frame2 go forward to frame9:
 *   mc1.Construct 
 *   mc2.Construct
 *   mc3.Construct
 *   mc1.Load
 *   mc2.Load
 *   mc1.Unload
 *   mc2.Unload
 *   mc3.Load
 *   mc3.Unload
 * At frame9 go backward to frame5:
 *   (no events triggered)
 * At frame5 go backward to frame4:
 *   mc1.Construct
 *   mc2.Construct
 *   mc1.Load
 *     actions in 1st frame of mc1
 *   mc2.Load 
 *     actions in 1st frame of mc2
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  7
#define OUTPUT_FILENAME "action_execution_order_test6.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, mc3, dejagnuclip;
  SWFDisplayItem it1, it2, it3;

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
  SWFMovie_setRate (mo, 1);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); /* 1st frame */
  
  add_actions(mo, " _root.x1=''; gotoAndPlay(9);");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  mc1 = newSWFMovieClip();
  add_clip_actions(mc1, " _root.note('actions in 1st frame of mc1'); "
                        " _root.x1 += 'x+'; ");
  SWFMovieClip_nextFrame(mc1); /* mc1, 1st frame */
  add_clip_actions(mc1, " _root.note('actions in 2nd frame of mc1'); "
                        " _root.x1 += 'x+'; "
                        " stop(); ");
  SWFMovieClip_nextFrame(mc1); /* mc1, 2nd frame */
 
  mc2 = newSWFMovieClip();
  add_clip_actions(mc2, " _root.note('actions in 1st frame of mc2'); "
                        " _root.x1 += 'xx+'; "); 
  SWFMovieClip_nextFrame(mc2); /* mc2, 1st frame */
  add_clip_actions(mc2, " _root.note('actions in 2nd frame of mc2'); "
                        " _root.x1 += 'xx+'; "
                        " stop(); "); 
  SWFMovieClip_nextFrame(mc2); /* mc2, 2nd frame */
  
  mc3 = newSWFMovieClip();
  add_clip_actions(mc3, " _root.note('actions in 1st frame of mc3'); "
                        " _root.x1 += 'xxx+'; "); 
  SWFMovieClip_nextFrame(mc3); /* mc3, 1st frame */
  add_clip_actions(mc3, " _root.note('actions in 2nd frame of mc3'); "
                        " _root.x1 += 'xxx+'; "
                        " stop(); "); 
  SWFMovieClip_nextFrame(mc3); /* mc3, 2nd frame */
  
  
  /* add mc1 to _root and name it as "mc1" */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Construct called');"
                         " _root.x1 += '1+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Load called');"
                         " _root.x1 += '4+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Unload called'); "
                         " _root.x1 += '6+'; "),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 EnterFrame called'); "), 
    SWFACTION_ENTERFRAME);
    
  /* add mc2 to _root and name it as "mc2" */
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 12); 
  SWFDisplayItem_setName(it2, "mc2"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Construct called');"
                         " _root.x1 += '2+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Load called'); "
                         " _root.x1 += '5+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Unload called'); "
                         " _root.x1 += '7+'; "),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 EnterFrame called'); "),
    SWFACTION_ENTERFRAME);
    
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  add_actions(mo, " if(flag == 1) { _root.check_result = _root.x1; gotoAndStop(10); } ");
  SWFMovie_nextFrame(mo); /* 4th frame */
  
  add_actions(mo, " gotoAndPlay(4); ");
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  /* add mc3 to _root and name it as "mc3" */
  it3 = SWFMovie_add(mo, (SWFBlock)mc3);  
  SWFDisplayItem_setDepth(it3, 11); 
  SWFDisplayItem_setName(it3, "mc3"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Construct called');"
                         " _root.x1 += '3+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Load called'); "
                         " _root.x1 += '8+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Unload called'); "
                         " _root.x1 += '9+';" ),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 EnterFrame called'); "),
    SWFACTION_ENTERFRAME);
  
  SWFMovie_nextFrame(mo); /* 6th frame */
  SWFMovie_nextFrame(mo); /* 7th frame */

  SWFDisplayItem_remove(it3); 
  SWFMovie_nextFrame(mo); /* 8th frame */
  
  add_actions(mo, " gotoAndPlay(5); flag = 1;");
  SWFMovie_nextFrame(mo); /* 9th frame */
  
  xcheck_equals(mo, "_root.check_result", "'1+2+3+4+5+6+7+8+9+1+2+4+x+5+xx+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 10th frame */
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



