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
 * Test for sounds being started and stopped
 * 
 * The _root movie has 20 frames. 
 *
 * In uneven frames a sound is started, and in even frames it is stopped.
 */


#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "eventSoundTest1.swf"


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFSound  so;
  FILE *sound_f;
  int i;
  const char* sound_filename;

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
  so =  newSWFSound(sound_f, SWF_SOUND_MP3_COMPRESSED | SWF_SOUND_44KHZ | SWF_SOUND_16BITS |SWF_SOUND_STEREO);
  for(i=0; i<20; i++)
  {
    SWFSoundInstance so_in = SWFMovie_startSound(mo, so);
    SWFMovie_nextFrame(mo);
    ++i;
    SWFMovie_stopSound(mo, so);
    SWFMovie_nextFrame(mo);
  }

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
