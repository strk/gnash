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
 *  Zou Lunkai, zoulunkai@gmail.com
 * 
 *  test opcode guard and setTarget
 */


.flash  bbox=800x600 filename="opcode_guard_test2.swf" background=white version=6 fps=12

.frame 1
  .action:
   #include "Dejagnu.sc"
  .end
  
  
.frame 2
  .sprite mc1 // Define a sprite
    .frame 2
    .action:
      setTarget('/mc1');
      _root.gotoAndPlay(5);
      _root.check(false); // shoudn't be executed
      setTarget('');
    .end
  .end  
  .put mc1    // Place mc1


.frame 4
 .del mc1

.frame 5
  .sprite mc2 // Define mc2
  .end
  .put mc2    // Place mc2
  .action:
    mc2.duplicateMovieClip('dup1', 10);
    mc2.duplicateMovieClip('dup2', 20);
    mc2.duplicateMovieClip('dup3', 30); 
    
    _root.dup3.onUnload = function() {};

    _root.check_equals(typeof(_root.dup1), 'movieclip');
    _root.check_equals(typeof(_root.dup2), 'movieclip');
    
    dup1.testVar = 'dup1_var'; // 
    setTarget('dup1');
      removeMovieClip(_root.dup1);
      // seems Gnash discarded the following 2 tests, caused by opcode guard with
      // current target.  I think we should use the original target.
      _root.check_equals(typeof(_root.dup1), 'undefined');
      _root.check_equals(testVar, undefined);
    setTarget('');
    
    dup2.testVar = 'dup2_var';
    with('dup2'){
      removeMovieClip(_root.dup2);
      // seems Gnash discarded the following 2 tests
      _root.check_equals(typeof(_root.dup2), 'undefined');
      _root.check_equals(testVar, undefined);
    }

    dup3.testVar = 'dup3_var'; // 
    setTarget('dup3');
      removeMovieClip(_root.dup3);
      // dup3 is unloaded but not destroyed
      _root.check_equals(typeof(_root.dup3), 'movieclip');
      _root.check_equals(testVar, 'dup3_var');
    setTarget('');

  .end


.frame 6
  .sprite mc31
    .action:
      setTarget('/mc3/mc32');
      _root.gotoAndPlay(8); // unload it's parent mc3
      _root.check(false);   // shouldn't be executed
      setTarget('');
    .end
  .end
  .sprite mc32
  .end
  .sprite mc3
    .put mc31
    .put mc32
  .end
  .put mc3


.frame 7
  .del mc3

.frame 8
  .sprite mc4
  .end
  .sprite mc5
    .action:
      setTarget('/mc4');
      _root.gotoAndPlay(9);
      _root.testvar = true; // should be executed
      setTarget('');
    .end
  .end
  .put mc4
  .put mc5

.frame 9
  .del mc4

.frame 10
  .action:
    _root.xcheck_equals(testvar, true);
  .end
  
//
// separate tests for setTargetExpression
//
.frame 12
  .action:
     mc100Ref = _root.createEmptyMovieClip("mcA", 100);
     mc100Ref.testvar = 100;
     mc101Ref = _root.createEmptyMovieClip("mcA", 101);
     mc101Ref.testvar = 101;
     _root.check_equals(mc100Ref.testvar, 100);
     _root.check_equals(mc101Ref.testvar, 101);
     
     setTarget(mc100Ref);
      _root.check_equals(testvar, 100);
     setTarget('');
     
     setTarget(mc101Ref);
      // reference mc100Ref.testvar
      _root.check_equals(testvar, 100);
      
      _root.mc99Ref = _root.createEmptyMovieClip("mcA", 99);
      _root.mc99Ref.testvar = 99;
      // still reference mc100Ref.testvar
      _root.check_equals(testvar, 100);
     setTarget('');
     
     _root.check_equals(mc99Ref.testvar, 99);
     
     
     ref200=createEmptyMovieClip('name', 200); 
     ref200.testvar = 200; 
     ref200.onUnload = function() {}; 
     ref201=createEmptyMovieClip('name', 201);
     ref201.testvar = 201; 
     ref200.removeMovieClip(); 
     _root.check_equals(ref200.getDepth(), -32969);
     _root.check_equals(ref201.getDepth(), 201);
     setTarget(ref200);
        _root.check_equals(testvar, 200);
     setTarget('');
     
     setTarget(ref201);
        _root.check_equals(testvar, 200);
     setTarget('');
  .end
  

//
// separate tests for setTarget('/')
//
.frame 13
    .sprite mc6
        .action:
            setTarget('/');
                _root.check_equals(_target, "/");
                gotoAndPlay(15);
            setTarget('');
        .end
    .end
 	.put mc6
 	
.frame 14
    .action:
        _root.check(false); // shoudn't executed!
    .end
    
.frame 15
 
  .action:
    stop();
    // Gnash failed on totals() by discarding some checks.
    xtotals(20);
  .end
  
.end  // file end

