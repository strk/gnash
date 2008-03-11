// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for NetStream ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: NetStream.as,v 1.21 2008/03/11 19:31:47 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

// Added in SWF6; some versions of the pp won't care, but not
// testing anyway.
// Use changes quite significantly from SWF6 (connections to media
// server) to SWF7 (streaming from various sources).

#else // OUTPUT_VERSION >= 6

check_equals(typeof(NetStream), 'function');
check_equals(typeof(NetStream.prototype), 'object');

var netstreamObj = new NetStream;


/* Constructor */

// test the NetStream constuct FAILED: ! NetStream.prototype.hasOwnProperty('bufferLength')or
check_equals ( typeof(netstreamObj), 'object' );


/* 
 * These properties should not yet exist. They may be added  *
 * when the NetStream connection is established.             *
 *                                                           */

xcheck(! NetStream.prototype.hasOwnProperty('currentFps'));
check(! netstreamObj.hasOwnProperty('currentFps'));

xcheck(! NetStream.prototype.hasOwnProperty('bufferLength'));
check(! netstreamObj.hasOwnProperty('bufferLength'));

xcheck(! NetStream.prototype.hasOwnProperty('bufferTime'));
check(! netstreamObj.hasOwnProperty('bufferTime'));

xcheck(! NetStream.prototype.hasOwnProperty('liveDelay'));
check(! netstreamObj.hasOwnProperty('liveDelay'));

xcheck(! NetStream.prototype.hasOwnProperty('time'));
check(! netstreamObj.hasOwnProperty('time'));

/* Added in SWF7 (still apply to SWF6) */
xcheck(! NetStream.prototype.hasOwnProperty('bytesLoaded'));
check(! netstreamObj.hasOwnProperty('bytesLoaded'));

xcheck(! NetStream.prototype.hasOwnProperty('bytesTotal'));
check(! netstreamObj.hasOwnProperty('bytesTotal'));


/* Subscriber Methods */

# if OUTPUT_VERSION > 5
// test the NetStream::close method
check_equals ( typeof(netstreamObj.close), 'function' );
// test the NetStream::setBufferTime method
check_equals ( typeof(netstreamObj.setBufferTime), 'function');
# else
// this is verified on at least one pp version for SWF5, but we
// aren't testing it. 
// test the NetStream::close method
check_equals ( typeof(netstreamObj.close), 'undefined' );
// test the NetStream::setBufferTime method
check_equals ( typeof(netstreamObj.setBufferTime), 'undefined');
# endif
check_equals ( typeof(netstreamObj.pause), 'function' );
// test the NetStream::play method
check_equals ( typeof(netstreamObj.play), 'function');
// test the NetStream::seek method
check_equals ( typeof(netstreamObj.seek), 'function' );

// receiveAudio (use with media server)
check_equals ( typeof(netstreamObj.receiveAudio()), 'undefined');
// receiveVideo (use with media server)
check_equals ( typeof(netstreamObj.receiveVideo()), 'undefined');

# if OUTPUT_VERSION == 6
check_equals ( typeof(netstreamObj.setbuffertime), 'function');
# else 
// SWF7 up is case-sensitive !
check_equals ( typeof(netstreamObj.setbuffertime), 'undefined');
# endif 


/* Publisher Methods */

// For use with a media server, in SWF6
// test attachAudio
check_equals ( typeof(netstreamObj.attachAudio()), 'undefined');
// test attachVideo
check_equals ( typeof(netstreamObj.attachVideo()), 'undefined');
// test publish
check_equals ( typeof(netstreamObj.publish()), 'undefined');
// test send
check_equals ( typeof(netstreamObj.send()), 'undefined');


/* Event Handlers */

check_equals(typeof(netstreamObj.onPlayStatus), 'undefined');
netstreamObj.onPlayStatus = 4;
check_equals(typeof(netstreamObj.onPlayStatus), 'number');
netstreamObj.onPlayStatus = "str";
check_equals(typeof(netstreamObj.onPlayStatus), 'string');

check_equals(typeof(netstreamObj.onStatus), 'undefined');
netstreamObj.onStatus = 4;
check_equals(typeof(netstreamObj.onStatus), 'number');
netstreamObj.onStatus = "str";
check_equals(typeof(netstreamObj.onStatus), 'string');

check_equals(typeof(netstreamObj.onCuePoint), 'undefined');
netstreamObj.onCuePoint = 4;
check_equals(typeof(netstreamObj.onCuePoint), 'number');
netstreamObj.onCuePoint = "str";
check_equals(typeof(netstreamObj.onCuePoint), 'string');

check_equals(typeof(netstreamObj.onMetaData), 'undefined');
netstreamObj.onMetaData = 4;
check_equals(typeof(netstreamObj.onMetaData), 'number');
netstreamObj.onMetaData = "str";
check_equals(typeof(netstreamObj.onMetaData), 'string');


/* Properties */

// currentFps (read-only)
check_equals ( typeof(netstreamObj.currentFps), 'undefined' );
netstreamObj.currentFps = 'string';
xcheck_equals ( typeof(netstreamObj.currentFps), 'string' );
netstreamObj.currentFps = false;
xcheck_equals ( typeof(netstreamObj.currentFps), 'boolean' );

// bufferLength (read-only)
xcheck_equals ( typeof(netstreamObj.bufferLength), 'undefined' );
netstreamObj.bufferLength = 'string';
xcheck_equals ( typeof(netstreamObj.bufferLength), 'string' );
netstreamObj.bufferLength = false;
xcheck_equals ( typeof(netstreamObj.bufferLength), 'boolean' );

// bufferTime
xcheck_equals ( typeof(netstreamObj.bufferTime), 'undefined' );
netstreamObj.setBufferTime(10);
xcheck_equals(netstreamObj.bufferTime, NULL);
netstreamObj.bufferTime = 20;
xcheck_equals(netstreamObj.bufferTime, 20);
netstreamObj.setBufferTime = 30;
xcheck_equals(netstreamObj.bufferTime, 20);
netstreamObj.setBufferTime(false);
xcheck_equals(netstreamObj.bufferTime, 20);
netstreamObj.setBufferTime('string');
xcheck_equals(netstreamObj.bufferTime, 20);
xnetstreamObj.setBufferTime('5');
xcheck_equals(netstreamObj.bufferTime, 20);

// liveDelay (read-only)
check_equals ( typeof(netstreamObj.liveDelay), 'undefined' );
netstreamObj.liveDelay = 'string';
xcheck_equals ( typeof(netstreamObj.liveDelay), 'string' );

// time (read-only)
xcheck_equals ( typeof(netstreamObj.time), 'undefined' );
netstreamObj.time = 'string';
xcheck_equals ( typeof(netstreamObj.time), 'string' );


/* Two properties added in SWF7 */

// bytesLoaded (read-only)
check_equals ( typeof(netstreamObj.bytesLoaded), 'undefined' );
// bytesLoaded (read-only)
check_equals ( typeof(netstreamObj.bytesTotal), 'undefined' );

#endif // OUTPUT_VERSION >= 6

#if OUTPUT_VERSION < 6
check_totals(0);
#else
check_totals(60);
#endif
