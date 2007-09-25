/*
 *   Copyright (C) 2007 Free Software Foundation, Inc.
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
 * Test registerClass
 *
 */

// use swf6 for case sensitiviness
.flash  bbox=800x600 filename="registerclass_test3.sc" background=white version=6 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
  .end
  
  .box b1 fill=green width=100 height=100


.frame 2
  .sprite libItem1 // Define a sprite libItem1
      .put b1  x=100 y=100
  .end 
  .sprite libItem2 // Define a sprite libItem2
      .put b1  x=100 y=200
  .end 
  

.frame 3
  .initaction libItem1:
     theClass1 = function() { this.testvar = 60;};
     theClass1.prototype = new MovieClip();
     Object.registerClass('libItem1', theClass1);
     
     _root.attachMovie('libItem1', 'clip1', 10);
     check_equals(typeof(clip1), 'movieclip');
     check_equals(clip1.__proto__, theClass1.prototype);
     
     clip1.duplicateMovieClip("dup1", 10);
     check_equals(typeof(dup1), 'movieclip');
     check_equals(dup1.__proto__, theClass1.prototype);
     check_equals(dup1.testvar, 60);
     
     // sprite libItem1 never placed.
     check_equals(typeof(libItem1), 'undefined');
  .end
  .initaction  libItem2:
     theClass2 = function() { this.testvar = 60;};
     theClass2.prototype = new MovieClip();
     Object.registerClass('libItem2', theClass2);
     
     // Gnash failed by executing init actions before DLIST tags.
     xcheck_equals(typeof(libItem2), 'movieclip');
     xcheck_equals(libItem2.__proto__, MovieClip.prototype);
  .end
  
  .put libItem2
  
    
.frame 4
  .action:
    totals(8);
    stop();
  .end

  
.end // end of the file


