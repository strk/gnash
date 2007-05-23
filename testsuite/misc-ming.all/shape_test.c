/* 
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "shape_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, dejagnuclip;
  SWFDisplayItem it;
  SWFShape  sh1,sh2;
  SWFAction ac1, ac2;
  int i;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      //fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      //return 1;
  }

  Ming_init();
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  //SWFMovie_setRate (mo, 1.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); 

  sh1 = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  sh2 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 0, 0, 0);
  
  it = SWFMovie_add(mo, (SWFBlock)sh1);  
  SWFDisplayItem_setName(it, "sh1"); 
  SWFDisplayItem_setDepth(it, 3); //place the sh1 character at depth 3;
  
  it = SWFMovie_add(mo, (SWFBlock)sh2);  
  SWFDisplayItem_setName(it, "sh2"); 
  SWFDisplayItem_setDepth(it, 4); //place the sh2 character at depth 4;

  check(mo, "sh1 != undefined");
  check(mo, "sh2 != undefined");
  
  add_actions(mo, 
  	// Do these checks mean that shapes are movieclips?
    "xcheck_equals(typeof(sh1), 'movieclip');"
    "xcheck_equals(typeof(sh2), 'movieclip');"
    
    "sh1.var1 = 10;"
    "sh2.var2 = 20;"
    // Do these checks mean that we can add variables to shapes?
    "check_equals(sh1.var1, 10);"
    "check_equals(sh2.var2, 20);"

    "xcheck_equals(sh1._x, 0);"
    "xcheck_equals(sh2._x, 0);"
    "sh1._x = 0;"
    "sh2._x = 400;"
    "xcheck_equals(sh1._x, 400);" //odd, why???
    "check_equals(sh2._x, 400);"
    
    // Do these checks mean that shapes are *not* movieclips?
    "check_equals(typeof(sh1.getDepth()), 'undefined');"
    "check_equals(typeof(sh2.getDepth()), 'undefined');"
    
    // Do these checks mean that shapes are *not* movieclips?
    "check_equals(typeof(getInstanceAtDepth(-16381)), 'undefined');"
    "check_equals(typeof(getInstanceAtDepth(-16380)), 'undefined');"
  );

  add_actions(mo, "_root.totals(); stop();");

  SWFMovie_nextFrame(mo); 

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
