 // 
//   Copyright (C) 2007, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; check not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fcheckth Floor, Boston, MA  02110-1301  USA
//

/*
 *  Zou Lunkai, zoulunkai@gmail.com
 *
 *  Test ActionEnumerate/2
 */


rcsid="enumerate.as";
#include "check.as"

#if OUTPUT_VERSION > 5

//
// enumerate characters of a sprite
//
{
  // Create a recorder and init it.
  recorder =  new Array(10);
  for(i=0; i<10; i++){
    recorder['mc'+i] = 0;
  }

  // Create six characters
  for(i=0; i<6; i++){
    _root.createEmptyMovieClip('mc'+i, i+1);
  }

  // Use the recorder to record the enumerated results in _root
  i = 0;
  for (var j in _root){
    recorder[j.toString()] = _root[j];
    i++;
  } 

  // Test what we got in the above step. Tests show that characters were also got enumerated!
  check_equals(typeof(recorder['mc0']), 'movieclip');
  check_equals(typeof(recorder['mc1']), 'movieclip');
  check_equals(typeof(recorder['mc2']), 'movieclip');
  check_equals(typeof(recorder['mc3']), 'movieclip');  
  check_equals(typeof(recorder['mc4']), 'movieclip');
  check_equals(typeof(recorder['mc5']), 'movieclip');
  check_equals(_root.hasOwnProperty('mc0'), false);
  check_equals(_root.hasOwnProperty('mc1'), false);
  check_equals(_root.hasOwnProperty('mc2'), false);
  check_equals(_root.hasOwnProperty('mc3'), false);
  check_equals(_root.hasOwnProperty('mc4'), false);
  check_equals(_root.hasOwnProperty('mc5'), false);
  delete recorder;
}

//
// enumerate properties of an AS object
//
{
  recorder =  new Array(10);
  
  // initialize the recorder
  for(i=0; i<10; i++){
    recorder['x'+i] = 0;
  }

  obj = new Object();
  obj.x1 = function () {};
  obj.x2 = 1;
  obj.x3 = Array();

  i = 0;
  for (var j in obj){
    // equivalent to recorder[j.toString()] = obj[j.toString()];
    recorder[j.toString()] = obj[j];
    i++;
  } 
  
  check_equals(typeof(recorder['x1']), 'function');
  check_equals(typeof(recorder['x2']), 'number');
  check_equals(typeof(recorder['x3']), 'object');
  check_equals(obj.hasOwnProperty('x1'), true);
  check_equals(obj.hasOwnProperty('x2'), true);
  check_equals(obj.hasOwnProperty('x3'), true);
  delete recorder;
}

enumerateObj = function(object) {
   list = ""; 
    for (el in object) {
        list += el + ",";
    }
    return list;
};

/// Try different enumerations.
{
    list = "";
    o = {};

    o.a = 3;
    check_equals(enumerateObj(o), "a,");

    o.b = "string";
    check_equals(enumerateObj(o), "b,a,");

    o["el"] = 5;
    check_equals(enumerateObj(o), "el,b,a,");
    
    o[8] = new Date();
    check_equals(enumerateObj(o), "8,el,b,a,");
    
    o.b = 8;
    check_equals(enumerateObj(o), "8,el,b,a,");

    delete o.b;
    check_equals(enumerateObj(o), "8,el,a,");

    o.b = "string again";
    check_equals(enumerateObj(o), "b,8,el,a,");

    r = o.u;
    check_equals(enumerateObj(o), "b,8,el,a,");

    t = {};
    o[t] = 9;
    check_equals(enumerateObj(o), "[object Object],b,8,el,a,");

    delete o["8"];
    check_equals(enumerateObj(o), "[object Object],b,el,a,");

    o.c = Object.prototype.toString;
    check_equals(enumerateObj(o), "c,[object Object],b,el,a,");

    o[9] = 7;
    check_equals(enumerateObj(o), "9,c,[object Object],b,el,a,");
    
    o["6"] = 8;
    check_equals(enumerateObj(o), "6,9,c,[object Object],b,el,a,");

}

extras = 0;

#ifdef MING_SUPPORTS_ASM
 extras += 6;

// The first test (enumerate) is possible with any asm-supporting ming, 
// the second only with version from 2009-05-20 or later.
 a = new Object();
 a.a = 1;
 a.b = "string";
 
 var r0, r1, r2, r3, r4;
 
 
 // We make sure the stack is clear, then push 'stackfirst', then enumerate
 // and see what all the stack values are.
 asm {
    pop
    pop
    push 'stackfirst'
    push 'a'
    enumerate
    setregister r:0
    push 'r0', r:0
    setvariable
    pop
    setregister r:1
    push 'r1', r:1
    setvariable
    pop
    setregister r:2
    push 'r2', r:2
    setvariable
    pop
    setregister r:3
    push 'r3', r:3
    setvariable
    pop
    setregister r:4
    push 'r4', r:4
    setvariable
    pop
 };
 
 check_equals(r0, "b");
 check_equals(r1, "a");
 check_equals(r2, undefined);

 // Use strictly equals to check that it isn't "null"
 check(r2 === undefined);
 
 check_equals(r3, "stackfirst");
 check_equals(r4, undefined);
#endif

#if MING_VERSION_CODE >= 00040004 
 extras += 6;
 
 // Do the same for enum2
 asm {
    pop
    pop
    push 'stackfirst'
    push 'a'
    getvariable
    enumerate2
    setregister r:0
    push 'r0', r:0
    setvariable
    pop
    setregister r:1
    push 'r1', r:1
    setvariable
    pop
    setregister r:2
    push 'r2', r:2
    setvariable
    pop
    setregister r:3
    push 'r3', r:3
    setvariable
    pop
    setregister r:4
    push 'r4', r:4
    setvariable
    pop
 };
 
 check_equals(r0, "b");
 check_equals(r1, "a");
 check_equals(r2, undefined);

 // Use strictly equals to check that it isn't "null"
 check(r2 === undefined);
 
 check_equals(r3, "stackfirst");
 check_equals(r4, undefined);
#endif

totals(31 + extras);

#else
totals(0);
#endif  // OUTPUT_VERSION > 5


