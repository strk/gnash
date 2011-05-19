// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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

// Test case for Camera ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Camera.as";
#include "check.as"

#if OUTPUT_VERSION < 6

check_equals(typeof(Camera), 'function');

#else // OUTPUT_VERSION >= 6

//trace("NOTE: System.capabilities.hasVideoEncoder:  " + System.capabilities.hasVideoEncoder);

// This is not the proper way to construct a camera object. According to the
// livedoc a camera should be retrieved using Camera.get(). The constructor
// returns an object with camera functions.

//test the Camera constuctor
check_equals(typeof(Camera), "function");

var cameraObj = new Camera;
check(cameraObj);
var cameraObj2 = new Camera();
check(cameraObj2);
check(cameraObj != cameraObj2);
check_equals(typeof(cameraObj), 'object');
check_equals(typeof(cameraObj.setCursor), "function");
check_equals(typeof(cameraObj.setKeyFrameInterval), "function");
check_equals(typeof(cameraObj.setLoopback), "function");

// These properties are never present before Camera.get() is called.
check(!Camera.prototype.hasOwnProperty("activityLevel"));
check(!Camera.prototype.hasOwnProperty("bandwidth"));
check(!Camera.prototype.hasOwnProperty("currentFps"));
check(!Camera.prototype.hasOwnProperty("fps"));
check(!Camera.prototype.hasOwnProperty("height"));
check(!Camera.prototype.hasOwnProperty("index"));
check(!Camera.prototype.hasOwnProperty("width"));
check(!Camera.prototype.hasOwnProperty("motionLevel"));
check(!Camera.prototype.hasOwnProperty("motionTimeout"));
check(!Camera.prototype.hasOwnProperty("muted"));
check(!Camera.prototype.hasOwnProperty("name"));
check(!Camera.prototype.hasOwnProperty("quality"));


// The .get() method is a class method, not exported
// to instances.
check(Camera.get);
check_equals(cameraObj.get, undefined); 

var cam = Camera.get();

trace("Camera.get() returns: " + cam);

// Static properties
check(!Camera.prototype.hasOwnProperty("names"));
check(!Camera.prototype.hasOwnProperty("get"));

// These properties are added to the prototype if a camera is present
// after get is called.
if (cam) {
    check(Camera.prototype.hasOwnProperty("activityLevel"));
    check(Camera.prototype.hasOwnProperty("bandwidth"));
    check(Camera.prototype.hasOwnProperty("currentFps"));
    check(Camera.prototype.hasOwnProperty("fps"));
    check(Camera.prototype.hasOwnProperty("height"));
    check(Camera.prototype.hasOwnProperty("index"));
    check(Camera.prototype.hasOwnProperty("width"));
    check(Camera.prototype.hasOwnProperty("motionLevel"));
    check(Camera.prototype.hasOwnProperty("motionTimeout"));
    check(Camera.prototype.hasOwnProperty("muted"));
    check(Camera.prototype.hasOwnProperty("name"));
    check(Camera.prototype.hasOwnProperty("quality"));
}
else {
    check(!Camera.prototype.hasOwnProperty("activityLevel"));
    check(!Camera.prototype.hasOwnProperty("bandwidth"));
    check(!Camera.prototype.hasOwnProperty("currentFps"));
    check(!Camera.prototype.hasOwnProperty("fps"));
    check(!Camera.prototype.hasOwnProperty("height"));
    check(!Camera.prototype.hasOwnProperty("index"));
    check(!Camera.prototype.hasOwnProperty("width"));
    check(!Camera.prototype.hasOwnProperty("motionLevel"));
    check(!Camera.prototype.hasOwnProperty("motionTimeout"));
    check(!Camera.prototype.hasOwnProperty("muted"));
    check(!Camera.prototype.hasOwnProperty("name"));
    check(!Camera.prototype.hasOwnProperty("quality"));
}

// test that the methods do not exist in the class
check_equals(Camera.setMode, undefined);
check_equals(Camera.setMotionLevel, undefined);
check_equals(Camera.setQuality, undefined);
check_equals(Camera.setCursor, undefined);

check_equals ( typeof(cam.setMode), 'function' );
check_equals ( typeof(cam.setMotionLevel), 'function' );
check_equals ( typeof(cam.setQuality), 'function');
check_equals ( typeof(cam.setCursor), 'function');
check_equals ( typeof(cam.setKeyFrameInterval), 'function');
check_equals ( typeof(cam.setLoopback), 'function');

// check properties
check_equals ( typeof(cam.activityLevel), 'number' );
check_equals ( typeof(cam.bandwidth), 'number');
check_equals ( typeof(cam.currentFps), 'number');
check_equals ( typeof(cam.fps), 'number');
check_equals ( typeof(cam.height), 'number');
check_equals ( typeof(cam.index), 'string'); //differs from spec, testing real bhvior
check_equals ( typeof(cam.motionLevel), 'number');
check_equals ( typeof(cam.motionTimeout), 'number');
check_equals ( typeof(cam.muted), 'boolean');
check_equals ( typeof(cam.name), 'string');
check_equals ( typeof(cam.names), 'undefined');
check_equals ( typeof(cam.quality), 'number');
check_equals ( typeof(cam.width), 'number');

// check initialized values as in spec (gnash initializes to default)
check_equals ( cam.activityLevel, -1 );
check_equals ( cam.bandwidth, 16384);
check_equals ( cam.height, 120);
check_equals ( cam.motionLevel, 50);
check_equals ( cam.motionTimeout, 2000);
check_equals ( cam.muted, true);
check_equals ( cam.width, 160);

// setting and getting

// check Camera::setMode with no args
cam.setMode();
check_equals ( cam.width, 160);
check_equals ( cam.height, 120);
check_equals ( cam.fps, 15);

// check Camera::setMode with all arguments set
cam.setMode(320, 280, 30, false);
check_equals ( cam.width, 320);
check_equals ( cam.height, 280);
check_equals ( cam.fps, 30);

// check Camera::setMotionLevel with no args
cam.setMotionLevel();
check_equals (cam.motionLevel, 50);
check_equals (cam.motionTimeout, 2000);

// check Camera::setMotionLevel with all arguments set
cam.setMotionLevel(75, 3000);
check_equals (cam.motionLevel, 75);
check_equals (cam.motionTimeout, 3000);

// check Camera::setMotionLevel with bad motionLevel argument
cam.setMotionLevel(200, 2000);
check_equals (cam.motionLevel, 100);

// check Camera::setQuality with no args
cam.setQuality();
check_equals (cam.bandwidth, 16384);
check_equals (cam.quality, 0);

// check Camera::setQuality with all arguments set
cam.setQuality(18000, 75);
check_equals (cam.bandwidth, 18000);
check_equals (cam.quality, 75);

// check Camera::setQuality with bad quality value
cam.setQuality(18000, 420);
check_equals (cam.bandwidth, 18000);
check_equals (cam.quality, 100);

#endif // OUTPUT_VERSION >= 6
totals();
