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
 * mc1 and mc2 are two movieClips created by PlaceObject2 tag;
 * dup1 and dup2 are two movieClips created by AS;
 * swap mc1 to the dynamic range depths;
 * swap dup1 to the static range depths;
 *
 * expected behaviour after loopback:
 *  mc1 keep alive;
 *  mc2 get removed;
 *  dup1 get removed;
 *  dup2 keep alive;
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "static_vs_dynamic1.swf"




int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc2, dejagnuclip;
  SWFDisplayItem it1, it2;
  SWFShape  sh1,sh2;

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
  SWFMovie_nextFrame(mo); 
  
  add_actions(mo, " if(loopback == undefined) {"  // first time play
                  " check_equals(typeof(mc1), 'undefined'); "
                  " check_equals(typeof(mc2), 'undefined'); "
                  " check_equals(typeof(dup1), 'undefined'); "
                  " check_equals(typeof(dup2), 'undefined'); "
                  "} else {"                      // loopback
                  " check_equals(typeof(mc1), 'movieclip');"
                  " check_equals(typeof(mc2), 'undefined');"
                  " check_equals(typeof(dup1), 'undefined');"
                  " check_equals(typeof(dup2), 'movieclip');"
                  "}" );
  SWFMovie_nextFrame(mo); 
  
  mc1 = newSWFMovieClip();
  mc2 = newSWFMovieClip();
  sh1 = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  sh2 = make_fill_square (100, 300, 60, 60, 255, 0, 0, 0, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh1);  
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc1);
  SWFMovieClip_nextFrame(mc2);
    
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setDepth(it1, 2); 
  SWFDisplayItem_setName(it1, "mc1");
  
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_setDepth(it2, 3); 
  SWFDisplayItem_setName(it2, "mc2");
  
  add_actions(mo, " if(loopback == undefined) {"
                  "   duplicateMovieClip('mc1', 'dup1', 1);"
                  "   duplicateMovieClip('mc2', 'dup2', 2);"
                  "   check_equals(mc1.getDepth(), -16382);"
                  "   check_equals(mc2.getDepth(), -16381);"
                  "   check_equals(dup1.getDepth(), 1);"
                  "   check_equals(dup2.getDepth(), 2);"
                  "   mc1.swapDepths(10);"
                  "   check_equals(mc1.getDepth(), 10);"
                  "   dup1.swapDepths(-10);"
                  "   check_equals(dup1.getDepth(), -10);"
                  "}");
  SWFMovie_nextFrame(mo);        


  add_actions(mo, "if(loopback == undefined) {"
                  "  loopback = 1;"   
                  "  gotoAndPlay(1);" //loopback
                  "} else {"
                  "   _root.totals(); stop();" 
                  "}");
  SWFMovie_nextFrame(mo);        
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
