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
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * To verify that for all movieClips within the same timeline: 
 *    (1) OnLoad: first added first called; 
 *    (2) OnUnload: first removed first called; 
 *    (3) OnEnterFrame: last added first called; 
 *
 * 1st frame of _root: 
 *    place mc1 at depth 10
 *    place mc2 at depth 12
 *    place mc3 at depth 11
 *
 * 4th frame of _root:
 *     remove mc1
 *     remove mc2 
 *     remove mc3 
 *
 * expected actions order:
 *     actions in 1st frame of _root; _root.OnLoad;
 * (advanced to 2nd frame of _root)
 *     mc1.OnConstruct; mc2.OnConstruct; mc3.OnConstruct;
 *     mc1.OnLoad; actions in 1st frame of mc1;
 *     mc2.OnLoad; actions in 1st frame of mc2;
 *     mc3.OnLoad; actions in 1st frame of mc3;
 *     mc3.OnEnterFrame; actions in 2nd frame of mc3;
 *     mc2.OnEnterFrame; actions in 2nd frame of mc2;
 *     mc1.OnEnterFrame; actions in 2nd frame of mc1;
 *     mc1.OnUnload; 
 *     mc2.OnUnload; 
 *     mc3.OnUnload; 
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
  SWFMovieClip  mc1, mc2, mc3, dejagnuclip;
  SWFDisplayItem it1, it2, it3;
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
  add_actions(mo, " x2 += 'as_start+'; "
                  " _root.OnLoad = function () { _root.note('_root onLoad called'); x2 += 'load_called+'; }; "
                  " x2 += 'as_end+'; "
                  " _root.onEnterFrame = function () { x3 += 'enterFrame_called+'; }; ");
  SWFMovie_nextFrame(mo); /* 1st frame */

  
  mc1 = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh_red);  
  add_clip_actions(mc1, " _root.note('actions in 1st frame of mc1'); "
			// Defining onLoad in first frame of a sprite doesn't work, but works for root
			" this.onLoad = function() { _root.note('mc1 onLoad called'); _root.x1 += 'YY'; };"
                        " _root.x1 += '2+'; ");
  SWFMovieClip_nextFrame(mc1); /* mc1, 1st frame */
  add_clip_actions(mc1, " _root.note('actions in 2nd frame of mc1'); "
                        " _root.x1 += '12+'; "
                        " stop(); ");
  SWFMovieClip_nextFrame(mc1); /* mc1, 2nd frame */
 
  mc2 = newSWFMovieClip();
  sh_red = make_fill_square (80, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh_red); 
  add_clip_actions(mc2, " _root.note('actions in 1st frame of mc2'); "
			// Defining onLoad in first frame of a sprite doesn't work, but works for root
			" this.onLoad = function() { _root.note('mc2 onLoad called'); _root.x1 += 'XX'; };"
                        " _root.x1 += '4+'; "); 
  SWFMovieClip_nextFrame(mc2); /* mc2, 1st frame */
  add_clip_actions(mc2, " _root.note('actions in 2nd frame of mc2'); "
                        " _root.x1 += '10+'; "
                        " stop(); "); 
  SWFMovieClip_nextFrame(mc2); /* mc2, 2nd frame */
  
  mc3 = newSWFMovieClip();
  sh_red = make_fill_square (160, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc3, (SWFBlock)sh_red);  
  add_clip_actions(mc3, " _root.note('actions in 1st frame of mc3'); "
			// Defining onLoad in first frame of a sprite doesn't work, but works for root
			" this.onLoad = function() { _root.note('mc3 onLoad called'); _root.x1 += 'ZZ'; };"
                        " _root.x1 += '6+';"); 
  SWFMovieClip_nextFrame(mc3); /* mc3, 1st frame */
  add_clip_actions(mc3, " _root.note('actions in 2nd frame of mc3'); "
                        " _root.x1 += '8+'; "
                        " stop(); "); 
  SWFMovieClip_nextFrame(mc3); /* mc3, 2nd frame */
  
  
  /* add mc1 to _root and name it as "mc1" */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Construct called');"
                         " _root.x0 += '01+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Load called');"
                         " _root.x1 += '1+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Unload called'); "
                         " _root.x1 += '13+'; "),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 EnterFrame called'); "
                         " _root.x1 += '11+'; "),
    SWFACTION_ENTERFRAME);
    
  /* add mc2 to _root and name it as "mc2" */
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 12); 
  SWFDisplayItem_setName(it2, "mc2"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Construct called');"
                         " _root.x0 += '02+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Load called'); "
                         " _root.x1 += '3+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 Unload called'); "
                         " _root.x1 += '14+'; "),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 EnterFrame called'); "
                         " _root.x1 += '9+'; "),
    SWFACTION_ENTERFRAME);
    
  /* add mc3 to _root and name it as "mc3" */
  it3 = SWFMovie_add(mo, (SWFBlock)mc3);  
  SWFDisplayItem_setDepth(it3, 11); 
  SWFDisplayItem_setName(it3, "mc3"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Construct called');"
                         " _root.x0 += '03+'; "),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Load called'); "
                         " _root.x1 += '5+'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 Unload called'); "
                         " _root.x1 += '15+';" ),
    SWFACTION_UNLOAD);
  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it3,
    compileSWFActionCode(" _root.note('mc3 EnterFrame called'); "
                         " _root.x1 += '7+'; "),
    SWFACTION_ENTERFRAME);
  
  add_actions(mo, " _root.x3 += '_root_frm2_as+'; ");
  SWFMovie_nextFrame(mo); /* 2nd frame */

  add_actions(mo, " _root.x3 += '_root_frm3_as+'; ");
  check_equals(mo, "_root.x3", "'enterFrame_called+_root_frm2_as+enterFrame_called+_root_frm3_as+'");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  /* It's no use to change the order below.
  After compile, Ming will re-organize them as 
  remove mc1; remove mc2; remove mc3;*/
  SWFDisplayItem_remove(it3);  
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); /* 4th frame */

  check_equals(mo, "_root.x0", "'01+02+03+'");
  check_equals(mo, "_root.x1", "'1+2+3+4+5+6+7+8+9+10+11+12+13+14+15+'");
  check_equals(mo, "_root.x2", "'as_start+as_end+load_called+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



