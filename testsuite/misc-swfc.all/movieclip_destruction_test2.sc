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
 * Test destruction of static movieclips and soft references
 *
 * Description:
 * 
 *  frame2: Place mc1, mc2, mc3 by PlaceObject2 tag.
 *          Create a soft reference mc1Ref for mc1, mc2Ref for mc2, mc3Ref for mc3.
 *          Define onUnload for mc2, define onUnload for mc3.
 *          Define mc2.testvar = 100, define mc3.testvar = new Number(100);
 *
 *  frame3: Remove mc1, mc2, mc3 by RemovieObject2 tag
 * 
 *
 * Expected behaviour:
 *    (1) mc1Ref in frame3 is dangling.
 *    (2) mc2Ref, mc3Ref, mc2 and mc3 are still accessible in frame3.
 *    (3) Movieclip.swapDepths() does not work for mc2 and mc3 in frame3.
 *    (4) 'testvar' of mc2 and mc3 keep alive after onUnload called.
 *    (5) mc2Ref, mc3Ref are dangling at frame4.
 * 
 */


.flash  bbox=800x600 filename="movieclip_destruction_test2.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box b1 fill=green width=100 height=100
  .box b2 fill=red width=100 height=100
  .box b3 fill=yellow width=100 height=100

.frame 2
  
  .sprite mc1 // Define a sprite mc1
    .frame 1
      .put b1 x = 0 y = 0
  .end //end of sprite
  
  .sprite mc2 // Define a sprite mc2
    .frame 1
      .put b2 x = 0 y = 0
  .end
  
  .sprite mc3 // Define a sprite mc3
    .frame 1
      .put b3 x = 0 y = 0
  .end
  
  .put mc1 x = 100 y = 300 // Place mc1
  .put mc2 x = 200 y = 300 // Place mc2
  .put mc3 x = 300 y = 300 // Place mc3
  
  .action:
  
    _root.mc2UnlaodedCount = 0;
    _root.mc3UnlaodedCount = 0;
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
    check_equals(mc2.getDepth(), -16382);
    check_equals(mc3.getDepth(), -16381);
    // Define a onUnload for mc2 and mc3
    mc2.onUnload = function ()  
    { 
       _root.check_equals(mc2.getDepth(), -16387); // already shifted inside unload handler !
       _root.check_equals(this.getDepth(), -16387); // ...
       _root.mc2UnlaodedCount++;     
       // mc2.testvar keeps alive as long as mc2 is alive
       _root.check_equals(mc2.testvar, 100); 
    };
    mc3.onUnload = function ()  
    { 
       _root.mc3UnlaodedCount++; 
       _root.check_equals(mc3.testvar, 100);
    };
    
    mc2.testvar = 100;
    mc3.testvar = new Number(100);
    
    // Create soft references for mc1 and mc2 and mc3
    mc1Ref = mc1;
    mc2Ref = mc2;
    mc3Ref = mc3;
  .end


.frame 3
  .del mc1 // Remove mc1 by RemoveObject2
  .del mc2 // Remove mc2 by RemoveObject2
  .del mc3 // Remove mc3 by RemoveObject2

  .action: 
    check_equals(mc2UnlaodedCount, 1); // mc2.onUnload triggered
    check_equals(mc3UnlaodedCount, 1); // mc3.onUnload triggered
    check_equals(mc1Ref.valueOf(), null);
    check_equals(mc2Ref, mc2);
    check_equals(mc3Ref, mc3);
    
    check_equals(typeof(mc1), 'undefined'); // cann't access the hard reference
    // mc1 is destroyed. it is not in the removed depth zone.
    check_equals(_root.getInstanceAtDepth(-16386), undefined); 
    check_equals(typeof(mc2), 'movieclip'); // mc2 is still accessable
    check_equals(typeof(mc3), 'movieclip'); // mc3 is still accessable
    check_equals(mc2.getDepth(), -16387);   // depth of mc2 changed after onUnload
    check_equals(mc3.getDepth(), -16388);   // depth of mc3 changed after onUnload
    
    mc2.swapDepths(mc3);      
    check_equals(mc2.getDepth(), -16387);  // depth not change after swapDepths
    check_equals(mc3.getDepth(), -16388);  // depth not change after swapDepths
    
    mc2.swapDephts(-10);
    mc2.swapDephts(10);
    check_equals(mc2.getDepth(), -16387);  // depth not change after swapDepths
    check_equals(mc3.getDepth(), -16388);  // depth not change after swapDepths
    
    check_equals(mc2.testvar, 100);       
    check_equals(mc3.testvar, 100); 
    mc2.removMovieClip();
    mc3.removMovieClip();
    check_equals(mc2UnlaodedCount, 1); // mc2.onUnload not triggered again
    check_equals(mc2UnlaodedCount, 1); // mc3.onUnload not triggered again
    check_equals(typeof(mc2), 'movieclip'); // mc2 is still accessible
    check_equals(typeof(mc3), 'movieclip'); // mc3 is still accessible
    check_equals(mc2.getDepth(), -16387); 
    check_equals(mc3.getDepth(), -16388);  
    check_equals(mc2._x, 200); 
    check_equals(mc3._y, 300);  
    check_equals(mc2.testvar, 100); 
    check_equals(mc3.testvar, 100); 
    
    mc2.onUnload();
    mc3.onUnload();
    check_equals(mc2UnlaodedCount, 2);  // we can still invoke onUnload
    check_equals(mc2UnlaodedCount, 2);  // we can still invoke onUnload
  .end


.frame 4
  .action:
    check_equals(typeof(mc1), 'undefined');
    check_equals(typeof(mc2), 'undefined');
    check_equals(typeof(mc3), 'undefined');
    check_equals(mc1Ref.valueOf(), null);
    check_equals(mc2Ref.valueOf(), null);
    check_equals(mc3Ref.valueOf(), null);
  .end
  
// Seperate tests for Movieclip.swapDepths
.frame 5 
    .put mc1 x = 100 y = 300 // Place mc1
    .action:
      check_equals(mc1.getDepth(), -16380);
      mc1.swapDepths(-16385); // doesn't work, can't swap mc1 to a depth below -16384
      check_equals(mc1.getDepth(), -16380);
      mc1.swapDepths(-16384); // works
      check_equals(mc1.getDepth(), -16384);
      mc1.swapDepths(-32769); // doesn't work, can't swap mc1 to a depth below -16384 
      check_equals(mc1.getDepth(), -16384);
      mc1.swapDepths(-402770); // doesn't work, can't swap mc1 to a depth below -16384 
      check_equals(mc1.getDepth(), -16384);
    .end

.frame 6
  .action:
    totals(52);
    stop();
  .end

  
.end // end of the file

