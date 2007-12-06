.flash bbox=200x200 filename="gotoandplay_stop.swf" version=6 fps=10

#include "check.sc"


.frame 1
    .action:
#include "Dejagnu.sc"
    .end

.frame 2
  .action:
    _root.jumped = false;
    _root.done = false;

    stop();
    
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
            note("Test done");

            Dejagnu.done();
            
            gotoAndPlay("10"); // end
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
  .end

.end
