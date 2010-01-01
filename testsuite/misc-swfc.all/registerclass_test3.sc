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
 * Test registerClass
 *
 * Description:
 *   frame1: tests simulate the layout of youtube player2.swf
 *   frame2: export libItem1, export libItem2
 *   frame3: DoInitAction(libItem1), DoInitAction(libItem2), PlaceObject(libItem2)
 *           "Object.registerClass('libItem1', theClass1)" is called in DoInitAction(libItem1)
 *           "Object.registerClass('libItem2', theClass2)" is called in DoInitAction(libItem2)
 * Observed:
 *   the effect of "Object.registerClass('libItem2', theClass2)" is not visible
 *   in DoInitAction(libItem2). Note: there's no attachMovie for libItem2 in DoInitAction(libItem2)
 */

// use swf6 for case sensitiviness
.flash  bbox=800x600 filename="registerclass_test3.sc" background=white version=6 fps=12

.frame 1
  .box b1 fill=green width=100 height=100
  
  .sprite xxx
  .end
  
  .initaction xxx:
    // make sure Dejagnu is available in the first frame
    #include "Dejagnu.sc"
  .end
  
  // Define fullDisplay and export it
  .sprite fullDisplay
  .end
  
  .action:  
    note("root DoAction of frame1");
    check_equals(typeof(player.movie), 'movieclip');
    check_equals(player.movie.__proto__, logic_Movie.prototype);
    player.movie.setMovie();
    check_equals(_root.testvar, 100);
  .end
  
  .sprite id141
    // Place a child sprite name it as 'movie'
    .put movie=fullDisplay
  .end

  .initaction id141:
    note("root first InitAction of frame1 (where we check if object placed after is visible)");
    check_equals(typeof(player.movie), 'movieclip');
    check_equals(player.movie.__proto__, MovieClip.prototype);
  .end
  
  // Place sprite id141 and name it as 'player'
  .put player=id141
  
  // Define _Packages.logic.Movie and export it
  .sprite _Packages.logic.Movie
  .end
  
  // Define class logic_Movie
  .initaction _Packages.logic.Movie:
    note("root second InitAction of frame1 (where the class is defined)");
    check_equals(typeof(player.movie), 'movieclip');
    check_equals(player.movie.__proto__, MovieClip.prototype);
    logic_Movie = function() {};
    logic_Movie.prototype = new MovieClip();
    logic_Movie.prototype.setMovie = function () { _root.testvar = 100; };
  .end
  
  // register sprite player.movie to class logic_Movie
  .initaction fullDisplay:
    note("root third InitAction of frame1 (where registerClass is invoked)");
    Object.registerClass("fullDisplay", logic_Movie);
  .end


.frame 2
  .sprite child1
    .put b1  x=100 y=100
  .end
  
  .sprite child2
    .put b1  x=100 y=200
  .end
  
  .sprite libItem1 // Define a sprite libItem1
      .put child1
  .end 
  .sprite libItem2 // Define a sprite libItem2
      .put child2
  .end 
  

.frame 3

  .action:
     // registerClass effects are visible here
     check_equals(libItem2.__proto__, theClass2.prototype);
  .end
    
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
     check_equals(typeof(libItem2), 'movieclip');
     check_equals(libItem2.__proto__, MovieClip.prototype);
     check_equals(libItem2.__proto__, MovieClip.prototype);

     // Childs of libItem2 have also been placed already
     // Gnash fails by executing init actions before frame0 tags
     check_equals(typeof(libItem2.child2), 'movieclip');
  .end
  
  .put libItem2

.frame 4
  .action:
     check_equals(libItem2.__proto__, theClass2.prototype);
  .end
   
.frame 5
  .action:
    totals(19);
    stop();
  .end

  
.end // end of the file


