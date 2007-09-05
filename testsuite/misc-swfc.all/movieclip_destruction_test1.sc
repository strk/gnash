/*
 *   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 

/*
 * Zou Lunkai, zoulunkai@gmail.com
 * 
 * Test the exact destruction time about movieclips
 *
 * Timeline:
 * 
 *   Frame  | 1 | 2 | 3 | 4 | 5 | 6 | 7 |
 *  --------+---+---+---+---+---+---+---+
 *   Event  |   |P J|   | R |   | * |   |
 * 
 *  P = place (by PlaceObject2)
 *  R = Remove (by RemoveObject2 tag)
 *  J = jump
 *  * = jump target
 * 
 * Description:
 * 
 *  frame2: character mc1 placed at depth -16381.
 *          mc1 has two frams, _root.gotoAndPlay(6) get executed in it's 2nd frame.
 *  frame3: 
 *  frame4: remove character -16381 
 *  frame6: 
 * 
 * Expected behaviour:
 *    (1) only part of the AS in a single action_buffer(in the 2nd frame of mc1) 
 *        get executed;
 *    (2) character mc1 get destroied at frame 4.
 * 
 */


.flash  bbox=800x600 filename="movieclip_destruction_test1.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box b1 fill=green width=100 height=100
  .box b2 fill=red width=100 height=100
  .box b3 fill=yellow width=100 height=100


.frame 2
  
  .sprite mc1 // Define a sprite named as mc1
    .frame 1
      .put b1 x = 300 y = 300
    .frame 2    
      .action:
        check_equals(mc1.getDepth(), -16383);
        _root.x = 0;
        _root.gotoAndPlay(6);
        // AS below have no chance to be executed.
        // Since mc1 will get removed during gotoFrame above.
        _root.x = 100; 
        _root.note("If you see this message, your player is bogus with action execution order");
      .end
  .end //end of sprite
  
  .put mc1 // Place mc1
  
  .action:
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
  .end

// No matter onUnload defined or not, the above actions still got skipped.
//#define DEFINE_ONUNLOAD
#ifdef DEFINE_ONUNLOAD
  .action:
    // Define onUnload(for deduction)
     mc1.onUnload = function () {};
  .end
#endif

.frame 3 
  .sprite mc2 // Define mc2 and add init_actions for it
    .frame 1
      .put b2 x = 300 y = 300
  .end
  
  .initaction mc2: // Add initactions for mc2(mc2 is not placed)
    // mc1 is still alive here, _root.gotoAndPlay(6) hasn't been executed yet.
    // Note mc1 has 2 frames.
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
  .end


.frame 4

  .sprite mc3 // Define mc2 and add initactions for it
    .frame 1
      .put b3 x = 300 y = 300
  .end
  
  .initaction mc3: // Add initactions for mc3(mc3 is not placed)
    _root.xcheck_equals(mc1, null);
    _root.xcheck_equals(typeof(mc1), 'undefined');
    _root.check_equals(typeof(_root.getInstanceAtDepth(-16386)), 'undefined');
  .end

  .del mc1  // Remove sprite mc1  


.frame 6 // target frame
  .action:
    check_equals(_root.x, 0);
  .end


//
// Seperate tests.
// Test that the whole function body still get executed even when
// 'this' object is null(removed by MovieClip.removeMovieClip()).
.frame 8
  .action:
    _root.createEmptyMovieClip("mc4", 100);
    mc4Ref = mc4;
    check_equals(typeof(_root.mc4), 'movieclip');
        
    mc4.func = function (clip)
    {
      _root.check_equals(this.valueOf(), mc4);
      _root.testvar1 = 100;
      // don't use clip.removeMovieClip here, to avoid bogus compiler conversion.
      // 'removeMovieClip' is converted to lower case with the above format.
      // This is a swf7 file.
      clip['removeMovieClip']();  
      _root.check_equals(typeof(_root.mc4), 'undefined');
      _root.check_equals(typeof(this), 'movieclip');
      _root.check_equals(this.valueOf(), null);  // this pointer is null!
      _root.testvar2 = 200;
    };
    
    mc4.func(mc4); // invoke the function and remove mc4
    
    check_equals(_root.testvar1, 100);
    check_equals(_root.testvar2, 200);
    check_equals(typeof(_root.mc4Ref), 'movieclip');  
    check_equals(_root.mc4Ref.valueOf(), undefined);
  .end

//
// seperate tests.
// similar to tests in frame8, but onUnload is defined for the given movieClip
.frame 10
  .action:  
    _root.createEmptyMovieClip("mc5", 200);
    check_equals(typeof(_root.mc5), 'movieclip');
    
    mc5.onUnload = function () {}; // Define onUnload for mc5
    
    mc5.func = function (clip)
    {
      _root.check_equals(this.valueOf(), mc5);
      _root.testvar1 = 300;
      clip['removeMovieClip']();
      _root.check_equals(typeof(_root.mc5), 'movieclip');
      _root.check_equals(typeof(this), 'movieclip');
      _root.check_equals(this, _root.mc5);  
      _root.testvar2 = 400;
    };
    
    mc5.func(mc5); // invoke the function and remove mc5
    
    check_equals(_root.testvar1, 300);
    check_equals(_root.testvar2, 400);
    check_equals(typeof(_root.mc5), 'movieclip');  
    check_equals(mc5.getDepth(), -32969); 
  .end

//
// Seperate tests for DoInitAction.
//
.frame 12
  .sprite mc61  // Define a movieclip
    .frame 1  b3
  .end
  
  .sprite mc6
    .frame 1  .put mc61
  .end
  
  .put mc6    // Place the movieclip
  
  .initaction mc6: // Add initactions for mc6
    // Gnash fails by not respecting actions order for initactions
    _root.xcheck_equals(typeof(mc6), 'movieclip');
    _root.xcheck_equals(typeof(mc6.mc61), 'movieclip');
    _root.xcheck_equals(typeof(mc7), 'movieclip');
  .end
  
  .sprite mc7  // Define a movieclip
    .frame 1  b3
  .end
  
  .put mc7    // Place the movieclip
  
  .action:
    stop();
    totals();
  .end
  
.end  // file end

