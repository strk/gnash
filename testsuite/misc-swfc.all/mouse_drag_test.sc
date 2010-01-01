/*
 *   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
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
 *  Test startDrag and endDrag
 *
 *  Expected behaviour:
 *
 *    static movieclips are immune to static transformation after startDrag.
 */

.flash  bbox=800x600 filename="mouse_drag_test.sc" background=white version=6 fps=12

.frame 1
    .action:
        #include "Dejagnu.sc"
    .end

.frame 2
    .box b1 color=white fill=green width=50 height=50 
    .sprite mc1
        .put shape1=b1 x=100 y=100
    .end
    .put mc1
    
    .action:
        // check the existence of mc1.shape1
        check_equals(typeof(mc1.shape1), 'movieclip');
        // the name of the shape is evaluated to its parent clip
        check_equals(mc1.shape1, mc1);
        mc1.startDrag(true);
        check_equals(mc1._x, 0);
        check_equals(mc1._y, 0);
    .end

.frame 3
    .jump mc1 x=200 y=200 //MOVE
    .action:
        // static transformation does not work after startDrag
        check(mc1._x != 200);
        check(mc1._y != 200);
        mc1.stopDrag();
    .end

.frame 4
    .jump mc1 x=300 y=300
    .action:
        // static transformation does not work even after stopDrag
        check(mc1._x != 300);
        check(mc1._y != 300);
    .end

.frame 5
    .action:
        // enable dragging again, just for visual check.
        mc1.startDrag(true, 200, 200, 300, 300);
    .end
    
.frame 6
    .action:
        totals(8);
        stop();
    .end

.end // end of the file


