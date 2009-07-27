// SharedObject_as.hx:  ActionScript 3 "SharedObject" class, for Gnash.
//
// Generated on: 20090528 by "bnaugle". Remove this
// after any hand editing loosing changes.
//
//   Copyright (C) 2009 Free Software Foundation, Inc.
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
	
	var nc:NetConnection = new NetConnection();
	// The Adobe flash player only supports remoting with RTMP
	rtmpuri = "rtmp://"+hostname+":"+rtmptport+"/fitcDemo";
	
#if flash9
        var x1:SharedObject = SharedObject.getRemote("sharedobjecttest", rtmpuri, true);
#else
	var x1:SharedObject = SharedObject.getRemote("sharedobjecttest", rtmpuri, true);
#end         
        if (Std.is(x1, SharedObject)) {
            DejaGnu.pass("SharedObject class exists");
        } else {
            DejaGnu.fail("SharedObject class doesn't exist");
        }
        DejaGnu.note("SharedObject type is "+Type.typeof(x1));

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
//	nc.connect(rtmpuri, "test");
	x1.connect(nc);
	DejaGnu.note("Connecting to "+rtmpuri);


// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
#if flash9
 	if (Std.is(x1.client, Dynamic)) {
 	    DejaGnu.pass("SharedObject.client property exists");
 	} else {
 	    DejaGnu.fail("SharedObject.client property doesn't exist");
 	}
	if (Type.typeof(x1.objectEncoding) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.objectEncoding property exists");
	} else {
	    DejaGnu.fail("SharedObject.objectEncoding property doesn't exist");
	}
	if (Type.typeof(x1.size) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.size property exists");
	} else {
	    DejaGnu.fail("SharedObject.size property doesn't exist");
	}
	if (Type.typeof(SharedObject.defaultObjectEncoding) == ValueType.TInt) {
	    DejaGnu.pass("SharedObject.defaultObjectEncoding property exists");
	} else {
	    DejaGnu.fail("SharedObject.defaultObjectEncoding property doesn't exist");
	}
#end
 	if (Std.is(x1.data, Dynamic)) {
 	    DejaGnu.pass("SharedObject.data property exists");
 	} else {
 	    DejaGnu.fail("SharedObject.data property doesn't exist");
 	}
// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::close() method exists");
	} else {
	    DejaGnu.fail("SharedObject::close() method doesn't exist");
	}
	if (Type.typeof(x1.connect) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::connect() method exists");
	} else {
	    DejaGnu.fail("SharedObject::connect() method doesn't exist");
	}
#if flash9
	if (Type.typeof(x1.setDirty) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::setDirty() method exists");
	} else {
	    DejaGnu.fail("SharedObject::setDirty() method doesn't exist");
	}
	if (Type.typeof(x1.setProperty) == ValueType.TFunction) {
	    DejaGnu.pass("SharedObject::setProperty() method exists");
	} else {
	    DejaGnu.fail("SharedObject::setProperty() method doesn't exist");
	}
#end
 	if (Type.typeof(SharedObject.getRemote) == ValueType.TFunction) {
 	    DejaGnu.pass("SharedObject::getRemote() method exists");
 	} else {
 	    DejaGnu.fail("SharedObject::getRemote() method doesn't exist");
 	}

// Call this after finishing all tests. It prints out the totals.
    DejaGnu.done();
    }

#if flash9
    public static function ncOnStatus(evt:NetStatusEvent):Void {
	DejaGnu.note("Got NC onStatus from "+rtmpuri);
//	DejaGnu.note(evt.info.code);//"NetConnection.Connect.Success" or
// 	    "NetStream.Publish.Start" etc.
// 		if( e.info.code == "NetConnection.Connect.Success")
// 		    {
// 			ns = new NetStream(nc);
// 			ns.client=this;
//	    ns.addEventListener(NetStatusEvent.NET_STATUS, onStatus);
// 		    }
	}
#end
    
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

