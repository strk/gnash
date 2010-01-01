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
 * Test destruction of nested movieClips.
 *
 * Description:
 *   frame2: Define a nested movieclip mc1(mc1.mc11.mc111.mc111)
 *   frame3: attach the nested movieclip to stage with a name 'nestedMovieClip'
 *   frame10: nestedMovieClip get removed by one of its child(mc111)
 *   
 * Expected behaviour:
 *   TODO: add it here.
 * 
 */


.flash  bbox=800x600 filename="movieclip_destruction_test3.swf" background=white version=7 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
   _root.as_order = '0+';
  .end
  
  // Define 3 shapes(b1, b2, b3)
  .box green_square fill=green width=100 height=100  

.frame 2
  
  .sprite mc1111 
    .frame 1 
      .put green_square x=300 y=300
    .frame 8
      .action:
        _root.x = 400;
        _root.as_order += '1+';
      .end
  .end 
  
  .sprite mc111 
    .frame 1  
      .put mc1111 
    .frame 8
      .action:
        _root.note("nestedMovieClip removed at frame " + _root._currentframe);
        _root.x = 300;
        _root.as_order += '2+';
        _parent._parent['removeMovieClip']();
        _root.note("actions here should not be executed");
         _root.x = 'as_should_be_discarded';
      .end
  .end
  
  .sprite mc11
    .frame 1  
      .put mc111 
    .frame 8
      .action:
        _root.x = 200;
        _root.as_order += '3+';
      .end
    
  .end
  
  .sprite mc1
    .frame 1 
      .put mc11 
    .frame 8
      .action:
        _root.x = 100;
        _root.as_order += '4+';
      .end
  .end
  
  
.frame 3
  .action:
    _root.attachMovie("mc1", "nestedMovieClip", 10);
    check_equals(typeof(nestedMovieClip), 'movieclip');
    check_equals(nestedMovieClip.getDepth(), 10);
    check_equals(typeof(nestedMovieClip.mc11), 'movieclip');
    check_equals(typeof(nestedMovieClip.mc11.mc111), 'movieclip');
    check_equals(typeof(nestedMovieClip.mc11.mc111.mc1111), 'movieclip');
  .end

#define DEFINE_ONUNLOAD
#ifdef DEFINE_ONUNLOAD
  .action:
    // Define onUnload(for deduction)
     nestedMovieClip.onUnload = function () {};
     nestedMovieClip.mc11.mc111.onUnload = function () {};
     nestedMovieClip.mc11.mc111.mc1111.onUnload = function () {};
  .end

.frame 10
  .action:
    // Check 'nestedMovieClip' has unloaded but not destroyed
     check_equals(nestedMovieClip.getDepth(), -32779);
  .end
#endif 


.frame 12
  .action:  
    check_equals(_root.x, 300);
    check_equals(typeof(nestedMovieClip), 'undefined');
    check_equals(_root.as_order, "0+1+2+");
  .end


.frame 15
  .action:
    _root.createEmptyMovieClip("mcA", 10);
    mcARef = mcA;
    mcA.onUnload = function () {
      _root.check_equals(mcA['_root'], _level0);
    };
    mcA['removeMovieClip']();
    _root.check_equals(mcARef.getDepth(), -32779);
    _root.check_equals(mcARef['_root'], _level0);
  .end

.frame 16
  .action:  
    stop();
    totals(12);
  .end

.end

