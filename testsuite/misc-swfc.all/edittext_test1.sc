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
 */


.flash  bbox=800x600 filename="edittext_test1.swf" background=white version=7 fps=1

.frame 1
  .action:
   #include "Dejagnu.sc"
  .end

  .edittext edtext1 size=200% 
            width=400 height=200 
            color=blue border multiline wordwrap
            text="Hello"
            variable="textVar1"
            
  .put edtext1 x=10 y=300
  
  .action:
    // check the initial values
    check_equals(typeof(edtext1), 'object');
    check_equals(edtext1._name, 'edtext1');
    check_equals(edtext1._target, '/edtext1');
    check_equals(edtext1.text, 'Hello');
    check_equals(edtext1.variable, 'textVar1');
    check_equals(_root.textVar1, 'Hello');
  .end


.frame 2
  .action:
    // Update the registered variable
    _root.textVar1 = 'new-string-frame2';   
    check_equals(_root.textVar1, 'new-string-frame2');
    // The return value of TextField.text also updated
    check_equals(edtext1.text, 'new-string-frame2');
  .end

.frame 3
  .action:
    // Update TextField.text
    edtext1.text = 'new-string-frame3';
    check_equals(edtext1.text, 'new-string-frame3');
    // The return value of the registered variable also updated
    check_equals(edtext1.text, 'new-string-frame3');
  .end

.frame 4
  .action:
    // rename the EditText variable to 'textVar2'
    edtext1.variable = 'textVar2'; 
    check_equals(_root.hasOwnProperty('textVar2'), true); 
    check_equals(edtext1.variable, 'textVar2');
    // textVar2 automatically initialized to 'Hello'
    // (the InitialText in DefineTextField tag, make sense!)
    check_equals(_root.textVar2, 'Hello');
    check_equals(edtext1.text, 'Hello');
    check_equals(_root.textVar1, 'new-string-frame3');
  .end


.frame 5
  .action:
    // restore the EditText variable name to 'textVar1'
    edtext1.variable = 'textVar1'; 
    check_equals(edtext1.variable, 'textVar1');
    // edtext1.text also restore to the value of 
    //  _root.textVar1(the registered variable)
    check_equals(edtext1.text, 'new-string-frame3');
  .end


.frame 6
  .action:
    edtext1.text = 'new-string-frame6';
    check_equals(edtext1.text, 'new-string-frame6');
    check_equals(_root.textVar1, 'new-string-frame6');
    
    // Rename the EditText variable to 'textVar3'
    edtext1.variable = 'textVar3'; 
    // textVar3 automatically initialized to 'Hello'
    // (the InitialText in DefineTextField tag, make sense!)
    check_equals(_root.textVar3, 'Hello');
    check_equals(_root.textVar1, 'new-string-frame6');
  .end
  
.frame 7
  .action:
    check_equals(_root.hasOwnProperty('textVar1'), true); 
    check_equals(_root.hasOwnProperty('textVar2'), true); 
    check_equals(_root.hasOwnProperty('textVar3'), true); 
  .end
  
  
.frame 8
  .del edtext1  // Remove edtext1
  .action:
    // after removing the TextField instance, all registered variables still keep alive
    check_equals(_root.hasOwnProperty('textVar1'), true); 
    check_equals(_root.hasOwnProperty('textVar2'), true); 
    check_equals(_root.hasOwnProperty('textVar3'), true); 
    check_equals(typeof(edtext1), 'undefined');
  .end


// new tests, seperate from the above
.frame 9
  .action:
    textVar4 = 'new_tests_begin';
  .end
  
.frame 10
  .edittext edtext2 size=200% 
            width=100 height=100 
            color=blue border multiline wordwrap
            text="Hello"
            variable="textVar4" // give a name already exists in main timeline.
  .put edtext2 x=10 y=300
  .action:
  	// returns the value of the registered variable in main timeline
    check_equals(edtext2.text, 'new_tests_begin');
  .end


.frame 11
  .action:
      edtext2.text = 'value_changed';
      check_equals(edtext2.text, 'value_changed');
      check_equals(textVar4, 'value_changed');
  .end
    
    
.frame 15
  .action:
    totals(31);
    stop();
  .end

 
.end  // file end

