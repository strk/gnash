/* 
 *   Copyright (C) 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
 *  Test that instance name are syntezed only if a name isn't given
 *  at all (empty name is still a name).
 *  TODO: we may test other name syntesis here, in particular for other
 *  kind of DisplayObjects..
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "instanceNameTest.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFDisplayItem it1, it2;
  SWFMovieClip mc1, mc2, dejagnuclip;
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
  SWFMovie_setRate(mo, 12);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
  SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  add_actions(mo, " haslooped1=false; haslooped2=false; haslooped3=false;"
                  " mc1Initialized=0; mc1Unloaded=0;"
                  " mc2Initialized=0; mc2Unloaded=0;"
                  " mc3Initialized=0; mc3Unloaded=0;"
                  " asOrder='0+';");
  SWFMovie_nextFrame(mo); // frame1


  mc1 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc1);

  mc2 = newSWFMovieClip();
  SWFMovieClip_nextFrame(mc2);
 
  /* An empty name ... */
  it1 = SWFMovie_add(mo, (SWFBlock)mc1);  
  SWFDisplayItem_setName(it1, ""); 
  SWFDisplayItem_addAction(it1, newSWFAction(
        "_root.check_equals(this._target, '/');"
        ), SWFACTION_INIT); 

  /* ... is different then no name at all. */
  it2 = SWFMovie_add(mo, (SWFBlock)mc2);  
  SWFDisplayItem_addAction(it2, newSWFAction(
        "_root.check_equals(this._target, '/instance2');"
        ), SWFACTION_INIT); 

  SWFMovie_nextFrame(mo); // frame2
  
  add_actions(mo, "totals(2); stop();");
  SWFMovie_nextFrame(mo);  // frame 15
  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
