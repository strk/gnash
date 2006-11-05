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

rcsid="$Id: Camera.as,v 1.6 2006/11/05 00:45:27 rsavoye Exp $";

#include "check.as"

var cameraObj = new Camera;

// test the Camera constuctor
check (cameraObj != undefined);

// test the Camera::get method
check (cameraObj.get != undefined);
// test the Camera::setmode method
check (cameraObj.setmode != undefined);
// test the Camera::setmotionlevel method
check (cameraObj.setmotionlevel != undefined);
// test the Camera::setquality method
check (cameraObj.setquality != undefined);
