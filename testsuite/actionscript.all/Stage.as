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


rcsid="$Id: Stage.as,v 1.25 2008/03/13 15:02:32 bwy Exp $";
#include "check.as"

check_equals (typeof(Stage), 'object');

var stageObj = new Stage;
check_equals (typeof(stageObj), 'undefined');

check_equals(Stage.__proto__, Object.prototype);

#if OUTPUT_VERSION > 5

// Stage was implicitly initialized by ASBroadcaster.initialize !
// See http://www.senocular.com/flash/tutorials/listenersasbroadcaster/?page=2
check_equals (typeof(Stage.addListener), 'function');
check_equals (typeof(Stage.removeListener), 'function');
check_equals(typeof(Stage.broadcastMessage), 'function');

check(Stage.hasOwnProperty("_listeners"));
check_equals(typeof(Stage._listeners), 'object');
check(Stage._listeners instanceof Array);

check(Stage.hasOwnProperty("height"));
check(Stage.hasOwnProperty("width"));
check(Stage.hasOwnProperty("scaleMode"));
check(Stage.hasOwnProperty("showMenu"));
check(Stage.hasOwnProperty("align"));
check(Stage.hasOwnProperty("displayState"));

#endif

/// Stage.align

check_equals(typeof(Stage.align), "string");

Stage.align = "T";
check_equals(Stage.align, "T");
Stage.align = "B";
check_equals(Stage.align, "B");
Stage.align = "l";
check_equals(Stage.align, "L");
Stage.align = "R";
check_equals(Stage.align, "R");
Stage.align = "TL";
check_equals(Stage.align, "LT");
Stage.align = "B        R";
xcheck_equals(Stage.align, "RB");
Stage.align = "LThhhhh";
xcheck_equals(Stage.align, "LT");
Stage.align = "B       rhhhh";
xcheck_equals(Stage.align, "RB");
Stage.align = "TR";
check_equals(Stage.align, "TR"); // why???
Stage.align = "RT";
check_equals(Stage.align, "TR");
Stage.align = "lb";
check_equals(Stage.align, "LB");
Stage.align = "BR";
check_equals(Stage.align, "RB");
Stage.align = "LT";
check_equals(Stage.align, "LT");
Stage.align = "X";
check_equals(Stage.align, "");

/// Stage.displayState

check_equals(typeof(Stage.displayState), "string");
check_equals(Stage.displayState, "normal");
Stage.displayState = "fullScreen";
check_equals(Stage.displayState, "fullScreen");
Stage.displayState = "X";
check_equals(Stage.displayState, "fullScreen");
Stage.displayState = "NORMAl";
check_equals(Stage.displayState, "normal");

#if OUTPUT_VERSION > 5

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

totals();
