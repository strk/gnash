//
// This ActionScript code is intended to be compiled in the third frame
// of a movie so composed:
//	frame1) Dejagnu.swf
//	frame2) A bitmap character named 'green'
//
// The aim is testing use of Key.addListener
// It is expected that any keypress in the movie makes the 'green' character move
// on the right, and any keyrelease will move it back on the left.
// Pressing multiple keys down will result in a longer right-shift, but
// when releasing all keys, the bitmap returns to its original position.
//


check_equals(typeof(green), 'movieclip');

// Move the 'green' character on the right 
// so that Dejagnu.swf xtrace window is visible
green._x = 200;

l = new Object;
l.onKeyDown = function ()
{
	green._x += 50;
};
l.onKeyUp = function ()
{
	green._x -= 50;
};

check_equals(typeof(Key), 'object');
Key.addListener(l);
Key = 4;
check_equals(typeof(Key), 'number');

totals();
stop();
