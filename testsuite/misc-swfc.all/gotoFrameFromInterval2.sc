// just like gotoFrameFromInterval, but using gotoAndPlay 

.flash bbox=800x600 filename="gotoFrameFromInterval2.swf" version=6 fps=10

.frame 1
    .action:
#include "Dejagnu.sc"
    asOrder = '0+';
    .end

.frame 2
  .action:
    stop();
    
    function local_whatever() {}
    
    intervalID = setInterval(
       function() {
          if (_currentframe != 2) return;
          trace("jumping...");
          gotoAndPlay(6);
       }
       ,0.0001);
       
      
    _root.framecount = 0; 
    
    this.onEnterFrame = function() {
      _root.framecount++;
      
      if (_root.framecount==10) {
        check_equals(_root._currentframe, 6);
        totals(2);
      }
    };
       
  .end

.frame 6
    .sprite mc1  // Define a sprite mc1
        .action:
            _parent.init_me(this);
        .end
    .end
    
    .action:
        stop();

        trace("Entering frame 6");
        clearInterval( intervalID );
        
        function init_me(obj) {
            // traces here are just for visual check, can be safely removed.
            // Please don't use _root.note() here, we don't need extra function calls.
            trace(obj);
            trace(obj+" --> 1 =");
            _root.asOrder += '1+';
            trace(obj+" --> 2 ==");
            _root.asOrder += '2+';
            local_whatever();
            trace(obj+" --> 3 ===");
            _root.asOrder += '3+';
            local_whatever();
            trace(obj+" --> 4 ====");
            _root.asOrder += '4+';
        }
    .end    
    
    .put clip1=mc1 // place a named sprite clip1
    .put clip2=mc1 // place a named sprite clip2
    .put clip3=mc1 // place a named sprite clip3
    
    .action:
        check_equals(asOrder, '0+1+2+3+4+1+2+3+4+1+2+3+4+');
    .end
    
    
.frame 7
  .action:
    stop();
    note("Entering unreachable frame 7 !");
  .end


.end // end of file
