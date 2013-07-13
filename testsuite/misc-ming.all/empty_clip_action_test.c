/*
 *   Copyright (C) 2005-2013 Free Software Foundation, Inc.
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


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>
#include <assert.h>

#include "ming_utils.h"

#define OUTPUT_VERSION  6
#define OUTPUT_FILENAME  "empty_clip_action_test.swf"

// So this is about as hacky as you get.  The idea is that we change the
// CHANGEOBJECT2 tag so that the first action (containing just a single
// null-byte) is truncated to nothing: event_length is updated to zero, the
// following two bytes are skipped and then of course the tag length itself has
// to be reduced by one.

// So far as I'm aware you can't do this programmatically in Ming, but of
// course if the output stream ever changes this will be completely broken. 
void spesiulOutputMethod(byte b, void *data);
void spesiulOutputMethod(byte b, void *data)
{
    static long count = 0;
    byte c = b;
    FILE *f = (FILE *)data;

    if (count == 0x293b) {
        assert( b == 122);
        c = 120;
    }
    if (count == 0x2953) {
        assert( b == 2);
        c = 0;
    }
    if (count == 0x2954 || count == 0x2955) {
         assert( b == 0);
         ++count;
         return;
    }

    fputc(c, f);
    ++count;
}



int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip  mc2, dejagnuclip;
  SWFDisplayItem it2;
  FILE* fh;

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
  add_actions(mo, "x2=0;");
  SWFMovie_nextFrame(mo);  /* 1st frame */

  
  mc2 = newSWFMovieClip();

  it2 = SWFMovie_add(mo, (SWFBlock)mc2); 
  SWFDisplayItem_setDepth(it2, 20); 
  SWFDisplayItem_setName(it2, "mc2"); 
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(""),
    SWFACTION_KEYPRESS);  
  SWFDisplayItem_addAction(it2,
    compileSWFActionCode(" _root.note('onClipEnterFrame triggered'); "
                         " _root.x2 = 14; "),
    SWFACTION_ENTERFRAME);  

  SWFMovie_nextFrame(mo); /* 2nd frame */
  
  check_equals(mo, "_root.x2", "14"); 
  SWFMovie_nextFrame(mo); /* 3rd frame */
  
  SWFDisplayItem_remove(it2);

  SWFMovie_nextFrame(mo); /* 4th frame */

  add_actions(mo, " _root.totals(); stop(); ");
  SWFMovie_nextFrame(mo); /* 5th frame */

  // Output movie
  puts("Saving " OUTPUT_FILENAME );

  // ..but wait! Now entering crazy-land.
  fh = fopen(OUTPUT_FILENAME, "w");
  SWFMovie_output(mo, spesiulOutputMethod, fh);
  fclose(fh);
  return 0;
}

