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
 * Test actions execution order
 *
 * Description:
 * 
 *  frame3: DoAction: Create user defined onContruct, onLoad and onUnload for both mc1 and mc2.
 *          Place mc1 by PlaceObject2.
 *          DoAction.
 *          Place mc2 by PlaceObject2.
 *          DoAction.
 *
 *  frame4: DoAction.
 *          delete mc1 by RemoveObject2.
 *          DoAction.
 *          delete mc2 by RemoveObject2.
 *          DoAction.
 * 
 *  frame8:
 *          PlaceObject(mc4), DoInitAction(mc4), DoInitAction(mc5), PlaceObject(mc5)
 *               - user defined onInitialize(mc4) isn't called
 *               - user defined onInitialize(mc5) isn't called
 *               - user defined onConstruct(mc4) is called
 *               - user defined onConstruct(mc5) is called
 *
 * Expected behaviour:
 *    (1) user defined onLoad should not be triggered in this case(when allEventFlags == zero).
 *        (guess: might be a pp bug)
 *    (2) If DoAction is before RemoveObject2, then actions in DoAction should be executed before
 *        onUnload, otherwise after onUnload.
 *    (3) Frame actions(frameNum>0): first placed last executed.
 * 
 */


.flash  bbox=800x600 filename="action_execution_order_test10.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
   
   _root.as_order1 = '0+';
   _root.as_order2 = '0+';
    check_equals(_root._currentframe, 1);
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box b1 fill=green width=100 height=100
  .box b2 fill=red width=100 height=100
  .box b3 fill=yellow width=100 height=100
  .box b4 fill=blue width=100 height=100

.frame 2
  
  .sprite mc1 // Define a sprite mc1
  .frame 1
      .put b1
      .action:
        _root.as_order1 += '2+';
      .end
  .frame 3
      .action:
        _root.as_order2 += '3+';
      .end
  .frame 10
  .end 
  
  .sprite mc2 // Define a sprite mc2
  .frame 1
      .put b2
      .action:
        _root.as_order1 += '4+';
      .end
  .frame 3
      .action:
        _root.as_order2 += '2+';
      .end
  .frame 10
  .end
  
  .sprite mc3 // Define a sprite mc3
  .frame 2
      .action:
        _root.as_order2 += '1+';
      .end
  .frame 10
  .end

.frame 3

  .action:
    // user defined onConstruct has no chance to be executed
    mc1.onConstruct = function () {_root.as_order1 += 'xx+';};
    mc2.onConstruct = function () {_root.as_order1 += 'xx+';};
    
    // user defined onLoad won't be triggered if allEventFlags is zero(this case),
    // otherwise, it will be triggered. A PP bug???
    mc1.onLoad = function () {_root.as_order1 += 'YY+';};
    mc2.onLoad = function () {_root.as_order1 += 'YY+';};
    
    mc1.onUnload = function () {_root.as_order1 += '7+';};
    mc2.onUnload = function () {_root.as_order1 += '9+';};
    
    _root.as_order1 += "1+";
  .end
  
  .put mc1 x = 0   y = 300  // Place mc1  
    
  .action:
    _root.as_order1 += "3+";
  .end
  
  .put mc2 x = 100 y = 300  // Place mc2

  .action:
    _root.as_order1 += "5+";
  .end


 
.frame 4
  .put mc3 // Place mc3


.frame 6
  
  .action:
    _root.as_order1 += "6+";
  .end
  
  .del mc1 // delete mc1 by RemoveObject2

  .action:
    _root.as_order1 += "8+";
  .end
  
  .del mc2 // delete mc2 by RemoveObject2
  
  .action:
    _root.as_order1 += "10+";
  .end
  
  .del mc3 // delete mc3 by RemoveObject2
  
.frame 7
  .action:
    check_equals(_root.as_order1, '0+1+2+3+4+5+6+7+8+9+10+');
    check_equals(_root.as_order2, '0+1+2+3+');
  .end


//
// seperate tests for user defined onInitialize, onConstruct, onLoad
//
.frame 8
  .sprite mc4
    .put b4  x=100 y=300
  .end
  .sprite mc5
    .put b4  x=100 y=400
  .end
  .put mc4 // PlaceObject2(mc4)
  .initaction mc4:
    _root.mc4_onConstruct_executed = false;

    _root.note("mc4 init actions"); 
    _root.check_equals(typeof(mc4), 'movieclip');
    // What a bad bug the pp has !
    // First query of __proto__ turns it into the correct prototype
    // (MovieClip.prototype) buf first query returns the *old* rather
    // then the new value 
    // UPDATE: Company knew about this, he mentions that unless for SWF6
    //         the first time a movieclip's __proto__ is queried it always
    //         returns Object.prototype.
    _root.xcheck(mc4.__proto__ == Object.prototype); // returns wrong answer at first, gnash does the right thing here
    _root.check(mc4.__proto__ != Object.prototype); // and correct at second and subsequent queries
    _root.check_equals(mc4.__proto__, MovieClip.prototype); // <--- this is the correct one
    
    mc4.onInitialize = function () { 
      _root.note("mc4 user defined onInitialize"); 
      _root.check(false); // should not be executed
    };
    mc4.onConstruct = function() { 
      _root.note("mc4 user defined onConstruct"); 
      _root.mc4_onConstruct_executed = true;
    };
    mc4.onLoad = function() { 
      _root.note("mc4 user defined onLoad"); 
      _root.check(false); // should not be executed
    };
  .end
  .initaction mc5:
    _root.mc5_onConstruct_executed = false;
    
    mc5.onInitialize = function () { 
      _root.note("mc5 user defined onInitialize"); 
      _root.check(false); // should not be executed
    };
    mc5.onConstruct = function() { 
      _root.note("mc5 user defined onConstruct"); 
      _root.mc5_onConstruct_executed = true;
    };
    mc5.onLoad = function() { 
      _root.note("mc5 user defined onLoad"); 
      _root.check(false); // should not be executed
    };
  .end
  .put mc5 // PlaceObject2(mc5)
  
.frame 9
  .action:
    check_equals(mc4_onConstruct_executed, true);
    check_equals(mc5_onConstruct_executed, true);
  .end

.frame 15
  .action:
    totals(9);
    stop();
  .end
  
  
.end // end of the file


