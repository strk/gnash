//
// This ActionScript code is intended to be compiled in the third frame
// of a movie so composed:
//	frame1) Dejagnu.swf
//	frame2) A bitmap character named 'green'
//
// The aim is testing clashes between a variable name
// and a display list character name
//

// Move the 'green' character on the right 
// so that Dejagnu.swf xtrace window is visible
green._x = 200;

// Verify that 'green' character is a MovieClip
check(green instanceOf MovieClip);
check_equals(typeof(green), 'movieclip');

// "create" a 'green' variable.
// The name of this variable will "clash" with the name of the
// existing character (added in frame2).
green = new Number(1);

// The *new* 'green' variable is no more a movieclip
check(green instanceOf Number);
check_equals(typeof(green), 'object');
check_equals(green._y, undefined);

// print totals and stop to avoid infinite loops
totals();
stop();
