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
#include <unistd.h>

#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 6
#define OUTPUT_FILENAME "streamingSoundTest2.swf"
void addMovieClipFrames(SWFMovieClip mc, int num);
void addMovieFrames(SWFMovie mc, int num);

void
addMovieClipFrames(SWFMovieClip mc, int num)
{
    int i;
    for (i = 0; i < num; ++i) {
        SWFMovieClip_nextFrame(mc);
    }
}

void
addMovieFrames(SWFMovie m, int num)
{
    int i;
    for (i = 0; i < num; ++i) {
        SWFMovie_nextFrame(m);
    }
}

int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip mc;
  SWFMovieClip dejagnuclip;
  SWFSoundStream ss;
  FILE* sound_f;
  const char* sound_filename;
  const char* srcdir;

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

  sound_filename=MEDIADIR"/click.mp3";

  sound_f = fopen(sound_filename, "r");
  if ( ! sound_f ) {
    perror(sound_filename);
    return EXIT_FAILURE;
  }

  printf("Using sound file %s\n", sound_filename);

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);
  
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 100, 100);
  SWFMovie_setRate(mo, 4);

	dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10,
            0, 80, 800, 600);
	SWFMovie_add(mo, (SWFBlock)dejagnuclip);

  ss = newSWFSoundStream(sound_f);

  mc = newSWFMovieClip();
  SWFMovieClip_setSoundStream(mc, ss, 8);
  addMovieClipFrames(mc, 24);
  SWFMovie_add(mo, mc);

#if 1
  mc = newSWFMovieClip();
  SWFMovieClip_setSoundStream(mc, ss, 16);
  addMovieClipFrames(mc, 24);
  SWFMovie_add(mo, mc);
#endif 

  // Only the first started stream governs the frame rate, but
  // it doesn't affect the first frame it's played in. 
  //
  // The MovieClips are all 4 frames long, so we expect a normal frame every
  // 4 frames, then 3 at 8 frames per second.

  SWFMovie_nextFrame(mo);

  SWFMovie_add(mo, newSWFAction(
              "var c = getTimer();"
              "approx = function(exp, act) {"
              "    if (Math.abs(exp - act) / exp < 0.25) return true;"
              "    return false;"
              "};"

              // Just check some in the middle now.
              // TODO: this should apply to most frames.
              "checkTime = function(frame, time) {"
              "     var f = frame % 24;"
              "     if (f > 5 && f < 13) {"
              "         check(approx(time, 1000 / 8));"
              "     };"
              "};"
              "onEnterFrame = function() { "
              "    var now = getTimer();"
              "    var delta = now - c;"
              "    trace('Frame ' + _currentframe + ', ' + delta);"
              "    checkTime(_currentframe, delta);"
              "    c = now;"
              "};"
              ));
  addMovieFrames(mo, 23);

#if 1
  // MovieClip 3
  mc = newSWFMovieClip();
  SWFMovieClip_setSoundStream(mc, ss, 1);
  addMovieClipFrames(mc, 24);
#endif

  addMovieFrames(mo, 24);


  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return EXIT_SUCCESS;
}
