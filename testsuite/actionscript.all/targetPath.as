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
 *  Test TARGETPATH tags (0x45)
 */


rcsid="$Id: targetPath.as,v 1.15 2008/03/11 19:31:49 strk Exp $";
#include "check.as"

#if MING_VERSION_CODE > 00040005
# define TARGETPATH targetpath
#else
# define TARGETPATH targetPath
#endif

// The AS version is always targetPath, just
// the ASM label changes in case before 0.4.0.beta6
check_equals(targetPath(_root), "_level0");

#ifdef MING_SUPPORTS_ASM_TARGETPATH

// TARGETPATH(null)
asm {
push "a"
push null
TARGETPATH
setvariable
};
check_equals(typeof(a), 'undefined');

// TARGETPATH(undefined)
asm {
push "a"
push undefined
TARGETPATH
setvariable
};
check_equals(typeof(a), 'undefined');

// TARGETPATH("str")
asm {
push "a"
push "str"
TARGETPATH
setvariable
};
check_equals(typeof(a), 'undefined');

// TARGETPATH(9)
asm {
push "a"
push 9
TARGETPATH
setvariable
};
check_equals(typeof(a), 'undefined');


// TARGETPATH("_root")
asm {
push "a"
push "_root"
TARGETPATH
setvariable
};
check_equals(typeof(a), 'undefined');

#endif // MING_SUPPORTS_ASM_TARGETPATH

#if OUTPUT_VERSION > 5

mc1 = createEmptyMovieClip('mc', 1);
mc2 = mc1.createEmptyMovieClip('mc', 1);

// hack to make check_equals work even with modified target
_global.fail_check = fail_check;
_global.xfail_check = xfail_check;
_global.pass_check = pass_check;
_global.xpass_check = xpass_check;

setTarget(null);
// getVaiable(_target) would ascend to other target
check_equals(_target, '/');
asm{          
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
};
//check current target is undefined
xcheck_equals(checkpoint, undefined);
setTarget("");

setTarget('...:mc1');
// getVaiable(_target) would ascend to other target
check_equals(_target, '/');
asm{          
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
};
//check current target is undefined
xcheck_equals(checkpoint, undefined);
setTarget("");

setTarget('...:mc');
// getVaiable(_target) would ascend to other target
check_equals(_target, '/');
asm{          
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
};
//check current target is undefined
xcheck_equals(checkpoint, undefined);
setTarget("");

asm{          
    push 'checkpoint'         
    push ''
    push 11         
    getproperty  //_target         
    setvariable             
};
check_equals(checkpoint, '/');

setTarget('mc');
check_equals(_target, '/mc');
setTarget("/");
check_equals(_target, '/');

setTarget('/mc/mc');
check_equals(_target, '/mc/mc');
setTarget("/");
check_equals(_target, '/');

#endif

#if OUTPUT_VERSION < 6
 check_totals(6);
#else
 check_totals(17);
#endif
