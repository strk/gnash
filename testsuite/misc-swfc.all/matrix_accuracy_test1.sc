/*
 *   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
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
 *
 *
 *  Test swf matrix related calculations.
 *  zou lunkai  zoulunkai@gmail.com
 */ 


.flash  bbox=800x600 filename="matrix_accuracy_test1.swf" background=white version=6 fps=10

.frame 1
    .action:
    #include "Dejagnu.sc"
    .end
    .box rbox width=100 height=30 fill=#FF0000 color=#000000
    .sprite mc1
        .put rbox  
    .end
    .put mc1 x = 0  y = 0

.frame 2
    .action:
        check_equals(mc1._xscale, 100);
        check_equals(mc1._yscale, 100);
        mc1._rotation = 30;
        check_equals(mc1._xscale, 100);
        check_equals(mc1._yscale, 100);
        mc1._rotation = 60;
        check_equals(mc1._xscale, 100);
        check_equals(mc1._yscale, 100);
        mc1._xscale = 200;
        check_equals(mc1._rotation, 60);
        
        // reset mc1 to normal status for later tests.
        mc1._xscale = 100; 
        mc1._rotation = 0;
    .end

.frame 3
    .action:
        mc1._x = 0x3fffffff;
        check_equals(mc1._x, -1);
        mc1._y = 0x3fffff00;
        check_equals(mc1._y, -256);
        mc1._x = mc1._y = 100;
        
        mc1._xscale = -1;
        check_equals(mc1._xscale, -1);
        mc1._xscale = 4294967295.0; // (unsigned)(0xffffffff);
        check_equals(mc1._xscale, 4294967295.0);
        check_equals(mc1._width, 2359295);
        check_equals(mc1._height, 30);
        mc1._yscale = 65536 * 100;
        check_equals(mc1._height, 0);
        mc1._yscale = 65530 * 100;
        check_equals(mc1._height, 180);
        
        // reset mc1 to normal status for later tests.
        // Gnash aborts without this.
        mc1._xscale = 100; 
        mc1._yscale = 100;
    .end
    
    
.frame 4
    // TODO: More tests for MATRIX concatenation, but be aware of compiler overflows(bugs).
    .action:
        stop();
        totals();
    .end
    
.end  // file end

