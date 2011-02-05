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
 */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  7
#define OUTPUT_FILENAME "opcode_guard_test.swf"

static void
my_error(const char *msg, ...)
{
        va_list args;

        va_start(args, msg);
        vprintf(msg, args);
        va_end(args);
        exit(1);
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc11, mc2, dejagnuclip;
  SWFDisplayItem it1, it11, it2;

  const char *srcdir=".";
  if ( argc>1 ) 
      srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  Ming_setErrorFunction(my_error);
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 1);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); // 1st frame 
  
  mc11 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc11); 
  
  mc1 = newSWFMovieClip();
    // should not be executed.
    add_clip_actions(mc1, "_root.xcheck(false);");
    // add child movieclip
    it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);  
    SWFDisplayItem_setDepth(it11, 1); 
    SWFDisplayItem_setName(it11, "mc11"); 
  SWFMovieClip_nextFrame(mc1); 
  

  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 
  /* Define Construct ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Construct called');"
                         // test child movieclip has been constructed(no unload handler defined).
                         " _root.check_equals(typeof(_root.mc1.mc11), 'movieclip');"
                         " _root.gotoAndPlay(3); "
                         " _root.testvar1 = 'executed'; "
                         // test child movieclip has been destroyed.
                         " _root.check_equals(typeof(_root.mc1.mc11), 'undefined');"),
    SWFACTION_CONSTRUCT);
  /* Define Load ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Load called');"
                         " _root.gotoAndPlay(3); "
                         " _root.testvar2 = 'executed'; "),
    SWFACTION_ONLOAD);
  /* Define Unload ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 Unload called'); "
                         " _root.gotoAndPlay(3); "
                         " _root.testvar3 = 'executed'; "
                         // test child movieclip has been destroyed(no unload handler defined)
                         " _root.check_equals(typeof(_root.mc1.mc11), 'undefined');"),
    SWFACTION_UNLOAD);
   /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it1,
    compileSWFActionCode(" _root.note('mc1 EnterFrame called'); "
                         " _root.gotoAndPlay(3); "
                         " _root.check(false); "),
    SWFACTION_ENTERFRAME);
  SWFMovie_nextFrame(mo); // 2nd frame
  
  
  SWFDisplayItem_remove(it1);
  SWFMovie_nextFrame(mo); // 3th frame
  
  check_equals(mo, "testvar1", "'executed'" );
  check_equals(mo, "testvar2", "'executed'" );
  check_equals(mo, "testvar3", "'executed'" );
  SWFMovie_nextFrame(mo); // 4th frame
  
  
  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2); 
  
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 12); 
  SWFDisplayItem_setName(it2, "mc2"); 

  /* Define EnterFrame ClipEvent */
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('mc2 EnterFrame called'); "
                         " _root.gotoAndPlay(8); "
                         " _root.xcheck(false); "), //should not be executed
    SWFACTION_ENTERFRAME);
  SWFMovie_nextFrame(mo); // 5th frame
  
  SWFMovie_nextFrame(mo); // 6th frame
  SWFDisplayItem_remove(it2);
  SWFMovie_nextFrame(mo); // 7th frame
  SWFMovie_nextFrame(mo); // 8th frame
  
  
  add_actions(mo,
    "orignal_target_var = 100;"
    "setTarget('non-exist-target');"
    "current_target = 0;"
    "asm{   "
    "   push  'current_target' "
    "   push  '' "
    "   push  11 " //_target   
    "   getproperty  "
    "   setvariable  "
    "}; "
    // non-exist target does not evaluated to _root!
    " _root.check_equals(current_target, undefined);"
    // No surprise, getVariable will ascend to _root!
    " _root.check_equals(_target, '/');"
    " _root.check_equals(_root._currentframe, 9);"
    " gotoAndPlay(10);"
    // the above gotoFrame has no effect as it was acting on an non-exist-target
    "   _root.check_equals(_root._currentframe, 9);"
    // ascend to the orignal target is the current target is undefined
    // we succeed by luck.
    "   _root.check_equals(orignal_target_var, 100);"
    "setTarget('');"); 
  SWFMovie_nextFrame(mo); // 9th frame
  
  
  add_actions(mo, "xtotals(11); stop();");
  SWFMovie_nextFrame(mo); // 10th frame
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
