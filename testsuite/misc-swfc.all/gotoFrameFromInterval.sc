.flash bbox=200x200 filename="gotoandplay_stop.swf" version=6 fps=10

.frame 1
    .action:
#include "Dejagnu.sc"
    .end

.frame 2
  .action:
    _root.jumped = false;
    _root.done = false;

    stop();
    
    note("starting! ($Id: gotoFrameFromInterval.sc,v 1.5 2007/12/06 15:41:30 udog Exp $)");
    
    function doit() {
      note("now jumping...");
      gotoAndPlay(5);
    }
    
    // test:
    setInterval(
        function() {
          if (!_root.jumped && (getTimer() > 1000)) {
            _root.jumped = true;
            doit();
          }
          
          if (!_root.done && (getTimer() > 2000)) {
            _root.done = true;
            
            check_equals(_root._currentframe, 5);
            note("test activated in frame "+_root._currentframe);
            
            //Dejagnu.done();
            
            // BUG NOTICE: jumping to frame 10 makes Gnash restart the movie
            // even with the -1 switch!
            gotoAndPlay(9); 
          }
          
        }
        , 100);
        
    
    
  .end

.frame 5
  .action:
    stop();
    note("frame 5 reached (good)");
  .end

.frame 8
  .action:
    stop();
    note("frame 6 reached (bad)");
  .end
  .stop

.frame 10
  .action:    
    note("goodbye");
    totals(1);
  .end

.end
