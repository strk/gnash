/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
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
 * To verify that for all movieClips within the same timeline: 
 *    (1) OnLoad: first added first called; 
 *    (2) OnUnload: first removed first called; 
 *    (3) OnEnterFrame: last added first called; 
 *
 * 1st frame of _root: 
 *    place mc_red1 at depth 10
 *    place mc_red2 at depth 12
 *    place mc_red3 at depth 11
 *
 * 4th frame of _root:
 *     remove mc_red1
 *     remove mc_red2 
 *     remove mc_red3 
 *
 * expected actions order:
 *     mc_red1.OnLoad; actions in 1st frame of mc_red1;
 *     mc_red2.OnLoad; actions in 1st frame of mc_red2;
 *     mc_red3.OnLoad; actions in 1st frame of mc_red3;
 *     mc_red3.OnEnterFrame; actions in 2nd frame of mc_red3;
 *     mc_red2.OnEnterFrame; actions in 2nd frame of mc_red2;
 *     mc_red1.OnEnterFrame; actions in 2nd frame of mc_red1;
 *     mc_red1.OnUnload; 
 *     mc_red2.OnUnload; 
 *     mc_red3.OnUnload; 
 *
 * The actual order of tags are dependent on compiler, so you need to 
 * verify first if the order of tags is what you expect. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_test4.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red1, mc_red2, mc_red3, mc_red4, mc_red5, dejagnuclip;
  SWFDisplayItem it_red1, it_red2, it_red3, it_red4, it_red5;
  SWFShape  sh_red;

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
  SWFMovie_nextFrame(mo); /* 1st frame */

  
  mc_red1 = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red1, (SWFBlock)sh_red);  
  add_clip_actions(mc_red1, " _root.note('actions in 1st frame of mc_red1'); "
                            " _root.x1 += '2+'; ");
  SWFMovieClip_nextFrame(mc_red1); /* mc_red1, 1st frame */
  add_clip_actions(mc_red1, " _root.note('actions in 2nd frame of mc_red1'); "
                            " _root.x1 += '12+'; "
                            " stop(); ");
  SWFMovieClip_nextFrame(mc_red1); /* mc_red1, 2nd frame */
 
  mc_red2 = newSWFMovieClip();
  sh_red = make_fill_square (80, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red2, (SWFBlock)sh_red); 
  add_clip_actions(mc_red2, " _root.note('actions in 1st frame of mc_red2'); "
                            " _root.x1 += '4+'; "); 
  SWFMovieClip_nextFrame(mc_red2); /* mc_red2, 1st frame */
  add_clip_actions(mc_red2, " _root.note('actions in 2nd frame of mc_red2'); "
                            " _root.x1 += '10+'; "
                            " stop(); "); 
  SWFMovieClip_nextFrame(mc_red2); /* mc_red2, 2nd frame */
  
  mc_red3 = newSWFMovieClip();
  sh_red = make_fill_square (160, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red3, (SWFBlock)sh_red);  
  add_clip_actions(mc_red3, " _root.note('actions in 1st frame of mc_red3'); "
                            " _root.x1 += '6+';"); 
  SWFMovieClip_nextFrame(mc_red3); /* mc_red3, 1st frame */
  add_clip_actions(mc_red3, " _root.note('actions in 2nd frame of mc_red3'); "
                            " _root.x1 += '8+'; "
                            " stop(); "); 
  SWFMovieClip_nextFrame(mc_red3); /* mc_red3, 2nd frame */
  
  
  /* add mc_red1 to _root and name it as "mc_red1" */
  it_red1 = SWFMovie_add(mo, (SWFBlock)mc_red1);  
  SWFDisplayItem_setDepth(it_red1, 10); 
  SWFDisplayItem_setName(it_red1, "mc_red1"); 
  /* Define onLoad ClipEvent */
  SWFDisplayItem_addAction(it_red1,
    compileSWFActionCode(" _root.note('mc_red1 onLoad called');"
                         " _root.x1 += '1+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it_red1,
    compileSWFActionCode(" _root.note('mc_red1 onUnload called'); "
                         " _root.x1 += '13+'; "),
    SWFACTION_UNLOAD);
  /* Define onEnterFrame ClipEvent */
  SWFDisplayItem_addAction(it_red1,
    compileSWFActionCode(" _root.note('mc_red1 onEnterFrame called'); "
                         " _root.x1 += '11+'; "),
    SWFACTION_ENTERFRAME);
    
  /* add mc_red2 to _root and name it as "mc_red2" */
  it_red2 = SWFMovie_add(mo, (SWFBlock)mc_red2);  
  SWFDisplayItem_setDepth(it_red2, 12); 
  SWFDisplayItem_setName(it_red2, "mc_red2"); 
  /* Define onLoad ClipEvent */
  SWFDisplayItem_addAction(it_red2,
    compileSWFActionCode(" _root.note('mc_red2 onLoad called'); "
                         " _root.x1 += '3+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it_red2,
    compileSWFActionCode(" _root.note('mc_red2 onUnload called'); "
                         " _root.x1 += '14+'; "),
    SWFACTION_UNLOAD);
  /* Define onEnterFrame ClipEvent */
  SWFDisplayItem_addAction(it_red2,
    compileSWFActionCode(" _root.note('mc_red2 onEnterFrame called'); "
                         " _root.x1 += '9+'; "),
    SWFACTION_ENTERFRAME);
    
  /* add mc_red3 to _root and name it as "mc_red3" */
  it_red3 = SWFMovie_add(mo, (SWFBlock)mc_red3);  
  SWFDisplayItem_setDepth(it_red3, 11); 
  SWFDisplayItem_setName(it_red3, "mc_red3"); 
  /* Define onLoad ClipEvent */
  SWFDisplayItem_addAction(it_red3,
    compileSWFActionCode(" _root.note('mc_red3 onLoad called'); "
                         " _root.x1 += '5+'; "),
    SWFACTION_ONLOAD);
  /* Define onUnload ClipEvent */
  SWFDisplayItem_addAction(it_red3,
    compileSWFActionCode(" _root.note('mc_red3 onUnload called'); "
                         " _root.x1 += '15+';" ),
    SWFACTION_UNLOAD);
  /* Define onEnterFrame ClipEvent */
  SWFDisplayItem_addAction(it_red3,
    compileSWFActionCode(" _root.note('mc_red3 onEnterFrame called'); "
                         " _root.x1 += '7+'; "),
    SWFACTION_ENTERFRAME);
    
  SWFMovie_nextFrame(mo); /* 2nd frame */


  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  /* It's no use to change the order below.
  After compile, Ming will re-organize them as 
  remove mc_red1; remove mc_red2; remove mc_red3;*/
  SWFDisplayItem_remove(it_red3);  
  SWFDisplayItem_remove(it_red1);
  SWFDisplayItem_remove(it_red2);
  SWFMovie_nextFrame(mo); /* 4th frame */

  xcheck_equals(mo, "_root.x1", "'1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



