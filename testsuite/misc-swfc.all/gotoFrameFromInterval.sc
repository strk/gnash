.flash bbox=800x600 filename="gotoFrameFromInterval.swf" background=white version=6 fps=10

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
          gotoAndPlay(6);
       }
       ,0.0001);
       

    _root.framecount = 0; 
    
    this.onEnterFrame = function() {
      _root.framecount++;
      
      if (_root.framecount==10) {
        totals(3);
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


//
// --case2--
// (1) test that a function block shouldn't be interrupted by init actions.
// (2) test interval callbacks shouldn't be interrupted by init actions.
.frame 7
    .action:
        _root.i = 0;
        _root.asOrder = String();
        
        function func() {
            return 'x';
        }
        
        function callback() {
            _root.asOrder += func();
            _root.asOrder += (i++);
            _root.gotoAndPlay(9);
            _root.asOrder += func();
        }
        
        intervalID1 = setInterval(callback, 0.0001);
        intervalID2 = setInterval(callback, 0.0001);
        check(intervalID1 != intervalID2);
        
        trace('frame7');
    .end



.frame 8
    .sprite sp1
    .end
    .sprite sp2
    .end
    .sprite sp3
    .end
    .initaction sp1:
        // clear the interval callbacks
        clearInterval( intervalID1 );
        clearInterval( intervalID2 );
   
        _root.asOrder += (i++);
    .end
    .initaction sp2:
        _root.asOrder += (i++);
    .end
    .initaction sp3:
        _root.asOrder += (i++);
    .end


.frame 10
    .action:
        stop();
        check_equals(_root.asOrder, 'x0xx1x234');
    .end

.end // end of file
