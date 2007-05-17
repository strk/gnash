/* 
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 * Plays an external FLV video
 * Should be used with the MovieTester to test if the video decoder works.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "NetStream-SquareTest.swf"

const char* mediadir=".";
char filename[256];


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFVideoStream stream;
  SWFDisplayItem item;
  SWFAction a;
  char buffer[1024];

  int video_width = 128;
  int video_height = 96;

  if ( argc>1 ) mediadir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }
	
  sprintf(filename, "%s/square.flv", mediadir);
  
  sprintf(buffer, 
  	"nc=new NetConnection();"
	"nc.connect(null);"
	"stream = new NetStream(nc);"
	"video.attachVideo(stream); "
	"stream.setBufferTime(2); "
	"stream.play(\"%s\");"
	"stop();",
	filename);

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);


  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);

  SWFMovie_setRate(mo, 1);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(mediadir), 10, 0, 0, 800, 600);
  item = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFDisplayItem_moveTo(item, 0, 100);
  SWFMovie_nextFrame(mo); 
 
  stream = newSWFVideoStream();
  SWFVideoStream_setDimension(stream, video_width, video_height);
  item = SWFMovie_add(mo, (SWFBlock)stream);
  /* SWFDisplayItem_moveTo(item, 0, 200); */
  SWFDisplayItem_setName(item, "video");

  a = newSWFAction(buffer);
  if(a == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)a);
  SWFMovie_add(mo, newSWFAction(

		"stream.onStatus = function(info) {"

		// Ignore Buffer.Flush for now
		"  if ( info.code == 'NetStream.Buffer.Flush' ) return; "

		// Print some info
		" _root.note('onStatus('+info.code+') called'); "
		" _root.note(' bufferLength:'+stream.bufferLength+"
		" 	' bytesLoaded:'+stream.bytesLoaded+"
		"	' currentFps:'+stream.currentFps+' time:'+stream.time);"

		" if ( info.code == 'NetStream.Play.Stop' )"
		" {"
		" 	_root.check(this instanceOf NetStream); "
		" 	_root.check_equals(this.bufferTime, 2); "
		" 	_root.check_equals(this.bytesTotal, 46893); "
		"	this.stopNotified = true;"
		"	_root.nextFrame();"
		" }"
#if 0
		" else if ( info.code == 'NetStream.Buffer.Empty' && this.stopNotified ) "
	        " {"
		"	check ( this.stopNotified )
		"	this.close();"
		"	_root.nextFrame();"
		" }"
#endif

		"};"

		"stream.onCuePoint = function(info) {"
		" _root.note('onCuePoint('+info+') called'); "
		"};"
		"stream.onMetaData = function(info) {"
		" _root.note('onMetaData('+info+') called'); "
		"};"
		));

  SWFMovie_nextFrame(mo);

  SWFMovie_add(mo, newSWFAction("totals(); stop();"));

  SWFMovie_nextFrame(mo);


  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
