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
#define OUTPUT_FILENAME  "duplicate_movie_clip_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFDisplayItem it1, it2;
  SWFShape  sh_red;

  /* For the button duplication test */
#if MING_VERSION_CODE >= 00040400
  SWFButton but;
  SWFButtonRecord br;
#endif

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

  
  mc1 = newSWFMovieClip();
  sh_red = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh_red);  
  add_clip_actions(mc1, "stop();");
  SWFMovieClip_nextFrame(mc1);

  mc2 = newSWFMovieClip();
  sh_red = make_fill_square (100, 200, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc2, (SWFBlock)sh_red);  
  add_clip_actions(mc2, "stop();");
  SWFMovieClip_nextFrame(mc2);

  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 

  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 20); 
  SWFDisplayItem_setName(it2, "mc2"); 
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('onClipLoad triggered'); "
                         " _root.x1 = _root.x1 + 1; "),
    SWFACTION_ONLOAD);  
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('onClipEnterFrame triggered'); "
                         " _root.x2 = _root.x2 + 1; "),
    SWFACTION_ENTERFRAME);  
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('onClipUnload triggered'); "
                         " _root.x3 = _root.x3 + 1; "),
    SWFACTION_UNLOAD); 

  add_actions(mo, " mc1.onLoad = function () {}; "
                  " mc1.prop1 = 10; "
                  " duplicateMovieClip('mc1', 'dup1', 1); "
                  " mc2.onLoad = function () {}; "
                  " duplicateMovieClip('mc2', 'dup2', 2); " );
  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  
  check_equals(mo, "mc1.prop1", "10");
  check_equals(mo, "typeof(mc1.onLoad)", "'function'");
  check_equals(mo, "mc1.getDepth()", "-16374");
  /* user defined property will not be duplicated */
  check_equals(mo, "dup1.prop1", "undefined");  
  /* user defined event handler will not be duplicated */
  check_equals(mo, "typeof(dup1.onLoad)", "'undefined'"); 
  check_equals(mo, "dup1.getDepth()", "1");
  /* check user defined onLoad */
  check_equals(mo, "typeof(mc2.onLoad)", "'function'");
  /* onClipEvent does not define a function */
  check_equals(mo, "typeof(mc2.onEnterFrame)", "'undefined'");
  /* user defined event handler will not be duplicated */
  check_equals(mo, "typeof(dup2.onLoad)", "'undefined'"); 
  check_equals(mo, "_root.x1", "2");
  check_equals(mo, "_root.x2", "2");
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  SWFDisplayItem_remove(it1);
  SWFDisplayItem_remove(it2);
  add_actions(mo, " dup2.removeMovieClip(); ");

  SWFMovie_nextFrame(mo); /* 4th frame */
  
#if MING_VERSION_CODE >= 00040400

  /* Create a button, add it to mc1 */
  but = newSWFButton();
  br = SWFButton_addCharacter(but, (SWFCharacter)sh_red, SWFBUTTON_UP);
  SWFButtonRecord_setDepth(br, 10);
  it1 = SWFMovie_add(mo, (SWFBlock)but);
  SWFDisplayItem_setName(it1, "button");

  /* Sanity check */
  check_equals(mo, "typeof(button)", "'object'");

  add_actions(mo,
          "trace(button);"
          "dupl = MovieClip.prototype.duplicateMovieClip;"
          "button.dupl = dupl;"
          "o = { x: 4 };"
          "d = button.dupl('buttdup', 201, o);"
          );
  
  xcheck_equals(mo, "typeof(d)", "'object'");
  xcheck_equals(mo, "'' + _root.buttdup", "'_level0.buttdup'");
  check_equals(mo, "_root.buttdup", "d");
  
  /* initobj not used */
  check_equals(mo, "_root.buttdup.x", "undefined");
#endif

  add_actions(mo,
          "t = new Object();"
          "t.dupl = dupl;"
          "o2 = { x: 44 };"
          "d2 = t.dupl('objdup', 202, o2);"
          "trace(_root.objdup);"
          );

  /* Does not work on plain objects */
  check_equals(mo, "typeof(d2)", "'undefined'");
  check_equals(mo, "typeof(_root.objdup)", "'undefined'");

  SWFMovie_nextFrame(mo); /* 5th frame */

  check_equals(mo, "_root.x1", "2");
  check_equals(mo, "_root.x2", "3");
  check_equals(mo, "_root.x3", "2");  
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



