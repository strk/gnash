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
 * Test destruction of brother movieClips.
 *
 * Description:
 *   frame2: Define three brother movieclips: mc1, mc2, mc3
 *   frame3: attach them to the main timeline
 *   frame6: brother1 and brother3 get removed by brother2
 *   
 * Expected behaviour:
 *   TODO: add it here.
 * 
 */


.flash  bbox=800x600 filename="movieclip_destruction_test4.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
   _root.as_order = '0+';
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box green_square fill=green width=100 height=100  
  .box red_square fill=red width=100 height=100 
  .box blue_square fill=blue width=100 height=100 

.frame 2
 
  .sprite mc1
    .frame 1 
      .put green_square x=300 y=200
  .end 
   
  .sprite mc2
    .frame 1 
      .put red_square x=300 y=300
    .frame 3
      .action:
        _root.brother1['removeMovieClip']();
        _root.brother3['removeMovieClip']();
      .end
  .end 
  
  .sprite mc3
    .frame 1 
      .put blue_square x=300 y=400
  .end 
  
  
.frame 3
  .action:
    _root.attachMovie("mc1", "brother1", 10);
    _root.attachMovie("mc2", "brother2", 20);
    _root.attachMovie("mc3", "brother3", 30);
  .end

.frame 10
  .action:  
    check_equals(typeof(brother1), 'undefined');
    check_equals(typeof(brother2), 'movieclip');
    check_equals(typeof(brother3), 'undefined');
    stop();
    totals();
  .end

.end

