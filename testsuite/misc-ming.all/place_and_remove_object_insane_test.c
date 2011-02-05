/* 
 *   Copyright (C) 2005, 2006, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 * Timeline:
 * 
 *   Frame  | 1  | 2  | 3  | 
 *  --------+----+----+----+
 *   Event  |PPP |RRR |PPPP|  
 * 
 *  P = place (by PlaceObject2)
 *  R = remove (by RemoveObject)
 * 
 * Description:
 * 
 *  frame1: place mc_red at depth3, mc_blue at depth30, mc_black at depth40
 *  frame2: remove mc_red, mc_blue, mc_black
 *  frame3: place mc_red at depth3 again with a different ratio; place mc_blue 
 *          at depth30 again with the same ratio; place mc_black at depth40 again
 *          but with a different name; place mc_green at depth4.
 *
 * Observed:
 *    
 *    loop back obeys the same rule as jump back.
 *
 *  run as ./place_and_remove_object_insane_test
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "place_and_remove_object_insane_test.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc_red, mc_green, mc_blue, mc_black, dejagnuclip;
  SWFShape  sh_red, sh_green, sh_blue, sh_black;

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

  mc_red = newSWFMovieClip();
  sh_red = make_fill_square (0, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc_red, (SWFBlock)sh_red);  
  SWFMovieClip_nextFrame(mc_red);
  
  mc_green = newSWFMovieClip();
  sh_green = make_fill_square (200, 300, 60, 60, 255, 0, 0, 0, 255, 0);
  SWFMovieClip_add(mc_green, (SWFBlock)sh_green);  
  SWFMovieClip_nextFrame(mc_green);
  
  mc_blue = newSWFMovieClip();
  sh_blue = make_fill_square (400, 300, 60, 60, 255, 0, 0, 0, 0, 255);
  SWFMovieClip_add(mc_blue, (SWFBlock)sh_blue);  
  SWFMovieClip_nextFrame(mc_blue);
  
  mc_black = newSWFMovieClip();
  sh_black = make_fill_square (600, 300, 60, 60, 255, 0, 0, 0, 0, 0);
  SWFMovieClip_add(mc_black, (SWFBlock)sh_black);  
  SWFMovieClip_nextFrame(mc_black);
  
  SWFDisplayItem it_red;
  SWFDisplayItem it_blue;
  SWFDisplayItem it_green;
  SWFDisplayItem it_black;
  
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  //add mc_red to the 1st frame at depth 3
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  
  it_blue = SWFMovie_add(mo, (SWFBlock)mc_blue); //add mc_blue to the 1st frame at depth 30
  SWFDisplayItem_setDepth(it_blue, 30); 
  SWFDisplayItem_setName(it_blue, "mc_blue");
  
  it_black = SWFMovie_add(mo, (SWFBlock)mc_black); //add mc_black to the 1st frame at depth 40
  SWFDisplayItem_setDepth(it_black, 40); 
  SWFDisplayItem_setName(it_black, "mc_black");
  
  check_equals(mo, "typeof(_root.mc_red)", "'movieclip'");
  check_equals(mo, "typeof(_root.mc_blue)", "'movieclip'");

  check_equals(mo, "_root.mc_green",  "undefined");

  add_actions(mo, " _root.mc_red._x += 10; \
                    if(counter == undefined) \
                    { \
                        check_equals(_root.mc_blue._x, 0); \
                    }else if(counter == 1)  \
                    { \
                        check_equals(_root.mc_blue._x, 60); \
                        check_equals(typeof(_root.mc_black), 'undefined'); \
                        check_equals(typeof(_root.mc_black_name_changed), 'movieclip'); \
                    } ");
  // This one is normal. mc_red._x should *not* be 20 when restart.
  // Note that mc_red has been removed at the 2nd frame, so when
  //   restart, it will be reconstructed.              
  check_equals(mo, " _root.mc_red._x",  "10"); 
  SWFMovie_nextFrame(mo);        
  //------------end of 1st frame---------------------------------
  
  
  SWFMovie_remove(mo, it_red);    //remove mc_red at the 2nd frame
  SWFMovie_remove(mo, it_blue);   //remove mc_blue at the 2nd frame
  SWFMovie_remove(mo, it_black);  //remove mc_black at the 2nd frame
  check_equals(mo, "typeof(_root.mc_red)",  "'undefined'");
  check_equals(mo, "typeof(_root.mc_blue)",  "'undefined'");
  check_equals(mo, "typeof(_root.mc_black)",  "'undefined'");
  check_equals(mo, "typeof(_root.mc_green)",  "'undefined'");
  SWFMovie_nextFrame(mo);       
  //------------end of 2nd frame---------------------------------
  
  
  it_red = SWFMovie_add(mo, (SWFBlock)mc_red);  //add mc_red to the 3rd frame at depth 3 again
  SWFDisplayItem_setDepth(it_red, 3); 
  SWFDisplayItem_setName(it_red, "mc_red");
  SWFDisplayItem_setRatio(it_red, 2.0); 
  
  it_blue = SWFMovie_add(mo, (SWFBlock)mc_blue); //add mc_blue to the 3rd frame at depth 30 again
  SWFDisplayItem_setDepth(it_blue, 30); 
  SWFDisplayItem_setName(it_blue, "mc_blue");

  it_black = SWFMovie_add(mo, (SWFBlock)mc_black);  //add mc_black to the 3rd frame at depth 40 again
  SWFDisplayItem_setDepth(it_black, 40); 
  SWFDisplayItem_setName(it_black, "mc_black_name_changed");
    
  it_green = SWFMovie_add(mo, (SWFBlock)mc_green);  //add mc_green to the 3rd frame at depth 4
  SWFDisplayItem_setDepth(it_green, 4); 
  SWFDisplayItem_setName(it_green, "mc_green");
    
  check_equals(mo, "typeof(_root.mc_red)", "'movieclip'");
  check_equals(mo, "typeof(_root.mc_blue)", "'movieclip'");
  
  add_actions(mo, " _root.mc_red._x += 60; _root.mc_blue._x += 60;");
  add_actions(mo, "if ( ++counter > 1 ) { _root.totals(); stop(); }");

  SWFMovie_nextFrame(mo);        
  //------------end of 3rd frame---------------------------------
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
