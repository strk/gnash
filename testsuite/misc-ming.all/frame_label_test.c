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
    _root
      |------mc1
              |------mc11
*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 5
#define OUTPUT_FILENAME  "frame_label_test.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc1, mc11, dejagnuclip;
  SWFDisplayItem it1, it11;
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
  add_actions(mo, "x1=0; x2=0; x3=0; x4=0; x5=0; x6=0; x7=0; x8=0;");
  SWFMovie_nextFrame(mo); /* 1st frame of _root */

  
  mc11 = newSWFMovieClip();
  sh_red = make_fill_square (100, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc11, (SWFBlock)sh_red);  
  add_clip_actions(mc11, "stop();");
  SWFMovieClip_nextFrame(mc11); 
  SWFMovieClip_nextFrame(mc11); 
  SWFMovieClip_nextFrame(mc11); 
  
  add_clip_actions(mc11, "_root.x1 = 'mc11_frame4'; stop(); "); 
  SWFMovieClip_labelFrame(mc11, "frame4");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x2 = 'mc11_frame5'; stop(); ");
  SWFMovieClip_labelFrame(mc11, "frame5");
  SWFMovieClip_nextFrame(mc11); 
  add_clip_actions(mc11, "_root.x3 = 'mc11_frame6'; stop(); ");
  SWFMovieClip_labelFrame(mc11, "frame6");
  SWFMovieClip_nextFrame(mc11); 
  
    
  mc1 = newSWFMovieClip();
  sh_red = make_fill_square (200, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  SWFMovieClip_add(mc1, (SWFBlock)sh_red);  
  add_clip_actions(mc1, "stop();");
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);  
  SWFDisplayItem_setDepth(it11, 10); 
  SWFDisplayItem_setName(it11, "mc11"); 
  SWFMovieClip_nextFrame(mc1); 
  SWFMovieClip_nextFrame(mc1); 
  SWFMovieClip_nextFrame(mc1); 
  
  add_clip_actions(mc1, "_root.x4 = 'mc1_frame4'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame4");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x5 = 'mc1_frame5'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame5");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x6 = 'mc1_frame6'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame6");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x7 = 'mc1_frame7'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame7");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x8 = 'mc1_frame8'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame8");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x9 = 'small_first'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "small_first");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x9 = 'Small_first'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "Small_first");
  SWFMovieClip_nextFrame(mc1);   
  add_clip_actions(mc1, "_root.x10 = 'mc1_frame10'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "frame10");
  SWFMovieClip_nextFrame(mc1);  
  add_clip_actions(mc1, "_root.x11 = 'Big_first'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "Big_first");
  SWFMovieClip_nextFrame(mc1); 
  add_clip_actions(mc1, "_root.x11 = 'big_first'; stop(); ");
  SWFMovieClip_labelFrame(mc1, "big_first");
  SWFMovieClip_nextFrame(mc1); 

    
  /* place _root.mc1 */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  SWFDisplayItem_setDepth(it1, 20); 
  SWFDisplayItem_setName(it1, "mc1"); 
  SWFMovie_nextFrame(mo); /* 2nd frame of _root */


  add_actions(mo, " gotoAndPlay('/mc1:frame4'); "   //GotoLabel
                  " gotoAndPlay('mc1:frame5'); "    //GotoLabel
                  " gotoAndPlay('/mc1/:6'); "       //GotoLabel
                  " lable = '/mc1/mc11/:frame4'; "
                  " gotoAndPlay(lable); "           //GotoExpression
                  " lable = '/mc1/mc11/:5'; "
                  " gotoAndPlay(lable); "           //GotoExpression
                  " "CALLFRAME"('/mc1/mc11/:frame6'); "
                  " "CALLFRAME"('mc1:7'); "
                  " "CALLFRAME"('mc1/:frame8'); "
                  " "CALLFRAME"('mc1/:Small_first'); "
                  " "CALLFRAME"('mc1/:Frame10'); "
                  " "CALLFRAME"('mc1/:big_first'); ");      
                  
  SWFMovie_nextFrame(mo); /* 3rd frame of _root */
   

  /* checks */
  /* GotoExpression and callFrame support target_path */
  check_equals(mo, "_root.x1", "'mc11_frame4'");
  check_equals(mo, "_root.x2", "'mc11_frame5'");
  check_equals(mo, "_root.x3", "'mc11_frame6'");
  check_equals(mo, "_root.x7", "'mc1_frame7'");
  check_equals(mo, "_root.x8", "'mc1_frame8'");
  check_equals(mo, "_root.x9", "'small_first'");
  check_equals(mo, "_root.x10", "'mc1_frame10'");
  check_equals(mo, "_root.x11", "'Big_first'");
  /* seems that GotoLabel does not support target_path */
  check_equals(mo, "_root.x4", "0");
  check_equals(mo, "_root.x5", "0");
  check_equals(mo, "_root.x6", "0");
  add_actions(mo, " "CALLFRAME"('/:1'); ");
  check_equals(mo, "_root.x1", "0");
  check_equals(mo, "_root.x2", "0");
  check_equals(mo, "_root.x3", "0");
  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 4th frame of _root */
  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}



