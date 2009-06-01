// NetStream_as.hx:  ActionScript 3 "NetStream" class, for Gnash.
//
// Generated on: 20090601 by "bnaugle". Remove this
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
import flash.net.NetConnection;
import flash.net.NetStream;
import flash.display.MovieClip;
import flash.media.SoundTransform;
#else
import flash.NetStream;
import flash.NetConnection;
import flash.MovieClip;
#end
import flash.Lib;
import Type;

// import our testing API
import DejaGnu;

// Class must be named with the _as suffix, as that's the same name as the file.
class NetStream_as {
    static function main() {
#if !flash6
#if !flash9
        var nc:NetConnection = new NetConnection();
        var x1:NetStream = new NetStream(nc);
#else
		var nc:NetConnection = new NetConnection();
		nc.connect(null);
		var x1:NetStream = new NetStream(nc);
#end
        // Make sure we actually get a valid class        
        if (Std.is(x1, NetStream)) {
            DejaGnu.pass("NetStream class exists");
        } else {
            DejaGnu.fail("NetStream class doesn't exist");
        }
// Tests to see if all the properties exist. All these do is test for
// existance of a property, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
	if (Std.is(x1.bufferLength, Float)) {
	    DejaGnu.pass("NetStream.bufferLength property exists");
	} else {
	    DejaGnu.fail("NetStream.bufferLength property doesn't exist");
	}
	if (Std.is(x1.bufferTime, Float)) {
	    DejaGnu.pass("NetStream.bufferTime property exists");
	} else {
	    DejaGnu.fail("NetStream.bufferTime property doesn't exist");
	}
	if (Std.is(x1.bytesLoaded, Int)) {
	    DejaGnu.pass("NetStream.bytesLoaded property exists");
	} else {
	    DejaGnu.fail("NetStream.bytesLoaded property doesn't exist");
	}
	if (Std.is(x1.bytesTotal, Int)) {
	    DejaGnu.pass("NetStream.bytesTotal property exists");
	} else {
	    DejaGnu.fail("NetStream.bytesTotal property doesn't exist");
	}
	if (Std.is(x1.time, Float)) {
	    DejaGnu.pass("NetStream.time property exists");
	} else {
	    DejaGnu.fail("NetStream.time property doesn't exist");
	}
#if !flash9
	if (Std.is(x1.currentFps, Float)) {
	    DejaGnu.pass("NetStream.currentFPS property exists");
	} else {
	    DejaGnu.fail("NetStream.currentFPS property doesn't exist");
	}
#end
	if (Std.is(x1.liveDelay, Float)) {
	    DejaGnu.pass("NetStream.liveDelay property exists");
	} else {
	    DejaGnu.fail("NetStream.liveDelay property doesn't exist");
	}
#if flash9
	if (Std.is(x1.currentFPS, Float)) {
	    DejaGnu.pass("NetStream.currentFPS property exists");
	} else {
	    DejaGnu.fail("NetStream.currentFPS property doesn't exist");
	}
//FIXME: This property only exists in haXe
	if (Std.is(x1.audioCodec, Int)) {
	    DejaGnu.pass("NetStream.audioCodec property exists");
	} else {
	    DejaGnu.fail("NetStream.audioCodec property doesn't exist");
	}
	if (Std.is(x1.checkPolicyFile, Bool)) {
	    DejaGnu.pass("NetStream.checkPolicyFile property exists");
	} else {
	    DejaGnu.fail("NetStream.checkPolicyFile property doesn't exist");
	}
 	if (Std.is(x1.client, Dynamic)) {
 	    DejaGnu.pass("NetStream.client property exists");
 	} else {
 	    DejaGnu.fail("NetStream.client property doesn't exist");
 	}
//FIXME: This property only exists in haXe
 	if (Std.is(x1.decodedFrames, Int)) {
 	    DejaGnu.pass("NetStream.decodedFrames property exists");
 	} else {
 	    DejaGnu.fail("NetStream.decodedFrames property doesn't exist");
 	}
	if (Std.is(x1.objectEncoding, Int)) {
	    DejaGnu.pass("NetStream.objectEncoding property exists");
	} else {
	    DejaGnu.fail("NetStream.objectEncoding property doesn't exist");
	}
 	if (Std.is(x1.soundTransform, SoundTransform)) {
 	    DejaGnu.pass("NetStream.soundTransform property exists");
 	} else {
 	    DejaGnu.fail("NetStream.soundTransform property doesn't exist");
 	}
//FIXME: This property only exists in haXe
 	if (Std.is(x1.videoCodec, Int)) {
 	    DejaGnu.pass("NetStream.videoCodec property exists");
 	} else {
 	    DejaGnu.fail("NetStream.videoCodec property doesn't exist");
 	}
#end

// Tests to see if all the methods exist. All these do is test for
// existance of a method, and don't test the functionality at all. This
// is primarily useful only to test completeness of the API implementation.
//FIXME: This function exists for AS2 in haXe, but not in Adobe spec
	if (Type.typeof(x1.attachAudio) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::attachAudio() method exists");
	} else {
	    DejaGnu.fail("NetStream::attachAudio() method doesn't exist");
	}
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::close() method exists");
	} else {
	    DejaGnu.fail("NetStream::close() method doesn't exist");
	}
	if (Type.typeof(x1.pause) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::pause() method exists");
	} else {
	    DejaGnu.fail("NetStream::pause() method doesn't exist");
	}
	if (Type.typeof(x1.play) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::play() method exists");
	} else {
	    DejaGnu.fail("NetStream::play() method doesn't exist");
	}
//FIXME: This function exists for AS2 in haXe, but not in Adobe spec
	if (Type.typeof(x1.publish) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::publish() method exists");
	} else {
	    DejaGnu.fail("NetStream::publish() method doesn't exist");
	}
//FIXME: This function exists for AS2 in haXe, but not in Adobe spec
	if (Type.typeof(x1.receiveAudio) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::receiveAudio() method exists");
	} else {
	    DejaGnu.fail("NetStream::receiveAudio() method doesn't exist");
	}
//FIXME: This function exists for AS2 in haXe, but not in Adobe spec
	if (Type.typeof(x1.receiveVideo) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::receiveVideo() method exists");
	} else {
	    DejaGnu.fail("NetStream::receiveVideo() method doesn't exist");
	}
	if (Type.typeof(x1.seek) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::seek() method exists");
	} else {
	    DejaGnu.fail("NetStream::seek() method doesn't exist");
	}
//FIXME: This function exists for AS2 in haXe, but not in Adobe spec
	if (Type.typeof(x1.send) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::send() method exists");
	} else {
	    DejaGnu.fail("NetStream::send() method doesn't exist");
	}
	if (Type.typeof(x1.close) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::close() method exists");
	} else {
	    DejaGnu.fail("NetStream::close() method doesn't exist");
	}
#if !flash9
	if (Type.typeof(x1.setBufferTime) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::setBufferTime() method exists");
	} else {
	    DejaGnu.fail("NetStream::setBufferTime() method doesn't exist");
	}
#end
#if flash9
	if (Type.typeof(x1.attachCamera) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::attachCamera() method exists");
	} else {
	    DejaGnu.fail("NetStream::attachCamera() method doesn't exist");
	}
//FIXME: Property receiveVideoFPS not found on flash.net.NetStream and there is no default value
//	if (Type.typeof(x1.receiveVideoFPS) == ValueType.TFunction) {
//	    DejaGnu.pass("NetStream::receiveVideoFPS() method exists");
//	} else {
//	    DejaGnu.fail("NetStream::receiveVideoFPS() method doesn't exist");
//	}
// AIR only
// 	if (x1.resetDRMVouchers == null) {
// 	    DejaGnu.pass("NetStream::resetDRMVouchers() method exists");
// 	} else {
// 	    DejaGnu.fail("NetStream::resetDRMVouchers() method doesn't exist");
// 	}
	if (Type.typeof(x1.resume) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::resume() method exists");
	} else {
	    DejaGnu.fail("NetStream::resume() method doesn't exist");
	}
// AIR only
// 	if (x1.setDRMAuthenticationCredentials == null) {
// 	    DejaGnu.pass("NetStream::setDRMAuthenticationCredentials() method exists");
// 	} else {
// 	    DejaGnu.fail("NetStream::setDRMAuthenticationCredentials() method doesn't exist");
// 	}
	if (Type.typeof(x1.togglePause) == ValueType.TFunction) {
	    DejaGnu.pass("NetStream::togglePause() method exists");
	} else {
	    DejaGnu.fail("NetStream::togglePause() method doesn't exist");
	}
#end

        // Call this after finishing all tests. It prints out the totals.
        DejaGnu.done();
#end
#if flash6
	DejaGnu.note("This class (NetStream) is only available in flash7, flash8, and flash9");
#end
    }
}

// local Variables:
// mode: C++
// indent-tabs-mode: t
// End:

