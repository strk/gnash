//
// This ActionScript code is intended to be compiled in the third frame
// of a movie so composed:
//	frame1) Dejagnu.swf
//	frame2) A bitmap DisplayObject named 'green'
//
// The aim is testing clashes between a variable name
// and a display list DisplayObject name
//

// Move the 'green' DisplayObject on the right 
// so that Dejagnu.swf xtrace window is visible
green._x = 500;
green._xscale = green._yscale = 50;

// Verify that 'green' DisplayObject is a MovieClip
check(green instanceOf MovieClip);
check_equals(typeof(green), 'movieclip');

// Set a reference to the movieclip
greenref = green;
check_equals(typeof(greenref), 'movieclip');

// "create" a 'green' variable.
// The name of this variable will "clash" with the name of the
// existing DisplayObject (added in frame2).
green = new Number(1);

// The *new* 'green' variable is no more a movieclip
check(green instanceOf Number);
check_equals(typeof(green), 'object');
check_equals(green._y, undefined);

// The movieclip reference is still valid
check_equals(typeof(greenref), 'movieclip');

// Change the name of the green DisplayObject
greenref._name = "stealth";
check_equals(typeof(greenref), 'movieclip');

MovieClip.prototype.stealth = 12;
// Only own properties hide chars, not inherited ones
check_equals(typeof(stealth), 'movieclip'); 
greenref._name = "stealth2";
check_equals(typeof(stealth), 'number'); 

// print totals and stop to avoid infinite loops
totals(10);
stop();
