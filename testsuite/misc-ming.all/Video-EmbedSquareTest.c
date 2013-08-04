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
 * Plays video stored internal in the SWF.
 * Should be used with the MovieTester to test if the video decoder works.
 *
 * TODO:
 *
 * Test all codecs:
 *
 *   - VIDEO_CODEC_H263     (id: 2) [ in square.flv ]
 *   - VIDEO_CODEC_SCREEN   (id: 3)
 *   - VIDEO_CODEC_VP6      (id: 4)
 *   - VIDEO_CODEC_VP6A     (id: 5)
 *   - VIDEO_CODEC_SCREEN2  (id: 6)
 * 
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "Video-EmbedSquareTest.swf"

const char* mediadir=".";

int
main(int argc, char** argv)
{
  SWFMovie mo;
  int frames;
  SWFVideoStream stream;
  SWFDisplayItem item;
  SWFMovieClip dejagnuclip;
  FILE *flv, *font_file;
  SWFFont font;
  char filename[256];
  char fdbfont[1024];

  if ( argc>1 ) mediadir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }
	
  
  sprintf(filename, "%s/square.flv", mediadir);
  flv = fopen(filename, "rb");
  if (flv == NULL) {
	  perror(filename);
	  return -1;
  }

  sprintf(fdbfont, "%s/Bitstream-Vera-Sans.fdb", mediadir);
  font_file = fopen(fdbfont, "r");
  if ( font_file == NULL )
  {
    perror(fdbfont);
    exit(1);
  }

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);

  font = loadSWFFontFromFile(font_file);
	
  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 320, 96);

  if (mo == NULL) {
    fclose(flv);
    return -1;
  }

  SWFMovie_setRate(mo, 5);

  dejagnuclip = get_dejagnu_clip((SWFBlock)font, 10, 0, 0, 200, 200);
  item = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFDisplayItem_setDepth(item, 200); 
  SWFDisplayItem_move(item, 140, 0); 

  stream = newSWFVideoStream_fromFile(flv);
  item = SWFMovie_add(mo, (SWFBlock)stream);
  SWFDisplayItem_setName(item, "vid");

  // A bug in Ming prevents this from working the
  // way I liked it to. It was useful to try this
  // out as it exposed a bad bug in gnash eating up
  // all memory and a few other issues.
  //
  // In order to keep the test *runner* untouched
  // this will be commented out for now. When Ming
  // is fixed I'll like to have two instances of
  // the definition playing one near the other,
  // both playing all frames.
  //
  //item = SWFMovie_add(mo, (SWFBlock)stream);
  //SWFDisplayItem_move(item, 150, 0);

  // Mouse clicks toggle play/stop
  add_actions(mo,
	"_root.onMouseDown = function() {"
	"  if (stopped) { play(); stopped=false; }"
	"  else { stop(); stopped=true; }"
	"};");

  // Pressing a number jumps to the specified frame
  add_actions(mo,
	"_root.onKeyDown = function() {"
	" var fnum = Key.getAscii() - 47;"
	//" trace('going to frame '+fnum);"
	" _root.gotoAndPlay(fnum);"
	"}; Key.addListener(_root);");

  // TODO: dynamic frame rate adjust
  frames = SWFVideoStream_getNumFrames(stream);
  for(; frames > 0; frames--)
  {
    SWFMovie_nextFrame(mo);
  }

  /// Video.clear() should not work on embedded Videos.
  add_actions(mo, "stop(); vid.clear(); trace(vid);");
  add_actions(mo, "note('You should still see the video frame."
          " Click to replay');"
		  "o = {};"
		  "o.onMouseDown = function() { _root.play(); };"
		  "Mouse.addListener(o);"
	     );

  SWFMovie_nextFrame(mo);

  //Output movie
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
