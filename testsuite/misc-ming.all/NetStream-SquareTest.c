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
  SWFAction b;
  char buffer_a[1028];
  char buffer_b[1028];

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
  
  sprintf(buffer_a,
  	"note(System.capabilities.version);"
  	"note('SWF version %d');"
  	
  	"nc=new NetConnection();"
	"nc.connect(null);"
	"check(!NetStream.prototype.hasOwnProperty('currentFPS'));" // version 7 here
	"xcheck(!NetStream.prototype.hasOwnProperty('currentFps'));"
	"stream = new NetStream();"
	"check_equals ( typeof(stream.bytesTotal), 'undefined' );" // not connected..
	"stream.play('fake');" // just test not to segfault..
	"stream = new NetStream(nc);"
	"check_equals ( typeof(stream.bytesTotal), 'number' );"
	"stream.bytesTotal = 'string';"
	"check_equals ( typeof(stream.bytesTotal), 'number' );",	
	OUTPUT_VERSION);

  	sprintf(buffer_b,
  	// bytesTotal (read-only)
	"stream.play('%s');"
	"stop();",
	filename);

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

  a = newSWFAction(buffer_a);
  if(a == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)a);

  /* Check that properties exist here
     See actionscript.all/NetStream.as for checks before
     NetStream is attached to a connection */

  check(mo, "NetStream.prototype.hasOwnProperty('currentFps')"); // currentFps
  check(mo, "NetStream.prototype.hasOwnProperty('bufferLength')"); // check bufferLength
  check(mo, "NetStream.prototype.hasOwnProperty('bufferTime')"); // check bufferTime
  check(mo, "NetStream.prototype.hasOwnProperty('liveDelay')"); // check liveDelay
  check(mo, "NetStream.prototype.hasOwnProperty('time')"); // check time
  check(mo, "NetStream.prototype.hasOwnProperty('bytesLoaded')"); // check bytesLoaded
  check(mo, "NetStream.prototype.hasOwnProperty('bytesTotal')"); // check bytesTotal
  
  add_actions(mo, "video.attachVideo(stream);"); 
  
  // currentFps (read-only)
  xcheck_equals (mo, "typeof(stream.currentFps)", "'number'" );
  add_actions(mo, "stream.currentFps = 'string';");
  xcheck_equals (mo, "typeof(stream.currentFps)", "'number'" );
  add_actions(mo, "stream.currentFps = false;");
  xcheck_equals (mo, "typeof(stream.currentFps)", "'number'" );

  // bufferLength (read-only)
  check_equals (mo, "typeof(stream.bufferLength)", "'number'" );
  add_actions(mo, "stream.bufferLength = 'string';");
  check_equals (mo, "typeof(stream.bufferLength)", "'number'" );
  add_actions(mo, "stream.bufferLength = false;");
  check_equals (mo, "typeof(stream.bufferLength)", "'number'" );

  // bufferTime
  check_equals (mo, "typeof(stream.bufferTime)", "'number'" );
  add_actions(mo, "stream.setBufferTime(2);");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.bufferTime = 20;");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.setBufferTime = 30;");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.setBufferTime(false);");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.setBufferTime('string');");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.setBufferTime('5');");
  check_equals (mo, "stream.bufferTime", "2");
  add_actions(mo,"stream.setBufferTime(10);");  // can't change it once set ...
  check_equals (mo, "stream.bufferTime", "2");

  // liveDelay (read-only)
  xcheck_equals (mo, "typeof(stream.liveDelay)", "'number'");
  add_actions(mo, "stream.liveDelay = 'string';");
  xcheck_equals (mo, "typeof(stream.liveDelay)", "'number'");

  // time (read-only)
  check_equals (mo, "typeof(stream.time)", "'number'" );
  add_actions(mo, "stream.time = 'string';");
  check_equals (mo, "typeof(stream.time)", "'number'" );

  // bytesLoaded (read-only)
  check_equals (mo, "typeof(stream.bytesLoaded)", "'number'" );
  add_actions(mo, "stream.bytesLoaded = 'string';");
  check_equals (mo, "typeof(stream.bytesLoaded)", "'number'" ); 

  xcheck_equals (mo, "stream.currentFps", "0" );

  /* Play video */
  b = newSWFAction(buffer_b);
  if(b == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)b);
 
  xcheck_equals (mo, "stream.currentFps", "0" );
 
  /* Publisher Methods */

  // These are documented as player methods for connections
  // to a media server.
  // Possibly they are only defined if the connection is to
  // such a server; at this point, they should be undefined in
  // any case.

  check_equals (mo, "typeof(stream.attachAudio())", "'undefined'");
  check_equals (mo, "typeof(stream.attachVideo())", "'undefined'");
  check_equals (mo, "typeof(stream.publish())", "'undefined'");
  check_equals (mo, "typeof(stream.send())", "'undefined'");
  check_equals (mo, "typeof(stream.receiveAudio())", "'undefined'");
  check_equals (mo, "typeof(stream.receiveVideo())", "'undefined'");

  /* Video checks */ 
  
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

		// debugging
		" var s='';"
		" for (e in info) { "
		"  s += e+':'+info[e]+' ';"
		" }"
		" _root.note('onMetaData: '+s);"

		" check_equals(arguments.length, 1, 'single argument');"
		" check(info instanceof Object);"

		// Test enumeration
		" var enu = new Array;"
		" for (e in info) { "
		"  enu.push(e);"
		" }"
		" xcheck_equals(enu.length, 11);" // gnash contains 2 more 

		// Test composision

		" check(info.hasOwnProperty('filesize'), 'metadata has filesize');"
		" check_equals(typeof(info.filesize), 'number', 'filesize is a number');"
		" check_equals(info.filesize, '21482', 'actual filesize');"
		" info.filesize = 'changed';"
		" check_equals(info.filesize, 'changed');" // can be overridden
		" delete info.filesize;"
		" check(!info.hasOwnProperty('filesize'), 'metadata filesize can be deleted');"

		" check(info.hasOwnProperty('audiocodecid'), 'metadata has audiocodecid');"
		" check_equals(typeof(info.audiocodecid), 'number', 'audiocodecid is a number');"
		" check_equals(info.audiocodecid, 2, 'actual audiocodecid');"
		" info.audiocodecid = 'changed';"
		" check_equals(info.audiocodecid, 'changed');" // can be overridden
		" delete info.audiocodecid;"
		" check(!info.hasOwnProperty('audiocodecid'), 'metadata audiocodecid can be deleted');"

		" check(info.hasOwnProperty('stereo'), 'metadata has stereo');"
		" check_equals(typeof(info.stereo), 'boolean', 'stereo is boolean');" 
		" check_equals(info.stereo, false, 'actual stereo');"
		" info.stereo = 'changed';"
		" check_equals(info.stereo, 'changed');" // can be overridden
		" delete info.stereo;"
		" check(!info.hasOwnProperty('stereo'), 'metadata stereo can be deleted');"

		" check(info.hasOwnProperty('audiosamplesize'), 'metadata has audiosamplesize');"
		" check_equals(typeof(info.audiosamplesize), 'number', 'audiosamplesize is a number');"
		" check_equals(info.audiosamplesize, 16, 'actual audiosamplesize');"
		" info.audiosamplesize = 'changed';"
		" check_equals(info.audiosamplesize, 'changed');" // can be overridden
		" delete info.audiosamplesize;"
		" check(!info.hasOwnProperty('audiosamplesize'), 'metadata audiosamplesize can be deleted');"

		" check(info.hasOwnProperty('audiosamplerate'), 'metadata has audiosamplerate');"
		" check_equals(typeof(info.audiosamplerate), 'number', 'audiosamplerate is a number');"
		" check_equals(info.audiosamplerate, 44100, 'actual audiosamplerate');"
		" info.audiosamplerate = 'changed';"
		" check_equals(info.audiosamplerate, 'changed');" // can be overridden
		" delete info.audiosamplerate;"
		" check(!info.hasOwnProperty('audiosamplerate'), 'metadata audiosamplerate can be deleted');"

		" check(info.hasOwnProperty('videocodecid'), 'metadata has videocodecid');"
		" check_equals(typeof(info.videocodecid), 'number', 'videocodecid is a number');"
		" check_equals(info.videocodecid, 2, 'actual videocodecid');"
		" info.videocodecid = 'changed';"
		" check_equals(info.videocodecid, 'changed');" // can be overridden
		" delete info.videocodecid;"
		" check(!info.hasOwnProperty('videocodecid'), 'metadata videocodecid can be deleted');"

		" check(info.hasOwnProperty('height'), 'metadata has height');"
		" check_equals(typeof(info.height), 'number', 'height is a number');"
		" check_equals(info.height, 96, 'actual height');"
		" info.height = 'changed';"
		" check_equals(info.height, 'changed');" // can be overridden
		" delete info.height;"
		" check(!info.hasOwnProperty('height'), 'metadata height can be deleted');"

		" check(info.hasOwnProperty('width'), 'metadata has width');"
		" check_equals(typeof(info.width), 'number', 'width is a number');"
		" check_equals(info.width, 128, 'actual width');"
		" info.width = 'changed';"
		" check_equals(info.width, 'changed');" // can be overridden
		" delete info.width;"
		" check(!info.hasOwnProperty('width'), 'metadata width can be deleted');"

		" check(info.hasOwnProperty('duration'), 'metadata has duration');"
		" check_equals(typeof(info.duration), 'number', 'duration is a number');"
		" check_equals(info.duration, 2.299, 'actual duration');" // seconds, rounded to milliseconds
		" info.duration = 'changed';"
		" check_equals(info.duration, 'changed');" // can be overridden
		" delete info.duration;"
		" check(!info.hasOwnProperty('duration'), 'metadata duration can be deleted');"

		"};"
		));

  SWFMovie_nextFrame(mo);

  SWFMovie_add(mo, (SWFBlock)newSWFAction("totals(101); stop();"));

  SWFMovie_nextFrame(mo);


  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
