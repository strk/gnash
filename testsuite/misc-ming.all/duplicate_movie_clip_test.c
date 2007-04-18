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
  SWFMovieClip  mc, dejagnuclip;
  SWFDisplayItem it;
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
  SWFMovie_nextFrame(mo); 

  
  mc = newSWFMovieClip();
  sh_red = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc, (SWFBlock)sh_red);  
  add_clip_actions(mc, "stop();");
  SWFMovieClip_nextFrame(mc);


  it = SWFMovie_add(mo, (SWFBlock)mc); 
  SWFDisplayItem_setDepth(it, 10); 
  SWFDisplayItem_setName(it, "mc"); 
  
  add_actions(mo, " mc.onLoad = function () {};"
                  " mc.prop1 = 10; "
                  " duplicateMovieClip('mc', 'dup', 1); ");
  SWFMovie_nextFrame(mo); 
  
  
  check_equals(mo, "mc.prop1", "10");
  check_equals(mo, "typeof(mc.onLoad)", "'function'");
  check_equals(mo, "mc.getDepth()", "-16374");
  check_equals(mo, "dup.prop1", "undefined");  // user defined property will not be duplicated
  check_equals(mo, "typeof(dup.onLoad)", "'undefined'"); //user defined event will not be duplicated
  check_equals(mo, "dup.getDepth()", "1");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); 
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



