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
 *          mc1 has two frams, _root.gotoAndStop(6) get executed in it's 2nd frame.
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


.flash  bbox=800x600 filename="movieclip_destruction_test1.swf" background=white version=6 fps=12

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
        _root.x = 0;
        _root.gotoAndStop(6);
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


.frame 3
  .sprite mc2 // Define mc2 and add init_actions for it
    .frame 1
      .put b2 x = 300 y = 300
  .end
  
  .initaction mc2: // Add initactions for mc2(mc2 is not placed)
    check_equals(typeof(mc1), 'movieclip');
    check_equals(mc1.getDepth(), -16383);
  .end


.frame 4

  .sprite mc3 // Define mc2 and add initactions for it
    .frame 1
      .put b3 x = 300 y = 300
  .end
  
  .initaction mc3: // Add initactions for mc3(mc3 is not placed)
    _root.check_equals(mc1, null);
    _root.check_equals(typeof(mc1), 'undefined');
  .end

  .del mc1  // Remove sprite mc1  


.frame 6 // target frame
  .action:
    check_equals(_root.x, 0);
    stop();
  .end
  
.end

