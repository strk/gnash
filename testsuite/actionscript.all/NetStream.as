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

rcsid="$Id: NetStream.as,v 1.14 2007/11/23 12:53:26 bwy Exp $";

#include "check.as"

#if OUTPUT_VERSION < 6

// A Player version >= 7 can
// still provide the class, altought limiting it's use
//check_equals(typeof(NetStream), undefined);

#else // OUTPUT_VERSION >= 6

check_equals(typeof(NetStream), 'function');

var netstreamObj = new NetStream;


/* Constructor */

// test the NetStream constuctor
check_equals ( typeof(netstreamObj), 'object' );


/* Subscriber Methods */

// test the NetStream::close method
check_equals ( typeof(netstreamObj.close), 'function' );
// test the NetStream::pause method
check_equals ( typeof(netstreamObj.pause), 'function' );
// test the NetStream::play method
check_equals ( typeof(netstreamObj.play), 'function');
// test the NetStream::seek method
check_equals ( typeof(netstreamObj.seek), 'function' );
// test the NetStream::setBufferTime method
check_equals ( typeof(netstreamObj.setBufferTime), 'function');
// receiveAudio (media server)
xcheck_equals ( typeof(netstreamObj.receiveAudio()), 'function');
// receiveVideo (media server)
xcheck_equals ( typeof(netstreamObj.receiveVideo()), 'function');

// SWF7 up is case-sensitive !
check_equals ( typeof(netstreamObj.setbuffertime), 'undefined');



/* Publisher Methods */

// For use with a media server, from SWF6
// test attachAudio
xcheck_equals ( typeof(netstreamObj.attachAudio()), 'function');
// test attachVideo
xcheck_equals ( typeof(netstreamObj.attachVideo()), 'function');
// test publish
xcheck_equals ( typeof(netstreamObj.publish()), 'function');
// test send
xcheck_equals ( typeof(netstreamObj.send()), 'function');
	

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
xcheck_equals ( typeof(netstreamObj.currentFps), 'number' );
// bufferLength (read-only)
check_equals ( typeof(netstreamObj.bufferLength), 'number' );
// bufferTime
check_equals ( typeof(netstreamObj.bufferTime), 'number' );
// liveDelay (read-only)
xcheck_equals ( typeof(netstreamObj.liveDelay), 'number' );
// time (read-only)
check_equals ( typeof(netstreamObj.time), 'number' );


/* Two properties added in SWF7 */

// bytesLoaded (read-only)
check_equals ( typeof(netstreamObj.bytesLoaded), 'number' );
// bytesLoaded (read-only)
check_equals ( typeof(netstreamObj.bytesTotal), 'number' );


/* Writeable Properties */

// bufferTime (the only writeable property?)
netstreamObj.setBufferTime(10);
check_equals(netstreamObj.bufferTime, 10);

#endif // OUTPUT_VERSION >= 6
totals();
