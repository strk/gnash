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
 *
 * Expected behaviour:
 *    (1) user defined onContruct should not be triggered.
 *    (2) user defined onLoad should not be triggered in this case(when allEventFlags == zero).
 *    (3) If DoAction is before RemoveObject2, then actions in DoAction should be executed before
 *        onUnload, otherwise after onUnload.
 *    (4) Frame actions(frameNum>0): first placed last executed.
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
  
.frame 7
  .action:
    check_equals(_root.as_order1, '0+1+2+3+4+5+6+7+8+9+10+');
    check_equals(_root.as_order2, '0+1+2+3+');
    _root.note(_root.as_order1);
    totals();
    stop();
  .end

  
.end // end of the file


