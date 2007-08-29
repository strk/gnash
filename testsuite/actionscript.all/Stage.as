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

// Test case for Stage ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: Stage.as,v 1.14 2007/08/29 18:38:35 strk Exp $";

#include "check.as"

check_equals (typeof(Stage), 'object');

var stageObj = new Stage;
check_equals (typeof(stageObj), 'undefined');

check_equals(Stage.__proto__, Object.prototype);

#if OUTPUT_VERSION > 5

// test the Stage::addlistener method
check_equals (typeof(Stage.addListener), 'function');
// test the Stage::removelistener method
check_equals (typeof(Stage.removeListener), 'function');

// Stage was implicitly initialized by ASBroadcaster.initialize !
// See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
xcheck(Stage.hasOwnProperty("_listeners"));
xcheck_equals(typeof(Stage._listeners), 'object');
xcheck(Stage._listeners instanceof Array);

listener = new Object;
listener.onResize = function() {
	_root.note("Resize event received, args to handler: "+arguments.length+" Stage.width="+Stage.width+", Stage.height="+Stage.height);
	Stage.height = 1;
	check(Stage.height != 1);
	// If we delete the Stage object, events won't arrive anymore, but
	// the precedent setting of 'scaleMode' will persist !!
	//delete Stage;
};
Stage.addListener(listener);

// resize events are not sent unless scaleMode == "noScale"
Stage.scaleMode = 5;
check_equals(Stage.scaleMode, "showAll");
Stage.scaleMode = "noScale";

o = new Object();
o.onResize = function() {
	_root.note("Resize event received by deleted object");
};
Stage.addListener(o);
delete o;

#else // OUTPUT_VERSION <= 5

check_equals (typeof(Stage.addListener), 'undefined');
check_equals (typeof(Stage.removeListener), 'undefined');

#endif // OUTPUT_VERSION <= 5

