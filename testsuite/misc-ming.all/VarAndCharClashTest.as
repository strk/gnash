//
// This ActionScript code is intended to be compiled in the third frame
// of a movie so composed:
//	frame1) Dejagnu.swf
//	frame2) A bitmap character named 'green'
//
// The aim is testing clashes between a variable name
// and a display list character name
//

// hide the 'green' character so that Dejagnu.swf xtrace window is visible
green._visible = false;

// "create" a 'green' variable.
// The name of this variable will "clash" with the name of the
// existing character (added in frame2).
green = 1;

// We expect that getting the 'green' label returns
// our "variable" rather then the character.
xcheck_equals(green, 1);

// print totals and stop to avoid infinite loops
totals();
stop();
