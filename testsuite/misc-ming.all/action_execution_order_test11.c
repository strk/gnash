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
 * 
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
                  " _root.asOrder = '0+'; ");
  SWFMovie_nextFrame(mo); // frame 1

  /*===================== Start of defining movieClips ==========================*/
  mc11 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc11); 

  mc12 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc11); 
    
  mc1 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc1);  // frame 1
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11); 
  // clip actions for mc11
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onLoad called'); "
                         "  _root.loadOrder += '3+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onEnterFrame called'); "
                         " _root.enterFrameOrder += '3+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it11,
    compileSWFActionCode(" _root.note('mc11 onUnload called'); "
                         " _root.unloadOrder += '1+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it11, "mc11"); 
  SWFMovieClip_nextFrame(mc1);  // frame 2
  SWFMovieClip_nextFrame(mc1);  // frame 3
  it12 = SWFMovieClip_add(mc1, (SWFBlock)mc12); 
  // clip actions for mc12
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onLoad called'); "
                         "  _root.loadOrder += '5+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onEnterFrame called'); "
                         " _root.enterFrameOrder += '5+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it12,
    compileSWFActionCode(" _root.note('mc12 onUnload called'); "
                         " _root.unloadOrder += '2+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it12, "mc12"); 
  SWFMovieClip_nextFrame(mc1);  // frame 4
  SWFMovieClip_nextFrame(mc1);  // frame 5
  SWFMovieClip_nextFrame(mc1);  // frame 6
  
  mc21 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc21); 
  
  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2);  // frame 1
  SWFMovieClip_nextFrame(mc2);  // frame 2
  it21 = SWFMovieClip_add(mc2, (SWFBlock)mc21); 
  // clip actions for mc21
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onLoad called'); "
                         " _root.loadOrder += '4+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onEnterFrame called'); "
                         "  _root.enterFrameOrder += '4+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it21,
    compileSWFActionCode(" _root.note('mc21 onUnload called'); "
                         " _root.unloadOrder += '4+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it21, "mc21"); 
  SWFMovieClip_nextFrame(mc2);  // frame 3
  SWFMovieClip_nextFrame(mc2);  // frame 4
  SWFMovieClip_nextFrame(mc2);  // frame 5
  SWFMovieClip_nextFrame(mc2);  // frame 6
  
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);   
  
  // clip actions for mc1
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onLoad called'); "
                         " _root.loadOrder += '1+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onEnterFrame called'); "
                         " _root.enterFrameOrder += '1+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 onUnload called'); "
                         " _root.unloadOrder += '3+'; "),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it1, "mc1"); 
  
  // clip actions for mc2
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onLoad called'); "
                         " _root.loadOrder += '2+'; "),
    SWFACTION_ONLOAD);
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onEnterFrame called'); "
                         " _root.enterFrameOrder += '2+'; "),
    SWFACTION_ENTERFRAME);
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 onUnload called'); "
                         " _root.unloadOrder += '5+';"),
    SWFACTION_UNLOAD);
  SWFDisplayItem_setName(it2, "mc2"); 
  
  SWFMovie_nextFrame(mo); // frame 2
  /*===================== End of defining movieClips ==========================*/
  
  SWFMovie_nextFrame(mo); // frame 3
  
  SWFMovie_nextFrame(mo); // frame 4
  
  SWFMovie_nextFrame(mo); // frame 5
  
  SWFMovie_nextFrame(mo); // frame 6

  SWFDisplayItem_remove(it1);  
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); // frame 7
  
  check_equals(mo, "_root.loadOrder", "'0+1+2+3+4+5+'");
  check_equals(mo, "_root.enterFrameOrder", "'0+2+1+3+2+1+4+3+2+1+5+4+3+2+1+'");
  check_equals(mo, "_root.unloadOrder", "'0+1+2+3+4+5+'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); // frame 8
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



