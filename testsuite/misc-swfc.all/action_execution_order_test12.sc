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
 * Test actions execution order.
 *
 * movieclips hiberarchy:
 *
 *   _root.frame2.mc1;  
 *   _root.frame2.mc2;  
 *
 *     mc1.frame2.mc11
 *     mc1.frame4.mc12 (mc1 has 4 frames)
 *
 *     mc2.frame3.mc21 (mc2 has 3 frames)
 * 
 */


.flash  bbox=800x600 filename="action_execution_order_test12.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
   
   _root.loadOrder = '0+';
   _root.enterFrameOrder = '0+';
   _root.unloadOrder = '0+';
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box b1 fill=green width=100 height=100
  .box b2 fill=red width=100 height=100
  .box b3 fill=yellow width=100 height=100
  .box b4 fill=blue width=100 height=100

.frame 2
  
  .sprite mc11 // Define mc11
    .frame 1 .put b1
  .end 
  
  .sprite mc12 // Define mc12
    .frame 1 .put b2
  .end
  
  .sprite mc21 // Define mc21
    .frame 1 .put b3
  .end
  
  .sprite mc1 // Define mc1
    .frame 2 .put mc11
    .frame 4 .put mc12
  .end
  
  .sprite mc2 // Define mc2
    .frame 3 .put mc21
  .end
  
  

.frame 3
  .put mc1  // Place mc1
  .put mc2  // Place mc2
  
  
  .action:
    mc1.onLoad = function () {
      _root.loadOrder += '1+';
    };
    mc1.onEnterFrame = function () {
      _root.enterFrameOrder += '2+';
    };
    mc1.onUnload = function () {  
      _root.note('mc1 unloaded');  
      _root.unloadOrder += '4+';  
    };
    
    mc2.onLoad = function () {
      _root.loadOrder += '2+';
    };
    mc2.onEnterFrame = function () {
      _root.enterFrameOrder += '1+';
    };
    mc2.onUnload = function () {
      _root.note('mc2 unloaded');
      _root.unloadOrder += '5+';
    };
    
    check_equals(typeof(mc2), 'movieclip');
    check_equals(typeof(mc2), 'movieclip');
    check_equals(typeof(mc1.mc11), 'undefined');
    check_equals(typeof(mc1.mc12), 'undefined');
    check_equals(typeof(mc1.mc21), 'undefined');
  .end
    

  
.frame 4
  .action:
    mc1.mc11.onLoad = function () {
      _root.loadOrder += '3+';
    };
    mc1.mc11.onEnterFrame = function () {
      _root.enterFrameOrder += '3+';
    };
    mc1.mc11.onUnload = function () {
      _root.note('mc1.mc11 unloaded');
      _root.unloadOrder += '2+';
    };
    
    check_equals(typeof(mc1.mc11), 'movieclip');
    check_equals(typeof(mc1.mc12), 'undefined');
  .end
 

.frame 5
  .action:
    mc2.mc21.onLoad = function () {
      _root.loadOrder += '4+';
    };
    mc2.mc21.onEnterFrame = function () {
      _root.enterFrameOrder += '4+';
    };
    mc2.mc21.onUnload = function () {
     _root.note('mc2.mc21 unloaded');
      _root.unloadOrder += '1+';
    };
    
    check_equals(typeof(mc2.mc21), 'movieclip');
    check_equals(typeof(mc1.mc12), 'undefined');
  .end 

.frame 6
  .action:
    mc1.mc12.onLoad = function () {
      _root.loadOrder += '5+';
    };
    mc1.mc12.onEnterFrame = function () {
      _root.enterFrameOrder += '5+';
    };
    mc1.mc12.onUnload = function () {
        _root.note('mc1.mc12 unloaded');
      _root.unloadOrder += '3+';
    };
    
    check_equals(typeof(mc1.mc12), 'movieclip');
  .end 
  
  
.frame 8
    .del mc1
    .del mc2
  .action:
      check_equals(_root.loadOrder, '0+');
      check_equals(_root.enterFrameOrder, '0+1+2+3+1+2+3+1+2+1+2+');
      // mc2.mc21, mc1.mc11 and mc1.mc12 were unloaded when loop back.
      // mc1 and mc2 were unloaded by RemoveObject2 tags.
      check_equals(_root.unloadOrder, '0+1+2+3+4+5+');
  .end


//
// test2:
//   test that a single action block can be interrupted by passing-by init actions 
.frame 9
    .action:
        _root.asOrder = '0+';
        gotoAndPlay(11);
        _root.asOrder += '1+';
        func = function () { _root.asOrder += '2+'; };
        func();
        _root.asOrder += '4+';
    .end

.frame 10
    .sprite mc3
    .end
    .initaction mc3:
        _root.asOrder += '3+';
    .end
    
.frame 11
    .action:
        check_equals(asOrder, '0+1+2+3+4+');
    .end

.frame 15
    .action:
        totals(); stop();
    .end
    
      
.end // end of the file
