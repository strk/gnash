// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
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
 *  Test targetPath tags (0x45)
 */

rcsid="$Id: targetPath.as,v 1.3 2007/07/01 10:54:43 bjacques Exp $";

#include "check.as"

check_equals(targetPath(_root), "_level0");

#ifdef MING_SUPPORTS_ASM_TARGETPATH

// targetPath(null)
asm {
push "a"
push null
targetPath
setvariable
};
check_equals(typeof(a), 'undefined');

// targetPath(undefined)
asm {
push "a"
push undefined
targetPath
setvariable
};
check_equals(typeof(a), 'undefined');

// targetPath("str")
asm {
push "a"
push "str"
targetPath
setvariable
};
check_equals(typeof(a), 'undefined');

// targetPath(9)
asm {
push "a"
push 9
targetPath
setvariable
};
check_equals(typeof(a), 'undefined');


// targetPath("_root")
asm {
push "a"
push "_root"
targetPath
setvariable
};
check_equals(typeof(a), 'undefined');

#endif // MING_SUPPORTS_ASM_TARGETPATH
