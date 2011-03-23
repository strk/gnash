/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * movieclips hiberarchy:
 *
 *   _root.frame2.mc1;  
 *   _root.frame2.mc2;  
 
 *     mc1.frame2.mc11
 *     mc1.frame4.mc12
 *     mc1.frame5.DoAction
 *
 *       mc2.frame3.mc21
 *       mc2.frame5.DoAction
 * 
 *       mc11.frame4.DoAction
 *       mc12.frame2.DoAction
 *       mc21.frame3.DoAction
 *
 * This file is complex, but completely sane.
 * It is used to test complex actions order. If passed, then congratulations:)!
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "action_execution_order_test11.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc11, mc12, mc2, mc21, dejagnuclip;
  SWFDisplayItem it1, it11, it12, it2, it21;

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
  add_actions(mo, " _root.loadOrder = '0+'; "
                  " _root.enterFrameOrder = '0+'; "
                  " _root.unloadOrder = '0+'; "
                  " _root.doActionOrder = '0+'; "
                  " _root.asOrder = '0+'; ");
  SWFMovie_nextFrame(mo); // _root frame 1

  /*===================== Start of defining movieClips ==========================*/
  mc11 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc11);  // mc11 frame1
  SWFMovieClip_nextFrame(mc11);  // mc11 frame2
  SWFMovieClip_nextFrame(mc11);  // mc11 frame3
  add_clip_actions(mc11, "_root.doActionOrder += '3+'; "
                         "_root.asOrder += '27+';");
  SWFMovieClip_nextFrame(mc11);  // mc11 frame4


  mc12 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc12); // mc12 frame1
  add_clip_actions(mc12, "_root.doActionOrder += '1+'; "
                         "_root.asOrder += '25+'; ");
  SWFMovieClip_nextFrame(mc12); // mc12 frame2
    
  mc1 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc1);  // mc1 frame1
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11); 
  SWFDisplayItem_setName(it11, "mc11"); 
  // clip actions for mc11
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onLoad called'); "
                         " _root.loadOrder += '3+'; "
                         " _root.asOrder += '5+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onEnterFrame called'); "
                         " _root.enterFrameOrder += '3+'; "
                         " _root.asOrder += '6+';"),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onUnload called'); "
                         " _root.unloadOrder += '1+'; "
                         " _root.asOrder += '20+';"),
    SWFACTION_UNLOAD);
  SWFMovieClip_nextFrame(mc1);  // mc1 frame2
  SWFMovieClip_nextFrame(mc1);  // mc1 frame3
  it12 = SWFMovieClip_add(mc1, (SWFBlock)mc12); 
  SWFDisplayItem_setName(it12, "mc12"); 
  // clip actions for mc12
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onLoad called'); "
                         "  _root.loadOrder += '5+'; "
                         " _root.asOrder += '14+';"),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onEnterFrame called'); "
                         " _root.enterFrameOrder += '5+'; "
                         " _root.asOrder += '15+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onUnload called'); "
                         " _root.unloadOrder += '2+'; "
                         " _root.asOrder += '21+'; "),
    SWFACTION_UNLOAD);
  SWFMovieClip_nextFrame(mc1);  // mc1 frame4
  add_clip_actions(mc1, "_root.doActionOrder += '5+';"
                        " _root.asOrder += '29+'; ");
  SWFMovieClip_nextFrame(mc1);  // mc1 frame5
  SWFMovieClip_nextFrame(mc1);  // mc1 frame6
  
  mc21 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc21); // mc21 frame1
  SWFMovieClip_nextFrame(mc21); // mc21 frame2
  add_clip_actions(mc21, "_root.doActionOrder += '2+';"
                         "_root.asOrder += '26+'; ");
  SWFMovieClip_nextFrame(mc21); // mc21 frame3
  
  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2);  // mc2 frame1
  SWFMovieClip_nextFrame(mc2);  // mc2 frame2
  it21 = SWFMovieClip_add(mc2, (SWFBlock)mc21); 
  SWFDisplayItem_setName(it21, "mc21"); 
  // clip actions for mc21
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onLoad called'); "
                         " _root.loadOrder += '4+'; "
                         " _root.asOrder += '8+';"),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onEnterFrame called'); "
                         "  _root.enterFrameOrder += '4+'; "
                         " _root.asOrder += '10+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onUnload called'); "
                         " _root.unloadOrder += '4+'; "
                         " _root.asOrder += '23+'; "),
    SWFACTION_UNLOAD);
  SWFMovieClip_nextFrame(mc2);  // mc2 frame3
  SWFMovieClip_nextFrame(mc2);  // mc2 frame4
  add_clip_actions(mc2, "_root.doActionOrder += '4+';"
                        "_root.asOrder += '28+';");
  SWFMovieClip_nextFrame(mc2);  // mc2 frame5
  SWFMovieClip_nextFrame(mc2);  // mc2 frame6
  
  // Place mc1 and mc2 and add clipEvents for them.
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setName(it1, "mc1"); 
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setName(it2, "mc2");   
  
  // clip actions for mc1
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onLoad called'); "
                         " _root.loadOrder += '1+'; "
                         " _root.asOrder += '1+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onEnterFrame called'); "
                         " _root.enterFrameOrder += '1+'; "
                         " _root.asOrder += '4+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onUnload called'); "
                         " _root.unloadOrder += '3+'; "
                         " _root.asOrder += '22+'; "),
    SWFACTION_UNLOAD);

  
  // clip actions for mc2
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onLoad called'); "
                         " _root.loadOrder += '2+'; "
                         " _root.asOrder += '2+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onEnterFrame called'); "
                         " _root.enterFrameOrder += '2+'; "
                         " _root.asOrder += '3+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onUnload called'); "
                         " _root.unloadOrder += '5+';"
                         " _root.asOrder += '24+'; "),
    SWFACTION_UNLOAD);
  
  SWFMovie_nextFrame(mo); // _root frame 2
  /*===================== End of defining movieClips ==========================*/
  
  SWFMovie_nextFrame(mo); // _root frame3
  
  SWFMovie_nextFrame(mo); // _root frame4
  
  SWFMovie_nextFrame(mo); // _root frame5
  
  SWFMovie_nextFrame(mo); // _root frame6

  SWFDisplayItem_remove(it1);  
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); // _root frame7
  
  check_equals(mo, "_root.loadOrder", "'0+1+2+3+4+5+'");
  check_equals(mo, "_root.enterFrameOrder", "'0+2+1+3+2+1+4+3+2+1+5+4+3+2+1+'");
  check_equals(mo, "_root.unloadOrder", "'0+1+2+3+4+5+'");
  check_equals(mo, "_root.doActionOrder", "'0+1+2+3+4+5+'");
  check_equals(mo, "_root.asOrder", "'0+1+2+3+4+5+6+3+8+4+10+6+3+4+14+15+25+10+26+6+27+3+28+4+29+20+21+22+23+24+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); // _root frame8
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
