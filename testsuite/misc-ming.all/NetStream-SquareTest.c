/* 
 *   Copyright (C) 2007, 2008, 2009, 2010, 2011 Free Software Foundation, Inc.
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
char filename2[256];
char filename3[256];


int
main(int argc, char** argv)
{
  SWFMovie mo;
  SWFMovieClip dejagnuclip;
  SWFVideoStream stream;
  SWFDisplayItem item;
  SWFAction a;
  SWFAction b;
  char buffer_a[2024];
  char buffer_b[2024];
  char buffer_c[2024];

  // This is different from the real video width to make sure that
  // Video.width returns the actual width (128).
  int video_width = 130;
  int video_height = 96;

  if ( argc>1 ) mediadir=argv[1];
  else
  {
    fprintf(stderr, "Usage: %s <mediadir>\n", argv[0]);
    return 1;
  }
	
  sprintf(filename, "%s/square.flv", mediadir);
  sprintf(filename2, "%s/square.ogg", mediadir);
  sprintf(filename3, "%s/audio_timewarp.flv", mediadir);

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
	"check(!nc.isConnected, 'newly created NetConnection is not connected');"
	"nc.connect(null);"
	"check(nc.isConnected, 'NetConnection is connected after .connect(null)');"
	"check(!NetStream.prototype.hasOwnProperty('currentFPS'));" // version 7 here
	"xcheck(!NetStream.prototype.hasOwnProperty('currentFps'));"
	"stream = new NetStream();"
	"check_equals ( typeof(stream.bytesTotal), 'undefined' );" // not connected..
	"stream.play('fake');" // just test not to segfault..
	"stream = new NetStream(nc);"
	"check_equals ( typeof(stream.bytesTotal), 'number' );"
	"stream.bytesTotal = 'string';"
	"check_equals ( typeof(stream.bytesTotal), 'number' );"
	"stream2 = new NetStream(nc);"
	"stream3 = new NetStream(nc);"
	, OUTPUT_VERSION);

  sprintf(buffer_b,
  	// bytesTotal (read-only)
	"MovieClip.prototype.addBytesLoadedProgress = function(v, s) {"
	"	var nam = 'blprogress_'+v;"
	"	var dep = this.getNextHighestDepth();"
	"	var pc = this.createEmptyMovieClip(nam, dep);"
	"	pc.stream = s;"
	"	pc.video = v;"
	"	var pcp = pc.createEmptyMovieClip('bar', pc.getNextHighestDepth());"
	"	var x = v._x;"
	"	var y = v._y+v._height+10;"
	"	var w = v._width;"
	"	var h = 10;"
	"	with(pcp) {"
	"		_x = x;"
	"		_y = y;"
	"		moveTo(0,0);"
	"		beginFill(0xFF0000,50);"
	"		lineTo(0, h);"
	"		lineTo(w, h);"
	"		lineTo(w, 0);"
	"		lineTo(0, 0);"
	"		endFill();"
	"	};"
	"	pc.onEnterFrame = function() {"
	"		pcp._xscale = 100*(this.stream.bytesLoaded/this.stream.bytesTotal);"
	"	};"
	"};");

    sprintf(buffer_c,
	"MovieClip.prototype.addBufferLoadedProgress = function(v, s) {"
	"	var nam = 'blprogress_'+v;"
	"	var dep = this.getNextHighestDepth();"
	"	var pc = this.createEmptyMovieClip(nam, dep);"
	"	pc.stream = s;"
	"	pc.video = v;"
	"	var pcp = pc.createEmptyMovieClip('bar', pc.getNextHighestDepth());"
	"	var x = v._x;"
	"	var y = v._y+v._height+22;"
	"	var w = v._width;"
	"	var h = 10;"
	"	with(pcp) {"
	"		_x = x;"
	"		_y = y;"
	"		moveTo(0,0);"
	"		beginFill(0x00FF00,50);"
	"		lineTo(0, h);"
	"		lineTo(w, h);"
	"		lineTo(w, 0);"
	"		lineTo(0, 0);"
	"		endFill();"
	"	};"
	"	pc.onEnterFrame = function() {"
	//"		_root.note(this.video._x+': bufferloaded: '+this.stream.bufferLength+'/'+this.stream.bufferTime);"
	"		pcp._xscale = 100*(this.stream.bufferLength/this.stream.bufferTime);"
	// 		when bufferLength > bufferTime (common):
	"		if ( pcp._xscale > 100 ) {"
	"			pcp._xscale = 100;"
	"			pcp._alpha = 100;"
	"		} else {"
	"			pcp._alpha = 50;"
	"		}"
	"	};"
	"};"
	"stream.play('%s');"
	"stream2.play('%s');"
	"stream3.play('%s');"
	"stream.pause(true);" 
	"stream.paused=true;"
	"_root.metadataNotified=0;"
	"_root.startNotified=0;"
	"_root.stopNotified=0;"
	"stop();",
	filename, filename2, filename3);

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

  stream = newSWFVideoStream();
  SWFVideoStream_setDimension(stream, video_width, video_height);
  item = SWFMovie_add(mo, (SWFBlock)stream);
  SWFDisplayItem_moveTo(item, 400, 0);
  SWFDisplayItem_setName(item, "video2");

  stream = newSWFVideoStream();
  SWFVideoStream_setDimension(stream, video_width, video_height);
  item = SWFMovie_add(mo, (SWFBlock)stream);
  SWFDisplayItem_setName(item, "video3");

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

  //-----------------------------------------
  // Dynamic volume tests using Sound object
  //-----------------------------------------

  // create a movieclip to use as sound controller
  add_actions(mo, "createEmptyMovieClip('dv1', 1);");

  // attach Sound s1 to DisplayObject dv1
  add_actions(mo, "s1 = new Sound(dv1);");
  // Sound.getVolume fetches volume from dv1 (see s2 below)
  check_equals(mo, "s1.getVolume()", "100");

  // Change volume of Sound s1 (will change volume of attached DisplayObject, see below)
  add_actions(mo, "s1.setVolume(1000);");
  check_equals(mo, "s1.getVolume()", "1000"); 

  // attach Sound s2 to DisplayObject dv2
  add_actions(mo, "s2 = new Sound(dv1);");
  // Sound s2 finds volume of dv1 being 1000
  check_equals(mo, "s2.getVolume()", "1000");

  // This shows that setVolume/getVolume make callbacks
  // to attached DisplayObject as changing volume of one Sound
  // influence the other attached sound...
  add_actions(mo, "s2.setVolume(5);");
  check_equals(mo, "s2.getVolume()", "5"); 
  check_equals(mo, "s1.getVolume()", "5"); 
  add_actions(mo, "s1.setVolume(80);");
  check_equals(mo, "s2.getVolume()", "80"); 
  check_equals(mo, "s1.getVolume()", "80");
  // btw, negative volume is fine..
  add_actions(mo, "s1.setVolume(-20);");
  check_equals(mo, "s2.getVolume()", "-20"); 
  check_equals(mo, "s1.getVolume()", "-20");

  // If the attached-to DisplayObject gets unloaded,
  // getVolume returns undefined...
  add_actions(mo, "dv1.removeMovieClip();");
  check_equals(mo, "typeof(s2.getVolume())", "'undefined'"); 
  check_equals(mo, "typeof(s1.getVolume())", "'undefined'");
  // even if you reset it explicitly
  add_actions(mo, "s2.setVolume(50);");
  check_equals(mo, "typeof(s2.getVolume())", "'undefined'"); 
  check_equals(mo, "typeof(s1.getVolume())", "'undefined'");
  // but you get it back when you create a replacement..
  add_actions(mo, "createEmptyMovieClip('dv1', 2);");
  check_equals(mo, "s2.getVolume()", "100"); 
  check_equals(mo, "s1.getVolume()", "100");
  add_actions(mo, "s1.setVolume(80);");
  check_equals(mo, "s2.getVolume()", "80"); 
  check_equals(mo, "s1.getVolume()", "80");

  // And you can attach a Sound to any DisplayObject, not
  // just MovieClip ones.
  // This is against a video instance
  add_actions(mo, "s1 = new Sound(video); s2 = new Sound(video);");
  check_equals(mo, "s2.getVolume()", "100"); 
  check_equals(mo, "s1.getVolume()", "100");
  add_actions(mo, "s1.setVolume(80);");
  check_equals(mo, "s2.getVolume()", "80"); 
  check_equals(mo, "s1.getVolume()", "80");

  // Here's a 3 level volume controller. Our square.flv doesn't have sound
  // so you can't really see if it's working or not... Anyway, change
  // the code to load a movie with sound and see what happens changing the volumes
  // below
  add_actions(mo, 
		"dv1.createEmptyMovieClip('dv2', 1);"
		"dv1.dv2.createEmptyMovieClip('dv3', 1);"
                "dv1.dv2.dv3.attachAudio(stream);" // attach stream to 3rd level
                "s1 = new Sound(dv1);"
                "s2 = new Sound(dv1.dv2);"
                "s3 = new Sound(dv1.dv2.dv3);"
		"s3.setVolume(50);" // ---------- level1 volume (change me for tests)
		"s1.setVolume(50);" // ---------- level2 volume (change me for tests)
		"s2.setVolume(400);" // --------- level3 volume (change me for tests)
                );

  //------------------------------------------
  // Now attach video to the video DisplayObjects
  //------------------------------------------
  check(mo, "Video.prototype.hasOwnProperty('attachVideo')");
  check(mo, "Video.prototype.hasOwnProperty('smoothing')");
  check(mo, "Video.prototype.hasOwnProperty('deblocking')");
  check(mo, "Video.prototype.hasOwnProperty('clear')");
  check(mo, "Video.prototype.hasOwnProperty('height')");
  check(mo, "Video.prototype.hasOwnProperty('width')");
  check_equals(mo, "video.height", "0");
  check_equals(mo, "video.width", "0");
  check_equals(mo, "video2.height", "0");
  check_equals(mo, "video2.width", "0");

  add_actions(mo, "video.attachVideo(stream);"); 
  add_actions(mo, "video2.attachVideo(stream2);"); 
  add_actions(mo, "video3.attachVideo(stream3);"); 
  
  check_equals(mo, "video.height", "0");
  check_equals(mo, "video.width", "0");
  check_equals(mo, "video2.height", "0");
  check_equals(mo, "video2.width", "0");

  // currentFps (read-only)
  check_equals (mo, "typeof(stream.currentFps)", "'number'" );
  add_actions(mo, "stream.currentFps = 'string';");
  check_equals (mo, "typeof(stream.currentFps)", "'number'" );
  add_actions(mo, "stream.currentFps = false;");
  check_equals (mo, "typeof(stream.currentFps)", "'number'" );

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

  check_equals (mo, "stream.currentFps", "0" );

  /* Play video */
  b = newSWFAction(buffer_b);
  if(b == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)b);
  b = newSWFAction(buffer_c);
  if(b == NULL) return -1;
  SWFMovie_add(mo, (SWFBlock)b);
 
  check_equals (mo, "stream.currentFps", "0" );
 
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
  check_equals(mo, "video.height", "0");
  check_equals(mo, "video.width", "0");
  check_equals(mo, "video2.height", "0");
  check_equals(mo, "video2.width", "0");

  check(mo, "Video.prototype.hasOwnProperty('attachVideo')");
  check(mo, "Video.prototype.hasOwnProperty('smoothing')");
  check(mo, "Video.prototype.hasOwnProperty('deblocking')");
  check(mo, "Video.prototype.hasOwnProperty('clear')");
  check(mo, "Video.prototype.hasOwnProperty('height')");
  check(mo, "Video.prototype.hasOwnProperty('width')");

  check(mo, "!Video.prototype.hasOwnProperty('_alpha')");
  check(mo, "!Video.prototype.hasOwnProperty('_height')");
  check(mo, "!Video.prototype.hasOwnProperty('_name')");
  check(mo, "!Video.prototype.hasOwnProperty('_parent')");
  check(mo, "!Video.prototype.hasOwnProperty('_rotation')");
  check(mo, "!Video.prototype.hasOwnProperty('_visible')");
  check(mo, "!Video.prototype.hasOwnProperty('_width')");
  check(mo, "!Video.prototype.hasOwnProperty('_x')");
  check(mo, "!Video.prototype.hasOwnProperty('_xmouse')");
  check(mo, "!Video.prototype.hasOwnProperty('_xscale')");
  check(mo, "!Video.prototype.hasOwnProperty('_y')");
  check(mo, "!Video.prototype.hasOwnProperty('_ymouse')");
  check(mo, "!Video.prototype.hasOwnProperty('_yscale')");
  check(mo, "!Video.prototype.hasOwnProperty('_xmouse')");

  check(mo, "!video.hasOwnProperty('_alpha')");
  check(mo, "!video.hasOwnProperty('_height')");
  check(mo, "!video.hasOwnProperty('_name')");
  check(mo, "!video.hasOwnProperty('_parent')");
  check(mo, "!video.hasOwnProperty('_rotation')");
  check(mo, "!video.hasOwnProperty('_visible')");
  check(mo, "!video.hasOwnProperty('_width')");
  check(mo, "!video.hasOwnProperty('_x')");
  check(mo, "!video.hasOwnProperty('_xmouse')");
  check(mo, "!video.hasOwnProperty('_xscale')");
  check(mo, "!video.hasOwnProperty('_y')");
  check(mo, "!video.hasOwnProperty('_ymouse')");
  check(mo, "!video.hasOwnProperty('_yscale')");
  check(mo, "!video.hasOwnProperty('_xmouse')");

  add_actions(mo,
		"video._x = 100;"
		"video._xscale = 120;"
		"video._yscale = 120;"
		"video._rotation = 45;"
		"_root.addBytesLoadedProgress(video, stream);"
		"_root.addBytesLoadedProgress(video2, stream2);"
		"_root.addBufferLoadedProgress(video, stream);"
		"_root.addBufferLoadedProgress(video2, stream2);"
	);

  check_equals(mo, "video._x", "100")	;
  check_equals(mo, "Math.round(video._xscale*100)/100", "120");
  check_equals(mo, "Math.round(video._yscale*100)/100", "120");
  check_equals(mo, "Math.round(video._rotation*100)/100", "45");


  // How can I test props here ?
  check_equals(mo, "typeof(video.hitTest)", "'undefined'");
  check_equals(mo, "typeof(video.getBounds)", "'undefined'");

  SWFMovie_add(mo, (SWFBlock)newSWFAction(
        "note('The video on the right is an OGG theora stream. It will appear "
        "in Gnash, but not the Adobe player');"
		"_root.onKeyDown = function() {"
		" 	_root.note(' bufferLength:'+stream.bufferLength+"
		" 		' bytesLoaded:'+stream.bytesLoaded+"
		"		' currentFps:'+stream.currentFps+' time:'+stream.time);"
		"	var ascii = Key.getAscii();"
		"	trace('Key down: '+ascii);"
		"	if ( ascii == 32 ) {" // ' ' - pause(toggle)
		"		stream.paused = !stream.paused;"
		"		stream.pause();" // stream.paused);"
		"	}"
		"	else if ( ascii == 112 ) {" // 'p' - play()
		"		stream.play();"
		"               _root.note(\"2. Verify video hasn't started, then press space to continue.\");"
		"	}"
		"	else if ( ascii == 99 ) {" // 'c' - play()
		"		_root.check(nc.isConnected, 'NetConnection is connected');"
		"		_root.nc.close();"
		"               _root.note(\"Closed netconnection\");"
		"		_root.check(!nc.isConnected, 'NetConnection is not connected');"
		"	}"
		"};"
		"Key.addListener(_root);"

		"\n"

		"stream.onStatus = function(info) {"

		"  if ( ! _root.enumerableStatusInfoChecked ) {"
		"    _root.check(info.code != undefined);"
		"    _root.check(info.level != undefined);"
		"    var tmp = new Array();"
		"    for (var e in info) tmp.push(e);"
		"    tmp.sort();"
		"    _root.check_equals(tmp.length, 2);"
		"    _root.check_equals(tmp[0], 'code');"
		"    _root.check_equals(tmp[1], 'level');"
		"    _root.enumerableStatusInfoChecked=true;"
		"    var backup = info.code; info.code = 65;"
		"    _root.check_equals(info.code, 65);"
		"    _root.check(delete info.code);"
		"    _root.check_equals(info.code, undefined);"
		"    info.code = backup;"
		"    var backup = info.level; info.level = 66;"
		"    _root.check_equals(info.level, 66);"
		"    _root.check(delete info.level);"
		"    _root.check_equals(info.level, undefined);"
		"    info.level = backup;"
		"  }"

		"\n"

		// Ignore Buffer.Flush for now
		"  if ( info.code == 'NetStream.Buffer.Flush' ) return; "

		// Print some info
		" _root.note('onStatus('+info.code+') called'); "

		"\n"

		" if ( info.code == 'NetStream.Play.Start' )"
		" {"
		"	check(!_root.startNotified, 'No duplicated Play.Start notification');"
		"	_root.startNotified++;"
		" }"

		" else if ( info.code == 'NetStream.Play.Stop' )"
		" {"
		" 	if ( ! _root.stopNotified )"
		" 	{"
		" 		_root.stopNotified++;"
		" 		stream.seek(0);"
		" 	} else {"
		"		_root.stopNotified++;"
		" 		_root.check(this instanceOf NetStream); "
		" 		_root.check_equals(this.bufferTime, 2); "
		" 		_root.check_equals(this.bytesTotal, 21482); "
		"		_root.nextFrame();"
		"	}"
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

		"\n"

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


		" _root.metadataNotified++;"

		// don't run other tests if already done
		// BTW: should be called once, gnash (gst) calls this twice
		//      and would succeed in composition checking in second call.
		" if ( _root.metadataNotified > 1 ) return;"

		" check(_root.startNotified, 'onMetaData should be notified after Play.Start');"
		" check_equals(arguments.length, 1, 'single argument');"
		" check(info instanceof Array, 'onMetaData argument sent from square.flv should be instanceof Array');"
		" check_equals(info.length, 11);" 

		// Test enumeration
		" var enu = new Array;"
		" for (e in info) { "
		"  enu.push(e);"
		" }"
		" check_equals(enu.length, 11);" // this is actual (not virtual) composition

		"\n"


		// Test composision

		" check(info.hasOwnProperty('length'), 'metadata has length');" // it's an array...

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

		"\n"

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

		"\n"

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

		"\n"

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

		"\n"

		" check(info.hasOwnProperty('duration'), 'metadata has duration');"
		" check_equals(typeof(info.duration), 'number', 'duration is a number');"
		" check_equals(info.duration, 2.299, 'actual duration');" // seconds, rounded to milliseconds
		" info.duration = 'changed';"
		" check_equals(info.duration, 'changed');" // can be overridden
		" delete info.duration;"
		" check(!info.hasOwnProperty('duration'), 'metadata duration can be deleted');"
		" _root.note('1. Press \"p\" key');"
		"};"
		));

  SWFMovie_nextFrame(mo);

  check_equals(mo, "video.height", "96");
  check_equals(mo, "video.width", "128");


  // See https://savannah.gnu.org/bugs/?26687
  SWFMovie_add(mo, (SWFBlock)newSWFAction(
  //"note('VM.time:'+getTimer()+' stream3.time:'+stream3.time);"
  "check(stream3.time > 4150, 'stream3.time '+stream3.time+' > 4150');"
  ));


  //add_actions(mo, "note('This is an OGG stream, so the Adobe player "
  //    "will fail the next two tests.');");
  // gstreamer also fails, so let's not test this.
  //check_equals(mo, "video2.height", "96");
  //check_equals(mo, "video2.width", "128");

  check_equals(mo, "metadataNotified", "1");
  check_equals(mo, "stopNotified", "2");
  check_equals(mo, "startNotified", "1");
  SWFMovie_add(mo, (SWFBlock)newSWFAction("totals(197); stop(); end_of_test=true;"));

  SWFMovie_nextFrame(mo);


  /* Output movie */
  puts("Saving " OUTPUT_FILENAME );
  SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
