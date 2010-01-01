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
 * Tests about soft reference to sprites.
 *
 * Description:
 *  frame1: create an movieclip and rename it;
 *  frame3: remove the movieclip;
 *  frame5: re-create an movieclip with the original name.
 */


.flash  bbox=800x600 filename="soft_reference_test1.swf" background=white version=6 fps=12

.frame 1
  .action:
  #include "Dejagnu.sc"

    //create a movieclip
    mcRef = this.createEmptyMovieClip("mc", 10);
    check(typeof(mc)=='movieclip');  
    check(typeof(mcRef)=='movieclip'); 
    check(mc == _level0.mc);    
    check(mcRef == _level0.mc); 
  
    //change the "_name" property
    mc._name = 'changed';
  .end

.frame 3
  .action:
    check(typeof(mc)=='undefined');   
    check(typeof(mcRef)=='movieclip');
    check(mc == undefined);           
    check(mcRef == _level0.changed); 
     
    // remove the created moiveclip in frame1
    changed.removeMovieClip();
  .end

.frame 5
  .action:
    check(typeof(mc) == 'undefined');    
    check(typeof(mcRef) == 'movieclip'); 
    check(mc == undefined);  
    check(mcRef.valueOf() == null); 
    
    //re-create a movieclip again, mcRef connects to the newly created movieclip
    this.createEmptyMovieClip("mc", 20);
    check(typeof(mc)=='movieclip');  
    check(typeof(mcRef)=='movieclip'); 
    check(mc == _level0.mc);    
    // Gnash bug:
    //   Target of the sprite mcRef points to changed, so seeking
    //   for the new target fails (_level0.changed doesn't exist)
    check(mcRef == _level0.mc);
  .end

.frame 7
  .action:
    // change the instance name again
    mc._name = "changed_again";
    
    check(typeof(changed_again)=='movieclip'); 
    check(mcRef != _level0.changed_again);
    
    check(mc == undefined); 
    check(typeof(mcRef)=='movieclip'); 
    check(mcRef.valueOf() == null);
    
    // change the instance name back
    mc._name = "mc";
    check(typeof(mc)=='undefined'); 
    check(mc == undefined); 

    check(typeof(mcRef)=='movieclip'); 
    check(mcRef.valueOf() == null); 
    
    // Release resources after testing
    delete mc;
    delete mcRef;
    changed_again.removeMovieClip();
  .end


// seperate tests in frame9
.frame 9
  .action:
    _root.createEmptyMovieClip("mc1", 30);
    mc1._name = "mc2";
    mcRef = mc2;
    
    check(typeof(mcRef)=='movieclip');
    check_equals(mcRef.getDepth(), 30);
    check(mcRef == _level0.mc2);
    
    mc2.removeMovieClip();
    _root.createEmptyMovieClip("mc2", 40);

    check(typeof(mcRef)=='movieclip');
    // Gnash bug:
    //   Target of the sprite pointed to by mcRef is
    //   not the one used on creation (_level0.mc1) but the one
    //   subsequently changed to by effect of _name assignment: _level0.mc2.
    //   Thus, when finding a *new* DisplayObject (the old one was unloaded)
    //   we find the *new* _level0.mc2.
    //   Should be fixed in the same way as for the bug exposed in frame 5
    check(mcRef.valueOf() == null) 
    
    // release resources after testing
    delete mcRef;
    mc2.removeMovieClip();
  .end

// seperate tests in frame11
.frame 11
  .action:
    mcContainer = new Array(10);
    i = 0;
    MovieClip.prototype.onConstruct = function ()
    {
      mcContainer[i++] = this;
      note("Constructed "+this+" in depth "+this.getDepth()+" and assigned to mcContainer["+(i-1)+"]");
    };
    _root.createEmptyMovieClip("mc1", 50);
    _root.createEmptyMovieClip("mc1", 51);
    check_equals(mcContainer[0].getDepth(), 50);
    check_equals(mcContainer[1].getDepth(), 51);
    check_equals(mc1.getDepth(), 50);
    
    mc1._name = "mc2"; // char at depth 50 changes name
    mcRef = mc2; // mcRef now points to char at depth 50, and target _level0.mc2
    
    check(typeof(mcRef)=='movieclip');
    check_equals(mcRef.getDepth(), 50);
    check(mcRef == _level0.mc2);
    
    mc2.removeMovieClip();
    _root.createEmptyMovieClip("mc2", 60);

    check(typeof(mcRef)=='movieclip');

    // Gnash bug:
    //   Still the same bug: the ref uses *current* target instead of the one
    //   as of creation time.
    check(mcRef == _level0.mc1);
    
    _root.totals(38);
    stop(); 
  .end
  
.end

