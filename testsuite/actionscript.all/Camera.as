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

// The .get() method is a class method, not exported
// to instances.
check(Camera.get);
xcheck_equals(cameraObj.get, undefined); 

trace("Camera.get() returns: "+Camera.get());

// test that the methods do not exist in the class
xcheck_equals(Camera.setmode, undefined);
xcheck_equals(Camera.setmotionlevel, undefined);
xcheck_equals(Camera.setquality, undefined);

#if OUTPUT_VERSION < 7
check (cameraObj.setmode); 
check (cameraObj.setmotionlevel);
check (cameraObj.setquality);
#else
xcheck_equals (cameraObj.setmode, undefined); 
xcheck_equals (cameraObj.setmotionlevel, undefined);
xcheck_equals (cameraObj.setquality, undefined);
#endif

#endif // OUTPUT_VERSION >= 6
totals();
