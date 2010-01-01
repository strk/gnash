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
 *  Zou Lunkai, zoulunkai@gmail.com
 * 
 *  test opcode guard.
 *
 *  expected behaviour:
 *     onButtonAction also respects opcode guard. It is guarded by its parent sprite,
 *     not the button DisplayObject itself(button._target).
 *
 *  TODO: 
 *    write a testrunner to support mouse and key presses.
 */


.flash  bbox=800x600 filename="opcode_guard_test3.swf" background=white version=6 fps=12

.frame 1
    .box b1 width=90 height=60 fill=blue
    .box b2 width=90 height=60 fill=green
    .box b3 width=90 height=60 fill=yellow
 
    .action:
        #include "Dejagnu.sc"
        _root.asExecuted1 = false;
        _root.asExecuted2 = false;
        _root.asExecuted3 = false;
        _root.asExecuted4 = false;
        _root.asExecuted5 = false;
        _root.asExecuted6 = false;
        _root.asExecuted7 = false;
        _root.asExecuted8 = false;
        _root.asExecuted9 = false;
    .end


// test1: 
//   place mc1 at frame X, _root.gotoAndPlay(X+2)
//   remove mc1 at frame X+1
// observed:
//   global code in mc1 is guarded by isUnloaded()
.frame 2
    .sprite mc1
        .action:
            _root.gotoAndPlay(4);
            _root.asExecuted1 = true;
        .end
    .end
    .put mc1
.frame 3
    .del mc1
.frame 4
.frame 5
 .action:
  _root.check_equals(asExecuted1, false);
 .end
 
 
// test2: 
//   place mc2 at frame X with onUnload() defined,  _root.gotoAndPlay(X+2)
//   remove mc1 at frame X+1
// observed:
//   global code in mc2 is guarded by isUnloaded()
.frame 6
    .sprite mc2
        .action:
            _root.mc2.onUnload = function () {};
            _root.gotoAndPlay(8);
            _root.asExecuted2 = true;
        .end
    .end
    .put mc2
.frame 7
    .del mc2
.frame 8
    .action:
        _root.check_equals(asExecuted2, false);
    .end
.frame 9


// test3: 
//   place mc3 at frame X, _root.gotoAndPlay(X+2)
//   remove mc3 at frame X+2
// observed:
//   global code in mc3 is guarded by isUnloaded()
.frame 10
    .sprite mc3
        .action:
            _root.mc3.onUnload = function () {};
            _root.gotoAndPlay(12);
            _root.asExecuted3 = true;
        .end
    .end
    .put mc3
.frame 12
    .del mc3
.frame 13
.frame 14
    .action:
        _root.check_equals(asExecuted3, false);
    .end


// test4: 
//   place a button at frame X, _root.gotoAndPlay(X+2)
//   remove the button at frame X+1
//
.frame 30
    .button btn1
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_press:
            _root.gotoAndPlay(32);
            _root.asExecuted4 = true;
        .end
    .end
    .put btn1 x=0 y=300
    .action:
        stop();
        _root.note("Press the button to continue the test!");
    .end
.frame 31
    .del btn1
.frame 32
    .action:
       // the current target for btn1 is _root, never unloaded
        _root.check_equals(asExecuted4, true);
    .end
    
    
// test5: 
//   place a button at frame X, _root.gotoAndPlay(X+3)
//   remove the button at frame X+2
//
.frame 33
    .button btn2
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_press:
            _root.gotoAndPlay(36);
            _root.asExecuted5 = true;
        .end
    .end
    .put btn2 x=100 y=300
    .action:
        stop();
        _root.note("Press the button to continue the test!");
    .end
.frame 34
.frame 35
    .del btn2
.frame 36
    .action:
      // the current target for btn2 is _root, never unloaded
        _root.check_equals(asExecuted5, true);
    .end
    

// test6:
//   place a sprite which contains a button at frame X, _root.gotoAndPlay(X+3)
//   remove the sprite at frame X+1
// observed:
//   opcode in onButtonRelease is guarded by isUnloaded()
.frame 37
    .button btn3
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_release:
            _root.gotoAndPlay(40);
            _root.asExecuted6 = true;
        .end
    .end
    .sprite button_target1
        .put btn3 x=200 y=300
    .end
    .put button_target1
    .action:
        stop();
        _root.note("Press the button and release to continue the test!");
    .end
.frame 38
    .del button_target1
.frame 40
    .action:
        // the current target for btn3 is button_target1, already unloaded
        _root.check_equals(asExecuted6, false);
    .end


// test7:
//   place a sprite which contains a button at frame X,  _root.gotoAndPlay(X+3)
//   remove the sprite at frame X+2
// observed:
//   opcode in onButtonRollOver is guarded by isUnloaded(). 
.frame 41
    .button btn4
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_move_in:
            _root.gotoAndPlay(44);
            _root.asExecuted7 = true;
        .end
    .end
    .sprite button_target2
        .put btn4 x=300 y=300
    .end
    .put button_target2
    .action:
        stop();
        _root.note("Move your mouse over the button to continue the test!");
    .end
.frame 42
.frame 43
    .del button_target2
.frame 44
    .action:
        // the current target for btn4 is button_target2, already unloaded
        _root.check_equals(asExecuted7, false);
    .end

.frame 60
    .action:
        stop();
        totals();
    .end
.end

