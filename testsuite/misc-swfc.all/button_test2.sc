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
 * This is a testcase for bug https://savannah.gnu.org/bugs/index.php?22982.
 * To trigger the bug:
 *   1) Make sure GC::collect() always run (don't wait for a threshold of newly allocated GcResources)
 *   2) Run, a red rectangle would appear and disappear every second
 *   3) Move the mouse on the rectangle while it's visible and wait for it to become invisible
 *   4) Move the mouse off the rectangle while it's not visible
 *   5) Wait for the rectangle to appear again and then disappear
 *   6) Segfault should be triggered when the button disappears
 *
 *
 */


.flash  bbox=800x600 filename="button_test2.swf" background=white version=6 fps=1

.frame 1

    .action:
#include "Dejagnu.sc"
    .end

.box rbox width=200 height=300 fill=#FF0000 color=#000000

.sprite buttonChild
	.put rbox
.end

.button button1
    .show buttonChild as=idle
    .show rbox        as=area
    .show rbox        as=hover
.end

.sprite buttonContainer
    .frame 2
    .put button1 pin=center x=100 y=100 scalex=100% scaley=100% alpha=100%
.end

.put buttonContainer 

.end  // file end
