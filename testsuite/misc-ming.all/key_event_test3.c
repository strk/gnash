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

/*
 *  frame1: set _root.x1 to zero;
 *  frame2: 
 *    create a dynamic movieclip at the static depth zone with a user defined onKeyDown event handler;
 *    increase _root.x1 within the event handler;
 *
 *  KeyDown events are provided by the testrunner.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "key_event_test3.swf"



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  dejagnuclip;
  SWFDisplayItem  it;

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
  SWFMovie_setRate (mo, 0.5);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, "_root.note('at frame1'); x1=0;");
  SWFMovie_nextFrame(mo);  // 1st frame 
  
  add_actions(mo, 
    "_root.createEmptyMovieClip('dynamic_mc', -10);"
    "dynamic_mc.onKeyDown = function() "
    "{"
    "  _root.note('KeyDown triggered');"
    "  _root.x1++;"
    "  _root.note('_root.x1 is: ' + _root.x1);"
    "};"
    "Key.addListener(dynamic_mc);"
  );
  SWFMovie_nextFrame(mo);  // 2nd frame  
  
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
