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
 * Plays an external FLV video
 * Should be used with the MovieTester to test if the video decoder works.
 *
 * Expected behaviour:
 *
 *   Shows a 54x54 pixels red square moving from left to right over
 *   a 152x120 yellow background. The whole thing rotated by 45 degrees clockwise.
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

  // Some online examples...
  //
  // 'ffdec_vp6f' 
  //sprintf(filename, "http://www.helpexamples.com/flash/video/water.flv");
  //sprintf(filename, "http://www.helpexamples.com/flash/video/clouds.flv");
  //sprintf(filename, "http://www.helpexamples.com/flash/video/typing_long.flv");
  //
  // This ones work
  //sprintf(filename, "http://www.helpexamples.com/flash/video/caption_video.flv");
  //sprintf(filename, "http://www.helpexamples.com/flash/video/sheep.flv");
  //
  //
  
  sprintf(buffer, 
  	"nc=new NetConnection();"
	"nc.connect(null);"
	"stream = new NetStream(nc);"
	"video.attachVideo(stream); "
	"stream.setBufferTime(2); "
	"stream.play('%s');"
	"stop();",
	filename );

  Ming_init();
  Ming_useSWFVersion (OUTPUT_VERSION);


  mo = newSWFMovie();
  SWFMovie_setDimension(mo, 800, 600);

  // We also want to test that 1FPS of SWF rate doesn't influence
  // rate of video playback, but for now it's more useful to actually
  // have something rendered, so check_pixel can eventually do something,
  // so we run fast...
  SWFMovie_setRate(mo, 32);

  dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(mediadir), 10, 0, 0, 800, 600);
  item = SWFMovie_add(mo, (SWFBlock)dejagnuclip);
  SWFDisplayItem_moveTo(item, 0, 250);
  SWFMovie_nextFrame(mo); 
 
  stream = newSWFVideoStream();
  SWFVideoStream_setDimension(stream, video_width, video_height);
  item = SWFMovie_add(mo, (SWFBlock)stream);
  /* SWFDisplayItem_moveTo(item, 0, 200); */
  SWFDisplayItem_setName(item, "video");

  a = newSWFAction(buffer);
  if(a == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)a);

  check_equals(mo, "video._xscale", "100");
  check_equals(mo, "video._yscale", "100");
  check_equals(mo, "video._rotation", "0");
  check_equals(mo, "video._target", "'/video'");

  add_actions(mo,
		"video._x = 100;"
		"video._xscale = 120;"
		"video._yscale = 120;"
		"video._rotation = 45;");

  check_equals(mo, "video._x", "100")	;
  check_equals(mo, "Math.round(video._xscale*100)/100", "120");
  check_equals(mo, "Math.round(video._yscale*100)/100", "120");
  check_equals(mo, "Math.round(video._rotation*100)/100", "45");


  // How can I test props here ?
  check_equals(mo, "typeof(video.hitTest)", "'undefined'");
  check_equals(mo, "typeof(video.getBounds)", "'undefined'");

  SWFMovie_add(mo, (SWFBlock)newSWFAction(

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
		" 	_root.check_equals(this.bytesTotal, 21482); "
		"	this.stopNotified = true;"
		"	_root.nextFrame();"
		" }"
#if 0
		" else if ( info.code == 'NetStream.Buffer.Empty' && this.stopNotified ) "
	        " {"
		"	check ( this.stopNotified )"
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

  SWFMovie_add(mo, (SWFBlock)newSWFAction("totals(); stop();"));

  SWFMovie_nextFrame(mo);


  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
