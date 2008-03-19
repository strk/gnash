/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 * Test for streaming sound
 * 
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "streamingSoundTest1.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc;
  SWFSoundStream ss;
  FILE* sound_f;
  const char* sound_filename;
  SWFDisplayItem it;

  if ( argc>1 ) {
    sound_filename=argv[1];
  } else {
    sound_filename="sound1.mp3";
  }

  sound_f = fopen(sound_filename, "r");
  if ( ! sound_f ) {
    perror(sound_filename);
    return 1;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 100, 100);
  SWFMovie_setRate(mo, 1);

  ss = newSWFSoundStream(sound_f);

  mc = newSWFMovieClip();
  SWFMovieClip_setSoundStream(mc, ss, 1);
  SWFMovieClip_nextFrame(mc);

  it = SWFMovie_add(mo, mc);
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
