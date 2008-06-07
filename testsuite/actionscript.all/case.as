// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modchecky
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
 *  Test case sensitivity 
 */


rcsid="$Id: case.as,v 1.21 2008/06/07 12:11:34 bwy Exp $";
#include "check.as"

#if OUTPUT_VERSION <= 6 // {
aBcD = 100;
oBj = new Object();
obj.xYZ = 100;

check_equals(ABCD, 100);
check_equals(typeof(OBJ), 'object');
check_equals(OBJ.xyz, 100);
#else // OUTPUT_VERSION > 6 }{
//swf7 and above are case sensitive
check_equals(ABCD, undefined);
check_equals(OBJ, undefined);
#endif  // OUTPUT_VERSION > 6 }

#if OUTPUT_VERSION <= 6
check_equals(_root, _ROOT);
check_equals(_level0, _LEVEL0);
#else // OUTPUT_VERSION >= 7
check(_root != _ROOT);
check(_level0 != _LEVEL0);
#endif // OUTPUT_VERSION >= 7

#if OUTPUT_VERSION == 6 // { 


// 
// create _root.mc0 and _root.mc0.mc1
// 
_ROOT.createEmptyMovieClip("mC0", 3);
check_equals(typeof(mc0), 'movieclip');
mC0.createEmptyMovieClip("mC1", 3);
check_equals(typeof(mc0.mc1), 'movieclip');

#ifdef MING_SUPPORTS_ASM // {
asm{
     push "/_ROOT/MC0/"
     push 0.0
     push 100
     setproperty
};
// check setproperty
check_equals(mC0._X, 100);
check_equals(mC0._x, 100);

//
// check _name and _target, they still keep the case
// 
check_equals(mC0._name, "mC0");
check_equals(mC0._target, "/mC0");
check_equals(mC0.mC1._name, "mC1");
check_equals(mC0.mC1._target, "/mC0/mC1");
#endif  // MING_SUPPORTS_ASM }
#endif  // OUTPUT_VERSION == 6  }

#if OUTPUT_VERSION >= 6 // {

//
// create two movieclips named by same letters but different cases
//
mcRef = new Array(10);
i = 0;
MovieClip.prototype.onConstruct = function ()
{
  mcRef[i++] = this;
  note("Constructed "+this+" in depth "+this.getDepth()+" and assigned to mcRef["+(i-1)+"]");
};

_root.createEmptyMovieClip("clip", 6); //this will invoke the onConstruct listener
_root.createEmptyMovieClip("CLIP", 7); //this will invoke the onConstruct listener

// support visual checks
with(mcRef[0])
{
  beginFill(0xff0000, 100);
  moveTo(100, 100);
  lineTo(200,100);
  lineTo(200,200);
  lineTo(100,200);
  lineTo(100,100);
  endFill();
  _alpha = 20;
}

// support visual checks
with(mcRef[1])
{
  beginFill(0x00ff00, 100);
  moveTo(100, 100);
  lineTo(200,100);
  lineTo(200,200);
  lineTo(100,200);
  lineTo(100,100);
  endFill();
  _alpha = 20;
}
mcRef[1]._x += 100;

check(mcRef[0]!= mcRef[1]);
check_equals(mcRef[0].getDepth(), 6);
check_equals(mcRef[0]._name, "clip");
check_equals(mcRef[0]._target, "/clip");

#if OUTPUT_VERSION > 6
check_equals(mcRef[1].getDepth(), 7); 
check_equals(mcRef[1]._name, "CLIP"); 
check_equals(mcRef[1]._target, "/CLIP"); 
#else // OUTPUT_VERSION <= 6
// Gnash used to fail these due to "soft references"
// Basically, a MOVIECLIP as_value stores the clip
// target, but in SWF<7 the target is insensitive
// so /clip and /CLIP both resolve to the *same*
// character.
//
check_equals(mcRef[1].getDepth(), 7);
check_equals(mcRef[1]._name, "CLIP"); 
check_equals(mcRef[1]._target, "/CLIP"); 
#endif // OUTPUT_VERSION <= 6


#if OUTPUT_VERSION <= 6
//case insensitive, so they reference the same movieclip.
//Or both are undefined with OUTPUT_VERSION <= 5
check(clip == CLIP); 
#else // OUTPUT_VERSION >= 7
//case sensitive, so they are different
check(clip != CLIP);
check_equals(clip.getDepth(), 6);
check_equals(CLIP.getDepth(), 7);
#endif  // OUTPUT_VERSION >= 7

_root.createEmptyMovieClip("CLIP2", 8); //this will invoke the onConstruct listener
_root.createEmptyMovieClip("clip2", 9); //this will invoke the onConstruct listener

check_equals(clip.getDepth(), 6);
#if OUTPUT_VERSION < 7
check_equals(CLIP.getDepth(), 6);
check_equals(CLIP2.getDepth(), 8);
check_equals(clip2.getDepth(), 8);
#else // OUTPUT_VERSION >= 7
check_equals(CLIP.getDepth(), 7);
check_equals(clip2.getDepth(), 9);
check_equals(CLIP2.getDepth(), 8);
#endif

#endif // OUTPUT_VERSION >= 6 }

//
// Test function args
//
func = function (xYz)
{
  check_equals(xYz, 100);
#if OUTPUT_VERSION < 7
  check_equals(xyz, 100);
#endif 
  this.testVar = xYz;
  check_equals(this.testVar, 100);
#if OUTPUT_VERSION < 7
  check_equals(this.testvar, 100);
#endif 
};
// call the function above,
// trigger tests in it.
func(100);

#if OUTPUT_VERSION > 5
  mcRef = _root.createEmptyMovieClip("mc_XYZ", 3);
  check_equals(typeof(_root['mc_XYZ']), 'movieclip');
  check_equals(typeof(_root['mcRef']), 'movieclip');
  check_equals(typeof(mcRef['gotoAndStop']), 'function');
#endif 


delete obj;
propRecorder = new Array();
obj = { A: 1, b: 2 };
for(var prop in obj)
{
   propRecorder.push(prop);
}
propRecorder.sort(); // case sensitive sort
check_equals(propRecorder.length, 2);
#if OUTPUT_VERSION < 7
xcheck_equals(propRecorder[0], 'A')
#else
check_equals(propRecorder[0], 'A')
#endif
check_equals(propRecorder[1], 'b')

propRecorder = new Array();
obj.a = 3;
for(var prop in obj)
{
   propRecorder.push(prop);
}
propRecorder.sort(); //case sensitive sort
#if OUTPUT_VERSION < 7
    check_equals(propRecorder.length, 2);
    xcheck_equals(propRecorder[0], 'A')
    check_equals(propRecorder[1], 'b')
#else
    check_equals(propRecorder.length, 3);
    check_equals(propRecorder[0], 'A')
    check_equals(propRecorder[1], 'a')
    check_equals(propRecorder[2], 'b')
#endif

propRecorder = new Array();
obj.B = 4;
for(var prop in obj)
{
   propRecorder.push(prop);
}
propRecorder.sort(); //case sensitive sort
#if OUTPUT_VERSION < 7
    check_equals(propRecorder.length, 2);
    xcheck_equals(propRecorder[0], 'A')
    check_equals(propRecorder[1], 'b')
#else
    check_equals(propRecorder.length, 4);
    check_equals(propRecorder[0], 'A') 
    check_equals(propRecorder[1], 'B') 
    check_equals(propRecorder[2], 'a')
    check_equals(propRecorder[3], 'b')
#endif

propRecorder = new Array();
delete obj.a;
for(var prop in obj)
{
   propRecorder.push(prop);
}
propRecorder.sort(); //case sensitive sort
#if OUTPUT_VERSION < 7
    check_equals(propRecorder.length, 1);
    check_equals(propRecorder[0], 'b')
#else
    check_equals(propRecorder.length, 3);
    check_equals(propRecorder[0], 'A')
    check_equals(propRecorder[1], 'B')
    check_equals(propRecorder[2], 'b')
#endif 

// Now create a new object, with lowercase A and uppercase B
// Gnash will fail as long as it uses a single string_table
// for the whole objects set
obj = { a: 1, B: 2 };
#if OUTPUT_VERSION > 5
 check(obj.hasOwnProperty('a'));
 #if OUTPUT_VERSION == 6
  check(obj.hasOwnProperty('A')); 
 #else
  check(!obj.hasOwnProperty('A')); 
 #endif
#endif
propRecorder = new Array();
for(var prop in obj)
{
   propRecorder.push(prop);
}
propRecorder.sort();
check_equals(propRecorder.length, 2);
#if OUTPUT_VERSION < 7
 // gnash fails here because 'B' will point to a previously used 'b'
 // and 'a' will point to a previously used 'A'
 // in the global string_table
 xcheck_equals(propRecorder[0], 'B');
 xcheck_equals(propRecorder[1], 'a');
#else
 // The problem above is a non-issue in SWF7 or higher, as 'b' and
 // 'B' will be separate entries in the string_table
 check_equals(propRecorder[0], 'B');
 check_equals(propRecorder[1], 'a');
#endif


#if OUTPUT_VERSION > 5
// Check that properties are not overridden by
// variable names.
_global.hasOwnProperty = ASnative(101, 5);

check(_global.hasOwnProperty("Date"));
# if OUTPUT_VERSION == 6
check(_global.hasOwnProperty("date"));
# endif
Color = 5;
color = 8;

check(_global.hasOwnProperty("Color"));
# if OUTPUT_VERSION == 6
check(_global.hasOwnProperty("color"));
# endif

#endif


#if OUTPUT_VERSION <= 5
 check_totals(23);
#endif
#if OUTPUT_VERSION == 6
 check_totals(52);
#endif
#if OUTPUT_VERSION >= 7
 check_totals(46);
#endif
