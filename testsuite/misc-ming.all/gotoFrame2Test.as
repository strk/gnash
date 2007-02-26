// gotoAndPlay() testcase
// Source file provided for reference

// This code is in frame #2 of a 2-frame file!

++counter;

var temp = _currentframe;

check_equals(temp, 2);
check_equals(counter, 1);

// this would emit a function call
//this.gotoAndPlay(temp);

// this would emit a GOTOFRAME tag
//gotoFrame(2);

// this emits a GOTOFRAME2 (GOTOEXPRESSION) tag
// (only if Ming version is 00040004 or higher)
gotoAndPlay(temp);

check_equals(_currentframe, temp);
check_equals(counter, 1); // gotoFrame(_currentframe) is a no-op

totals();
stop();
 
