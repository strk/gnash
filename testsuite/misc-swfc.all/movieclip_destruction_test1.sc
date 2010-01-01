/*
 *   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software Foundation, Inc.
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
 *  frame2: DisplayObject mc1 placed at depth -16381.
 *          mc1 has two frames, _root.gotoAndPlay(6) get executed in it's 2nd frame.
 *  frame3: 
 *  frame4: remove DisplayObject -16381 
 *  frame6: 
 * 
 * Expected behaviour:
 *    (1) only part of the AS in a single action_buffer(in the 2nd frame of mc1) 
 *        get executed;
 *    (2) DisplayObject mc1 get destroied at frame 4.
 *    (3) init actions defined in a passing-by frame get executed (while normal actions don't)
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
    .action:
        note("mc1.frame1");
    .end
    .frame 2    
      .action:
        note("mc1.frame2");
        check_equals(_root.mc1.getDepth(), -16383);
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
    note("root.frame2 (after put mc1)");
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
  .end


.frame 3 
    .action: note("root.frame3 (before definesprite)");
    .end

  .sprite mc2 // Define mc2 and add init_actions for it
    .frame 1
      .put b2 x = 300 y = 300
  .end
  
  .initaction mc2: // Add initactions for mc2(mc2 is not placed)
    note("initaction mc2");
    _root.initActionExecuted = "mc2";
    // mc1 is still alive here, _root.gotoAndPlay(6) hasn't been executed yet.
    // Note mc1 has 2 frames.
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
  .end

    .action: note("root.frame3 (after initaction)");
    .end

.frame 4

  .sprite mc3 // Define mc2 and add initactions for it
    .frame 1
      .put b3 x = 300 y = 300
  .end

    .action:
        note("root.frame4 (before initaction)");
        _root.check(false); // should not be executed
    .end
  
  .initaction mc3: // Add initactions for mc3(mc3 is not placed)
     note("initaction mc3 in root frame4");
    _root.initActionExecuted += ", mc3";
    _root.check_equals(typeof(mc1), 'undefined'); 
    _root.check_equals(typeof(_root.getInstanceAtDepth(-16386)), 'undefined');
  .end

  .action:
     note("root.frame4 (after initaction)");
     _root.check(false); // should not be executed
  .end

  .del mc1  // Remove sprite mc1  

  .action:
     note("root.frame4 (after del mc1)");
     _root.check(false); // should not be executed
  .end


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
    .frame 1  .put b3
  .end
  
  .sprite mc6
    .frame 1 
      .put mc61
      .initaction mc61: // Add initactions for mc6
        _root.initActionExecuted += ", mc61";
        _root.check_equals(this, _root); // target is the root !
      .end
  .end
  
  .put mc6    // Place the movieclip

  .initaction mc6: // Add initactions for mc6
    // Gnash fails by not respecting actions order for initactions
    _root.initActionExecuted += ", mc6";


    // Due to a bug in the proprietary player, we need to trace(__proto__) to
    // force proper construction of the sprites.
    _root.xcheck_equals(mc6.__proto__, Object.prototype); // returns wrong answer at first, gnash does the right thing here
    _root.xcheck_equals(mc7.__proto__, Object.prototype); // returns wrong answer at first, gnash does the right thing here
    trace(mc6.__proto__); trace(mc7.__proto__);
    _root.check_equals(mc6.__proto__, MovieClip.prototype); 
    _root.check_equals(mc7.__proto__, MovieClip.prototype); 

    _root.check_equals(typeof(mc6), 'movieclip'); // Gnash fails because executes init actions before DLIST tags
    _root.check_equals(typeof(mc6.mc61), 'movieclip'); // Gnash fails because executes init actions before DLIST tags
    _root.check_equals(typeof(mc7), 'movieclip'); // Gnash fails because executes init actions before DLIST tags
    _root.check_equals(typeof(mc7.mc71), 'movieclip'); // Gnash fails because executes init actions before DLIST tags
    _root.check_equals(this, _root); // target is the root !
  .end
  
  .sprite mc71
    .action:
          _root.check_equals(this.__proto__, MovieClip.prototype); 
    .end
    .frame 1  .put b3
  .end
  
  .sprite mc7  // Define a movieclip
    .frame 1  .put mc71
  .end
  
  .put mc7    // Place the movieclip
  

.frame 15
  .initaction mc6: //Add initactions for mc6 again.
    x = 0;
    _root.initActionExecuted += ", mc6";
    // This check should not be executed.
    // We should ignore the second init actions for the same sprite.
    // It is here just for detecting some bogus implementation
    _root.check_equals(x, 1);
  .end


.frame 16
  .sprite mc8
  .end
  
  .sprite mc9
    .put mc8
  .end
  
  // test initactions for child sprite.
  .initaction mc8:
    _root.check_equals(this, _root);
    _root.initActionExecuted += ", mc8";
  .end
   
  .action:
    _root.check_equals(initActionExecuted, "mc2, mc3, mc61, mc6, mc8");
    stop();
    totals(39);
  .end
  
.end  // file end

