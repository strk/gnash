// 
//   Copyright (C) 2005, 2006, 2007, 2009 Free Software Foundation, Inc.
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


rcsid="$Id: Camera.as,v 1.15 2008/03/11 19:31:46 strk Exp $";
#include "check.as"

#if OUTPUT_VERSION < 6

xcheck_equals(typeof(Camera), 'function');

#else // OUTPUT_VERSION >= 6

//trace("NOTE: System.capabilities.hasVideoEncoder:  " + System.capabilities.hasVideoEncoder);

// test the Camera constuctor
check(Camera);
var cameraObj = new Camera;
check(cameraObj);
var cameraObj2 = new Camera();
check(cameraObj2);

check(cameraObj != cameraObj2);
check_equals(typeof(cameraObj), 'object');

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

check(cameraObj.setMode); 
check(cameraObj.setMotionLevel);
check(cameraObj.setQuality);
check_equals(typeof(cameraObj.setMode), "function");
check_equals(typeof(cameraObj.setMotionLevel), "function");
check_equals(typeof(cameraObj.setQuality), "function");

#endif // OUTPUT_VERSION >= 6
totals();
