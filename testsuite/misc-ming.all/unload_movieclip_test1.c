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
 * zou lunkai  zoulunkai@gmail.com
 *
 * Test "this" context in the UNLOAD event handler.
 *
 * run as ./unload_movieclip_test1
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "unload_movieclip_test1.swf"

SWFDisplayItem add_static_mc(SWFMovie mo, const char* name, int depth);

SWFDisplayItem
add_static_mc(SWFMovie mo, const char* name, int depth)
{
  SWFMovieClip mc;
  SWFDisplayItem it;
   
  mc = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, (SWFBlock)mc);
  SWFDisplayItem_setDepth(it, depth); 
  SWFDisplayItem_setName(it, name);

  return it;
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFDisplayItem it;

  const char *srcdir=".";
  if ( argc>1 ) 
    srcdir=argv[1];
  else
  {
      fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
      return 1;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);
  SWFMovie_setRate(mo, 2);

  // Frame 1: Place dejagnu clip and init testing variables

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, "_root.x = 0;");
  SWFMovie_nextFrame(mo); 
  
  // Frame 2: Place a static mc and define onUnload for it
  
  it = add_static_mc(mo, "mc", 3);
  add_actions(mo, "mc.onUnload = function () { "
                  "    _root.x = this._currentframe; "
                  "    _root.check_equals(typeof(this),  'movieclip'); "
                  "    _root.check_equals(this, _root.mc); "
                  "};");
  SWFMovie_nextFrame(mo);
  
  // Frame 3: Remove the mc to trigger onUnload
  
  SWFDisplayItem_remove(it);
  SWFMovie_nextFrame(mo);
  
  // Frame 4: checks
  
  check_equals(mo, "_root.x", "1");
  
  add_actions(mo, "_root.totals(); stop(); ");
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}

