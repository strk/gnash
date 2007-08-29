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
 * Tests about soft reference to sprites.
 *
 * Description:
 *  frame1: create an movieclip and rename it;
 *  frame3: remove the movieclip;
 *  frame5: re-create an movieclip with the original name.
 */


.flash  bbox=200x200 filename="soft_reference_test1.swf" version=6 fps=12

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
    xcheck(mcRef == _level0.mc);
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
    
    totals();
    stop();
  .end
  
.end

