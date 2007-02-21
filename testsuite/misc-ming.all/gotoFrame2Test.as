// gotoAndPlay() testcase
// Source file provided for reference

// This code is in frame #2 of a 3-frame file!

++counter;

var temp = _currentframe;

check_equals(temp, 2);
check_equals(counter, 1);

// this would emit a function call
//gotoAndPlay(temp);

// this emits a GOTOFRAME2 tag
gotoFrame(temp);

check_equals(_currentframe, temp);
check_equals(counter, 1); // gotoFrame(_currentframe) is a no-op

totals();
stop();
 
