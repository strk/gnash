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


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "event_handler_scope_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc, dejagnuclip;
  SWFDisplayItem  it;
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
  add_actions(mo, "x1=0; x2=0; x3=0;");
  SWFMovie_nextFrame(mo);  /* 1st frame */

  
  mc = newSWFMovieClip();
  sh_red = make_fill_square (100, 200, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc); //frame1
  SWFMovieClip_nextFrame(mc); //frame2
  add_clip_actions(mc, " if (scope_test == 1); scope_test = 2; stop();");
  SWFMovieClip_nextFrame(mc); //frame3

  it = SWFMovie_add(mo, (SWFBlock)mc); 
  SWFDisplayItem_setDepth(it, 20); 
  SWFDisplayItem_setName(it, "mc"); 
  /* Define onClipEnterFrame */
  SWFDisplayItem_addAction(it,
    compileSWFActionCode(" _root.note('onClipEnterFrame triggered'); "
                         " var scope_test = 1; "), // Define mc.scope_test
    SWFACTION_ENTERFRAME);  
  /* Define onEnterFrame */
  add_actions(mo, " mc.onEnterFrame = function () "
                  " { _root.note('user defined onEnterFrame called'); "
                  "   scope_test = 3; "          // Define _root.scope_test 
                  " var scope_test = 4; }; " );  // Define a local var
  
  check_equals(mo, "_root.scope_test", "undefined");
  check_equals(mo, "_root.mc.scope_test", "undefined");
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  check_equals(mo, "_root.mc.scope_test", "1");
  check_equals(mo, "_root.scope_test", "3");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  check_equals(mo, "_root.mc.scope_test", "2");
  SWFMovie_nextFrame(mo); /* 4th frame */
  
  check_equals(mo, "_root.scope_test", "3");
  SWFMovie_nextFrame(mo); /* 5th frame */
  
  SWFDisplayItem_remove(it);
  check_equals(mo, "_root.mc.scope_test", "undefined");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 6th frame */
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



