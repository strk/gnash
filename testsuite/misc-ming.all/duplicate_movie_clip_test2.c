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
 *
 * movieClips hiberarchy:
 * _root  (5 frames)
 *   |----dejagnuclip
 *   |----mc1
 *   |     |----mc11
 *   |----mc2
 *
 * expected behaviour:
 * (1)target path is supported in duplicateMovieClip, valid path formats
 *    are: a.b.c; a/b/c; a/b/c/; a/b/:c; a/b/:c/;  /:a/:b/:c(odd);
 * (2)the source movie clip should be within the same timeline as the new movie clip.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "duplicate_movie_clip_test2.swf"


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
  mo = newSWFMovieWithVersion(OUTPUT_VERSION);
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate (mo, 12.0);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFMovie_nextFrame(mo); // 1st frame 

  /* add mc1, mc2 to the main movie */  
      
  mc1 = newSWFMovieClip();
  mc11 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc11); 
  it11 = SWFMovieClip_add(mc1, (SWFBlock)mc11);  
  SWFDisplayItem_setDepth(it11, 10); 
  SWFDisplayItem_setName(it11, "mc11"); 
  SWFMovieClip_nextFrame(mc1); 

  add_clip_actions(mc1, 
    "duplicateMovieClip('mc11', 'dup_ch1', 1);"
    "_root.check_equals(typeof(dup_ch1), 'movieclip');"
    
    "duplicateMovieClip('/_root/mc1/mc11', 'dup_ch2', 2);"
    "_root.check_equals(typeof(dup_ch2), 'movieclip');"
    
    "duplicateMovieClip('/:_root/:mc1/:mc11', 'dup_ch3', 3);"
    "_root.check_equals(typeof(dup_ch3), 'movieclip');"
    
    "duplicateMovieClip('mc2', 'dup_ch4', 4);"
    // can not duplicate a movie clip in a different time
    "_root.check_equals(typeof(dup_ch4), 'undefined');"
    
    "duplicateMovieClip('/:mc2', 'dup_ch5', 5);"
    // can not duplicate a movie clip in a different time
    "_root.check_equals(typeof(dup_ch5), 'undefined');"
    
    // Don't do this, duplicate a self-clip will crash the proprietary player"
    // "duplicateMovieClip('/:mc1', 'dup_ch6', 6);"
    "stop();"
    );
  SWFMovieClip_nextFrame(mc1); 
  
  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2); 
  
  it1 = SWFMovie_add(mo, (SWFBlock)mc1); 
  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it1, 10); 
  SWFDisplayItem_setName(it1, "mc1"); 
  SWFDisplayItem_setDepth(it2, 20); 
  SWFDisplayItem_setName(it2, "mc2"); 
  SWFMovie_nextFrame(mo); // 2nd frame


  add_actions(mo, "duplicateMovieClip('mc1', 'dup1', 1);");
  check_equals(mo, "typeof(dup1)", "'movieclip'");
  add_actions(mo, "removeMovieClip(dup1);");
  check_equals(mo, "typeof(dup1)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('/:mc1', 'dup2', 2);");
  check_equals(mo, "typeof(dup2)", "'movieclip'");
  add_actions(mo, "removeMovieClip(dup2);");
  check_equals(mo, "typeof(dup2)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('/:mc1/', 'dup3', 3);");
  check_equals(mo, "typeof(dup3)", "'movieclip'");
  add_actions(mo, "removeMovieClip(dup3);");
  check_equals(mo, "typeof(dup3)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('/_root/mc1/', 'dup4', 4);");
  check_equals(mo, "typeof(dup4)", "'movieclip'");
  add_actions(mo, "removeMovieClip(dup4);");
  check_equals(mo, "typeof(dup4)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('_root.mc1', 'dup5', 5);");
  check_equals(mo, "typeof(dup5)", "'movieclip'");
  add_actions(mo, "removeMovieClip(dup5);");
  check_equals(mo, "typeof(dup5)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('_root.mc1.mc11', 'dup6', 6);");
  // can not duplicate a movieclip in a different timeline
  check_equals(mo, "typeof(dup6)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('/_root/mc1/:mc11', 'dup7', 7);");
  // can not duplicate a movieclip in a different timeline
  check_equals(mo, "typeof(dup7)", "'undefined'");
  
  add_actions(mo, "duplicateMovieClip('/_root/mc1/mc11', 'dup8', 8);");
  // can not duplicate a movieclip in a different timeline
  check_equals(mo, "typeof(dup8)", "'undefined'");
  SWFMovie_nextFrame(mo); // 3rd frame

  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); // 4th frame
  
    //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

