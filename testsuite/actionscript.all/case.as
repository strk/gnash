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

#include "check.as"

#if OUTPUT_VERSION <= 6

aBcD = 100;
oBj = new Object();
obj.xYZ = 100;

check_equals(ABCD, 100);
check_equals(typeof(OBJ), 'object');
check_equals(OBJ.xyz, 100);

#if OUTPUT_VERSION == 6
// createEmptyMovieClip is supported with swf > 5
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

//
// check _name and _target, they still keep the case
// 
check_equals(mC0._name, "mC0");
check_equals(mC0._target, "/mC0");
check_equals(mC0.mC1._name, "mC1");
check_equals(mC0.mC1._target, "/mC0/mC1");
#endif  // MING_SUPPORTS_ASM
#endif  // OUTPUT_VERSION == 6

#endif  // OUTPUT_VERSION > 5
