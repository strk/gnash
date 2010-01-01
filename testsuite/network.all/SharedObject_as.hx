// SharedObject_as.hx:  ActionScript 3 "SharedObject" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009, 2010 Free Software Foundation, Inc.
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

// This test case must be processed by CPP before compiling to include the
//  DejaGnu.hx header file for the testing framework support.


#if flash9
import flash.net.SharedObject;
import flash.display.MovieClip;
import flash.net.NetConnection;
import flash.events.NetStatusEvent;
import flash.net.NetStream;
#else
import flash.MovieClip;
import flash.SharedObject;
import flash.NetConnection;
import flash.NetStream;
#end
import flash.Lib;
import Type;
import Std;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class SharedObject_as {
    static var undef = untyped __global__[ "undefined" ];
    static var hostname : String;
    static var rtmptport : String;
    static var rtmpport : String;
    static var rtmpuri : String;

    static function main() {
	
	// -P FlashVars='hostname=localhost,rtmptport5080=rtmpport=1935'
	if (hostname == undef) {
	    hostname="localhost";
	    DejaGnu.note("No hostname specified, defaulting to "+hostname);
	}
	
	if (rtmptport == undef) {
	    rtmptport = "5080";
	    DejaGnu.note("No RTMPT port specified, defaulting to "+rtmptport);
	}
	
	if (rtmpport == undef) {
	    rtmpport = "1935";
	    DejaGnu.note("No RTMP port specified, defaulting to "+rtmpport);
	}
	
	// The Adobe flash player only supports remoting with RTMP
	rtmpuri = "rtmp://"+hostname+":"+rtmpport+"/fitcDemo";

	var nc:NetConnection = new NetConnection();
// 	addEventListeners();
	nc.connect(rtmpuri);
	DejaGnu.note("Connecting to "+rtmpuri);

#if flash9
        var x1:SharedObject = SharedObject.getRemote("rsoltest", nc.uri, true);
#else
	var x1:SharedObject = SharedObject.getRemote("rsoltest", nc.uri, true);
#end         

        if (Std.is(x1, SharedObject)) {
            DejaGnu.pass("SharedObject class exists");
        } else {
            DejaGnu.fail("SharedObject class doesn't exist");
        }
//        DejaGnu.note("SharedObject type is "+Type.typeof(x1));

// 	var ns:NetStream = new NetStream(nc);
#if flash9
	nc.addEventListener(NetStatusEvent.NET_STATUS, ncOnStatus);
#else
	nc.setID = function(id) {
	    DejaGnu.note("Got a setID() from "+rtmpuri);
	}
        x1.onStatus = function(e):Void {
	    DejaGnu.note("Got onStatus from "+rtmpuri);
 	    if (e.level == "error") {
		DejaGnu.note("ERROR: " +e.code);
	    }
	};
        x1.onSync = function(list):Void {
	    DejaGnu.note("Got onSync from "+rtmpuri);
	    DejaGnu.note(list[0].code);
	};
//  	    DejaGnu.note(e.code);
//  	    DejaGnu.note(e.description);
#end
  	x1.connect(nc);
// 	x1.addEventListener(SyncEvent.SYNC, syncHandler);

	x1.data.whoami = "me";
	x1.send("rsoltest", "Hello World");

	DejaGnu.note("Sent SharedObject to "+rtmpuri);

#if flash9
	x1.setDirty("data");

	x1.setProperty("data", nc);
#end

// Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
    }
    
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

