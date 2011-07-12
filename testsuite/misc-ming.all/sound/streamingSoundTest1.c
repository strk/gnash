/* 
 *   Copyright (C) 2007, 2009, 2010, 2011 Free Software Foundation, Inc.
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

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
  struct stat statbuf;

  sound_filename=MEDIADIR"/click.mp3";

  sound_f = fopen(sound_filename, "r");
  if ( ! sound_f ) {
    perror(sound_filename);
    return EXIT_FAILURE;
  }

  printf("Using sound file %s\n", sound_filename);

  if ( -1 == fstat(fileno(sound_f), &statbuf) )
  {
    perror("fstat");
    return EXIT_FAILURE;
  }
  if ( S_ISDIR(statbuf.st_mode) )
  {
    fprintf(stderr, "%s is a directory, we need a file\n", sound_filename);
    return EXIT_FAILURE;
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 100, 100);
  SWFMovie_setRate(mo, 1);

  ss = newSWFSoundStream(sound_f);

  mc = newSWFMovieClip();
  SWFMovieClip_setSoundStream(mc, ss, 2);
  SWFMovieClip_nextFrame(mc);
  SWFMovieClip_nextFrame(mc);
  SWFMovieClip_nextFrame(mc);
  SWFMovieClip_nextFrame(mc);

  SWFMovie_add(mo, mc);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);
  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return EXIT_SUCCESS;
}
