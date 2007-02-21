// gotoAndPlay() testcase
// Source file provided for reference

// This code is in frame #2 of a 3-frame file!

note("Executing actions in frame "+_currentframe+", counter = "+counter);
if ( ++counter < 2 ) {
	var temp = _currentframe;
	check_equals(temp, 2);
	//gotoAndPlay(temp); // this is just a function call, and works fine
	note("About to execute GOTOFRAME2");
	gotoFrame(temp); // this emits a GOTOFRAME2 tag
	note("After GOTOFRAME2, _currentframe is "+_currentframe);
	check_equals(_currentframe, temp);
} else {
	totals();
	stop();
}
 
