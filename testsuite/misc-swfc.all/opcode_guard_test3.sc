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
 *  Zou Lunkai, zoulunkai@gmail.com
 * 
 *  test opcode guard.
 *
 *  onButtonActions are 'insane'(hard to understand) at the moment.
 *  See comments in each test.
 *
 *  TODO: 
 *    (1) more tests and develop a more general model about the opcode guard.
 *    (2) write a testrunner to support mouse and key presses.
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
            _root.gotoAndPlay(13);
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
// observed:
//   opcode in onButtonPress is guarded by isUnloaded() in this case
.frame 30
    .button btn1
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_press:
            _root.gotoAndPlay(32);
            _root.asExecuted5 = true;
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
        _root.check_equals(asExecuted4, false);
    .end
    
    
// test5: 
//   place a button at frame X, _root.gotoAndPlay(X+3)
//   remove the button at frame X+2!
// observed insane behaviour:
//   opcode in onButtonPress is NOT guarded by isUnloaded()/isDestroyed() in this case
// comments:
//   the behaviour is strange to me! As it is not consistent with test1~3, where
//   all opcode guard works as expected. But this is a very common case is real swfs.
//   And many of the confusing bugs were caused by the onButtionActions.
//   Seems the difference is caused by the removing time of the buttons. test6~8
//   show similar cases. 
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
        _root.check_equals(asExecuted5, true);
    .end
    

// test6:
//   place a button at frame X, _root.gotoAndPlay(X+3)
//   remove the button at frame X+2!
// observed insane behaviour:
//   opcode in onButtonRelease is NOT guarded by isUnloaded()/isDestroyed() in this case
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
    .put btn3 x=200 y=300
    .action:
        stop();
        _root.note("Press the button and release to continue the test!");
    .end
.frame 38
.frame 39
    .del btn3
.frame 40
    .action:
        _root.check_equals(asExecuted6, true);
    .end


// test7:
//   place a button at frame X, _root.gotoAndPlay(X+3)
//   remove the button at frame X+2!
// observed insane behaviour:
//   opcode in onButtonRollOver is NOT guarded by isUnloaded()/isDestroyed() in this case 
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
    .put btn4 x=300 y=300
    .action:
        stop();
        _root.note("Move your mouse over the button to continue the test!");
    .end
.frame 42
.frame 43
    .del btn4
.frame 44
    .action:
        _root.check_equals(asExecuted7, true);
    .end


// test8:
//   place a button at frame X, _root.gotoAndPlay(X+3)
//   remove the button at frame X+2!
// observed:
//   opcode in onButtonRollOut is NOT guarded by isUnloaded()/isDestroyed() in this case
.frame 45
    .button btn5
        .show b1 as=idle
        .show b2 as=hover
        .show b3 as=pressed
        .on_move_out:
            _root.gotoAndPlay(48);
            _root.asExecuted8 = true;
        .end
    .end
    .put btn5 x=400 y=300
    .action:
        stop();
        _root.note("Move your mouse across the button to continue the test!");
    .end
.frame 46
.frame 47
    .del btn5
.frame 48
    .action:
        _root.check_equals(asExecuted8, true);
    .end    
    

.frame 60
    .action:
        stop();
        totals();
    .end
.end

