// gotoAndPlay() testcase
// Source file provided for reference

// This code is in frame #2 of a 3-frame file!

var temp = _currentframe;

if (was_here) {
  trace("NOTE: Frame 2 has been executed again!!");
}
was_here=true;

if (temp!=2) {
 
  trace("UNRESOLVED: _currentframe reports frame "+temp+", can't continue test");
  
} else {

  trace("PASSED: _currentframe reports frame 2");
  
  temp=_currentframe;
  
  gotoAndPlay(temp);
  //gotoAndStop(temp);
  
  if (_currentframe==temp) 
    trace("PASSED: _currentframe is correct after gotoAndPlay() call ("+_currentframe+")");
  else
    trace("FAILED: _currentframe reports frame "+_currentframe+" instead of "+temp);
  
}


