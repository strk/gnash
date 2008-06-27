// testcase for gotoAndXXXX(frame-label) when invoked as AS function

.flash bbox=800x600 filename="gotoFrameLabelAsFunction.swf" version=6 fps=10

.frame 1
    .action:
#include "Dejagnu.sc"
    asOrder = '0+';
    .end

.frame 2
  .action:
    
    this.onEnterFrame = function() {
    	_root.framecount++;
    	if (_root.framecount==10) {
    		check_equals(_root._currentframe, 5);
    		totals(1);
    	}
    };
        
    // the "_root." part is important!
    _root.gotoAndPlay("dest");
           
  .end

.frame 4
  .action:
    trace("reached label 4 (wrong)");
    check(0);
    stop();       
  .end

.frame 5 name="dest"
  .action:
    trace("reached label 5 (correct)");
    stop();       
  .end

.frame 6
  .action:
    trace("reached label 6 (wrong)");
    check(0);
    stop();       
  .end
  
.end // end of file
