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
 * movieClip hiberarchy:
 * main timeline  (5 frames)
 *     |----dejagnuclip(placed at 1st frame of main timeline)
 *     |----mc_red     (placed at 2nd frame of main timeline, and removed at 4th frame)
 *             |----mc_blu(placed at 1st frame of mc_red)
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "action_execution_order_extend_test.swf"


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
  add_actions(mo, " trace('as in frame1 of root');"); // can't use 'note' here, as it's not been defined yet
  SWFMovie_nextFrame(mo); /* 1st frame */

  mc_blu = newSWFMovieClip();
  sh_blu = make_fill_square (20, 320, 20, 20, 0, 0, 255, 0, 0, 255);
  SWFMovieClip_add(mc_blu, (SWFBlock)sh_blu);  
  add_clip_actions(mc_blu, " _root.note('as in frame1 of mc_blu'); _root.x1 = \"as_in_mc_blu\"; ");
  SWFMovieClip_nextFrame(mc_blu); /* 1st frame */
  add_clip_actions(mc_blu, " _root.note('as in frame2 of mc_blu'); _root.x2 = \"as_in_mc_blu\"; stop();");
  SWFMovieClip_nextFrame(mc_blu); /* 2nd frame */
  
  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  /* Add mc_blu to mc_red and name it as "mc_blu" */
  it_blu = SWFMovieClip_add(mc_red, (SWFBlock)mc_blu);  
#if 0 // adding *any* clip-event handler makes user-defined onLoad execute !
  SWFDisplayItem_addAction(it_blu,
		compileSWFActionCode("_root.note('mc_blu clip unload executed'); "
		), SWFACTION_UNLOAD);
#endif
  SWFDisplayItem_setDepth(it_blu, 1000); 
  SWFDisplayItem_setName(it_blu, "mc_blu");
#if 1 /* setting ratio doesn't change the fact we won't execute user-defined onLoad event handler
       * if no clip handlers are defined */
  SWFDisplayItem_setRatio(it_blu, 0);
#endif
  add_clip_actions(mc_red, "_root.note('as in frame1 of mc_red'); _root.x1 = \"as_in_mc_red\"; ");
  add_clip_actions(mc_red, " func = function() {}; ");
  SWFMovieClip_nextFrame(mc_red); /* 1st frame */
  add_clip_actions(mc_red, " _root.note('as in frame2 of mc_red'); _root.x2 = \"as_in_mc_red\"; stop();");
  SWFMovieClip_nextFrame(mc_red); /* 2nd frame */
  
  /* Add mc_red to _root and name it as "mc_red" */
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  
  SWFDisplayItem_setDepth(it_red, 20); 
  SWFDisplayItem_setName(it_red, "mc_red");
  SWFDisplayItem_addAction(it_red,
		compileSWFActionCode("_root.note('mc_red clip load executed'); "
			"_root.y1bis = 'mc_red onClipLoad called';"),
		SWFACTION_ONLOAD);
  /* Woo, the PlaceObject tag hasn't defined an 'onLoad' function.
		 maybe just pushed something to the action list???
	*/
  check_equals(mo, "typeOf(_root.mc_red.onLoad)", "'undefined'");
  add_actions(mo, " note('as in frame2 of root'); var x1 = \"as_in_root\"; ");
  add_actions(mo, " _root.mc_red.onLoad = function () \
                   { \
                       note('mc_red load executed'); \
                       _root.y1 = 'mc_red onLoad called'; \
                   }; \
                   _root.mc_red.onEnterFrame = function () \
                   { \
                       note('mc_red enterFrame executed'); \
                       _root.y2 = 'mc_red onEnterFrame called'; \
                   }; \
                   _root.mc_red.onUnload = function () \
                   { \
                       note('mc_red unload executed'); \
                        _root.y3 = 'mc_red onUnload called'; \
                   }; \
                   _root.mc_red.mc_blu.onLoad = function () \
                   { \
                       note('mc_blu load executed'); \
                       _root.y4 = 'mc_blu onLoad called'; \
                   }; \
                   _root.mc_red.mc_blu.onEnterFrame = function () \
                   { \
                       note('mc_blu enterFrame executed'); \
                        _root.y5 = 'mc_blu onEnterFrame called'; \
                   }; \
                   _root.mc_red.mc_blu.onUnload = function () \
                   { \
                       note('mc_blu user-defined UNLOAD executed'); \
                       _root.y6 = 'mc_blu onUnload called'; \
                   };");

  /*
   * Check that the DisplayList is initialized deep to the mc_blu level
   * Even if their actions are not expected to be executed yet
   */
  check_equals(mo, "typeOf(_root.mc_red)", "'movieclip'");
  check_equals(mo, "typeOf(_root.mc_red.func)", "'undefined'");
  check_equals(mo, "typeOf(_root.mc_red.mc_blu)", "'movieclip'");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  
  add_actions(mo, " note('as in frame3 of root'); \
                    _root.x2 = 'as_in_root'; \
                    _root.y2 = 'as_in_root'; \
                    _root.y5 = 'as_in_root';");
  check_equals(mo, "typeOf(_root.mc_red.func)", "'function'");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  
  SWFDisplayItem_remove(it_red);
  add_actions(mo, " note('as in frame4 of root'); \
                    _root.y3 = 'as_in_root'; \
                    _root.y6 = 'as_in_root'; ");
  /* In the frame placing mc_red,  actions in mc_red is executed *after* actions in _root */
  check_equals(mo, "_root.x1", "'as_in_mc_blu'");
  /* In subsequent frames, actions in mc_red is executed *before* actions in _root */
  check_equals(mo, "_root.x2", "'as_in_root'");
  SWFMovie_nextFrame(mo); /* 4th frame */
  
  /* mc_red onload is only called IFF onClipEvent(load) is also defined! */
  check_equals(mo, "_root.y1", "'mc_red onLoad called'");
  check_equals(mo, "_root.y1bis", "'mc_red onClipLoad called'");
  /* actions in main timeline is executed *after* mc_red.onEnterFrame */
  check_equals(mo, "_root.y2", "'as_in_root'");
  /* actions in main timeline is executed *before* mc_red.onUnload in
     this testcase, but I don't believe it is alway true. The actually order
     may dependent on order of tags. It seems that Ming always place REMOVE_OBJECT
     tag after DO_ACTION Tag. What if the order is reversed(I can't perform this 
     with Ming)?? */
  check_equals(mo, "_root.y3", "'mc_red onUnload called'");
  /* mc_blu Onload is not called */
  check_equals(mo, "_root.y4", "undefined");
  /* actions in main timeline is executed *after* mc_blu.onEnterFrame */
  check_equals(mo, "_root.y5", "'as_in_root'");
  /* actions in main timeline is executed *before* mc_blu.onUnload */
  check_equals(mo, "_root.y6", "'mc_blu onUnload called'");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



