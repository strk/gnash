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
 * Test destruction of brother movieClips.
 *
 * Description:
 *   frame2: Export three movieclips: mc1-->mc11, mc2, mc3-->mc31
 *   frame3: attach 6 brother movieclips: brother{1,2,3,4,5,6}
 *   frame5: brother{1,3,4,5,6} are removed by brother2
 *   
 * Expected behaviour:
 *   (1)clipA.removeMovieClip() won't shift its parent or child.
 *   (2)unload a parent automatically unload its children.
 *   (3)whether a child should be unreachable(destroyed) after unload
 *      is not dependent its parent's onUnload.
 *   (4)whether a parent should be unreachable(destroyed) after unload
 *      is dependent on its children' onUnload
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
 
  .sprite mc11
    .frame 1 
      .action:
        _root.note("Running frame1["+this._currentframe+"] actions of mc11["+this._target+"] (adding green square @ 400,200)");
      .end
    .put green_square x=400 y=200
  .end
  
  .sprite mc1
    .frame 1 
      .action:
        _root.note("Running frame1["+this._currentframe+"] actions of mc1["+this._target+"] (adding green square @ 300,200 and mc11)");
      .end
      .put green_square x=300 y=200
      .put mc11
    .frame 6
  .end 
   
  .sprite mc2
    .frame 1 
      .action:
        _root.note("Running frame1["+this._currentframe+"] actions of mc2["+this._target+"] (adding red square)");
      .end
      .put red_square x=300 y=300
    .frame 2 
      .action:
        _root.note("Running frame2["+this._currentframe+"] actions of mc2["+this._target+"] (nothing new)");
      .end
    .frame 3
      .action:
        _root.note("Running frame3["+this._currentframe+"] actions of mc2["+this._target+"] (removing brothers 1,3,4,5,6)");
        _root.brother1['removeMovieClip']();
        _root.brother3['removeMovieClip']();
        _root.brother4['removeMovieClip']();
        _root.brother5['removeMovieClip']();
        _root.brother6['removeMovieClip']();
      .end
  .end 
  
  .sprite mc31
    .put blue_square x=400 y=400
  .end
  
  .sprite mc3
    .frame 1 
      .action:
        _root.note("Running frame1["+this._currentframe+"] actions of mc3["+this._target+"] (adding blue square)");
      .end
      .put blue_square x=300 y=400
      .put mc31
    .frame 6
  .end 
  
  
.frame 3
  .action:
    _root.note("Running frame3 actions of _root (attach brothers)");
    _root.attachMovie("mc1", "brother1", 10);
    _root.attachMovie("mc2", "brother2", 20);
    _root.attachMovie("mc3", "brother3", 30);
    _root.attachMovie("mc3", "brother4", 40);
    _root.attachMovie("mc3", "brother5", 50);
    _root.attachMovie("mc3", "brother6", 60);
    
    // Define a parent onUnload
    brother4.onUnload = function () {
      _root.check_equals(this.getDepth(), -32809);
      // child mc31 has no onUnload defined. child mc31 has been destroyed.
      // Gnash fails by keeping the child alive(referenceable)
      _root.check_equals(typeof(this.mc31), 'undefined');
    };
    
    // Define child onUnload
    brother5.mc31.onUnload = function () {
      // child mc31 has onUnload defined, not shifted.
      _root.check_equals(this.getDepth(), -16382);
      _root.check_equals(typeof(this), 'movieclip');
      _root.check_equals(typeof(this._parent), 'movieclip');
      _root.check_equals(this._parent.getDepth(), -32819);
    };
    
    //
    // Define both parent onUnload and child onUnload
    //
    brother6.onUnload = function () {
      _root.check_equals(this.getDepth(), -32829);
      // child mc31 has onUnload defined, not shifted.
      _root.check_equals(typeof(this.mc31), 'movieclip');
      _root.check_equals(this.mc31.getDepth(), -16382);
    };
    
    brother6.mc31.onUnload = function () {
      // child mc31 not shifted
      _root.check_equals(this.getDepth(), -16382);
    };
  .end

.frame 4
  .action:
    _root.note("Running frame3 actions of _root (nothing new)");
  .end


.frame 5
  .action:
    _root.note("Running frame5 actions of _root");
    check_equals(typeof(brother1), 'undefined');
    check_equals(typeof(brother2), 'movieclip');
    check_equals(typeof(brother3), 'undefined');
    check_equals(typeof(brother4), 'movieclip');
    check_equals(typeof(brother5), 'movieclip');
    check_equals(typeof(brother6), 'movieclip');
  .end
  
  
.frame 10
  .action:  
    check_equals(typeof(brother1), 'undefined');
    check_equals(typeof(brother2), 'movieclip');
    check_equals(typeof(brother3), 'undefined');
    stop();
    totals(19);
  .end

.end

