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

rcsid="$Id: case.as,v 1.7 2007/08/23 09:55:55 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION <= 6
aBcD = 100;
oBj = new Object();
obj.xYZ = 100;

check_equals(ABCD, 100);
check_equals(typeof(OBJ), 'object');
check_equals(OBJ.xyz, 100);
#else
//swf7 and above are case sensitive
check_equals(ABCD, undefined);
check_equals(OBJ, undefined);
#endif 

#if OUTPUT_VERSION == 6
// 
// create _root.mc0 and _root.mc0.mc1
// 
_ROOT.createEmptyMovieClip("mC0", 3);
check_equals(typeof(mc0), 'movieclip');
mC0.createEmptyMovieClip("mC1", 3);
check_equals(typeof(mc0.mc1), 'movieclip');

#ifdef MING_SUPPORTS_ASM
asm{
     push "/_ROOT/MC0/"
     push 0.0
     push 100
     setproperty
};
// check setproperty
xcheck_equals(mC0._X, 100);
xcheck_equals(mC0._x, 100);

//
// check _name and _target, they still keep the case
// 
check_equals(mC0._name, "mC0");
check_equals(mC0._target, "/mC0");
check_equals(mC0.mC1._name, "mC1");
check_equals(mC0.mC1._target, "/mC0/mC1");
#endif  // MING_SUPPORTS_ASM
#endif  // OUTPUT_VERSION == 6

#if OUTPUT_VERSION >= 6
//
// creat two movieclips named by same letters but different cases
//
mcRef = new Array(10);
i = 0;
MovieClip.prototype.onConstruct = function ()
{
	mcRef[i++] = this;
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
check_equals(mcRef[1].getDepth(), 7);
check_equals(mcRef[0]._name, "clip");
check_equals(mcRef[1]._name, "CLIP");
check_equals(mcRef[0]._target, "/clip");
check_equals(mcRef[1]._target, "/CLIP");
#endif

#if OUTPUT_VERSION <= 6
//case insensitive, so they reference the same movieclip.
//Or both are undefined with OUTPUT_VERSION <= 5
check(clip == CLIP); 
#else 
//case sensitive, so they are different
xcheck(clip != CLIP);
check_equals(clip.getDepth(), 6);
xcheck_equals(CLIP.getDepth(), 7);
#endif 

//_root.totals(); // this is done by dejagnu_so_fini.as
