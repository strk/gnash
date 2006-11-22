// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for Camera ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Camera.as,v 1.7 2006/11/22 09:50:03 strk Exp $";

#include "check.as"

// test the Camera constuctor
var cameraObj = Camera.get();
xcheck (cameraObj != undefined);

// test that Camera.get() returns a singleton
check_equals(cameraObj, Camera.get());

// test that get() method is NOT exported to instances
check_equals (cameraObj.get, undefined);
// test the Camera::setmode method
xcheck (cameraObj.setmode != undefined);
// test the Camera::setmotionlevel method
xcheck (cameraObj.setmotionlevel != undefined);
// test the Camera::setquality method
xcheck (cameraObj.setquality != undefined);
