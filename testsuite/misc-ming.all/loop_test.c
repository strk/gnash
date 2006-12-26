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

/*
 * Zou Lunkai, zoulunkai@gmail.com
 *
 * Test for movie loop.
 * 
 * The _root movie has 30 frames. 
 *
 * Normally, you can see both the red and black squares overlap each
 *  other with equal time.
 *
 * run as ./loop_test
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "loop_test.swf"

//actions in _root movie, the last frame
SWFAction  action_in_root(void);
SWFAction  action_in_root()
{
  SWFAction ac;
  ac = compileSWFActionCode(" \
      var mc1_depth = movieClip1.getDepth(); \
      var mc2_depth = movieClip2.getDepth(); \
      movieClip1.swapDepths(movieClip2); \
  ");
  // we're not loading dejagnu clip !
  //_root.check_equals(movieClip1.getDepth(), mc2_depth); 
  //_root.check_equals(movieClip2.getDepth(), mc1_depth);
  return ac;
}


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc1, mc2, dejagnuclip;
  SWFShape  sh1,sh2;
  SWFAction ac;
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
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);

  //dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  //SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  //SWFMovie_nextFrame(mo); 


  sh1 = make_fill_square (300, 300, 60, 60, 255, 0, 0, 255, 0, 0);
  mc1 = newSWFMovieClip();
  SWFMovieClip_add(mc1, (SWFBlock)sh1); 
  SWFMovieClip_nextFrame(mc1);
  
  sh2 = make_fill_square (330, 270, 120, 120, 0, 0, 0, 0, 0, 0);
  mc2 = newSWFMovieClip();
  SWFMovieClip_add(mc2, (SWFBlock)sh2);  
  SWFMovieClip_nextFrame(mc2); 

  SWFDisplayItem it1, it2;
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  //add movieClip1 to the _root
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  //add movieClip2 to the _root
  SWFDisplayItem_setName(it1, "movieClip1"); //name movieClip1
  SWFDisplayItem_setName(it2, "movieClip2"); //name movieClip2
  
  for(i=0; i<29; i++)
  {
    SWFMovie_nextFrame(mo); 
  }

  ac =  action_in_root();
  SWFMovie_add(mo, (SWFBlock)ac);
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
