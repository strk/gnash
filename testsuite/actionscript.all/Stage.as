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

// Test case for Stage ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="Stage.as";
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
check_equals(Stage.align, "RB");
Stage.align = "LThhhhh";
check_equals(Stage.align, "LT");
Stage.align = "B       rhhhh";
check_equals(Stage.align, "RB");
Stage.align = "TR";
check_equals(Stage.align, "TR");
Stage.align = "RT";
check_equals(Stage.align, "TR");
Stage.align = "lb";
check_equals(Stage.align, "LB");
Stage.align = "BR";
check_equals(Stage.align, "RB");
Stage.align = "LT";
check_equals(Stage.align, "LT");
Stage.align = "LTR";
check_equals(Stage.align, "LTR");
Stage.align = "LTRB";
check_equals(Stage.align, "LTRB");
Stage.align = "TBR";
check_equals(Stage.align, "TRB");
Stage.align = "BT";
check_equals(Stage.align, "TB");
Stage.align = "RL";
check_equals(Stage.align, "LR");
Stage.align = "R mdmdmdmdmdmdmsdcmbkjaskjhasd";
check_equals(Stage.align, "RB");
Stage.align = "xR mdmdmdmdmdmdmsdcmbkjaskjhasd";
check_equals(Stage.align, "RB");
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

stageheightcheck = 0;
rscount = 0;

listener = new Object;
listener.onResize = function() {
	_root.note("Resize event received, args to handler: "+arguments.length+" Stage.width="+Stage.width+", Stage.height="+Stage.height);
	Stage.height = 1;
	stageheightcheck = Stage.height;
	rscount++;
	// If we delete the Stage object, events won't arrive anymore, but
	// the precedent setting of 'scaleMode' will persist !!
	//delete Stage;
};

fscount = 0;
valtype = "";

listener.onFullScreen = function(fs) {
    _root.note("onFullScreen event received: value " + fs);
    valtype = typeof(fs);
    fscount++;
};

Stage.addListener(listener);

// resize events are not sent unless scaleMode == "noScale"
Stage.scaleMode = 5;
check_equals(Stage.scaleMode, "showAll");

Stage.scaleMode = "exactFit";
check_equals(Stage.scaleMode, "exactFit");

Stage.scaleMode = "sHOwall";
check_equals(Stage.scaleMode, "showAll");

Stage.scaleMode = "noBorder";
check_equals(Stage.scaleMode, "noBorder");

Stage.scaleMode = "noScale";
check_equals(Stage.scaleMode, "noScale");

Stage.displayState = "fullScreen";
Stage.displayState = "normal";

// number of calls to Stage.onFullScreen()
check_equals (fscount, 2);

// number of calls to Stage.onResize()
// NOTE: proprietary player for linux is bogus here,
//       in that it always sends an onResize event
//       when scaleMode is set to "noScale" from something else
note("NOTE: Linux version of the proprietary player is known to fail a test (sending a bogus onResize event)");
check_equals (rscount, 0);

// Type of onFullScreen argument
check_equals (valtype, "boolean");

// Try to set in onResize (but it should be read only).
check(stageheightcheck != 1);

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


#if OUTPUT_VERSION > 5
 check_totals(51);
#else
 check_totals(32);
#endif


