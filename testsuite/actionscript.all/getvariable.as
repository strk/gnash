// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for getvariable Action tag class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: getvariable.as,v 1.1 2007/01/11 22:53:59 strk Exp $";

#include "check.as"

// see check.as
#ifdef MING_SUPPORTS_ASM

//---------------------------------------------------------------------
// Check 'var' access 
//---------------------------------------------------------------------

var variable_in_root = 5;
asm {
        push 'checkpoint'
	push 'variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5);

//---------------------------------------------------------------------
// Check '../var' access 
// (expected to fail)
//---------------------------------------------------------------------

var variable_in_root = 5;
asm {
        push 'checkpoint'
	push '../variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj.member' access using GetVariable (rather then getMember)
//---------------------------------------------------------------------

var obj = { memb:3 };
asm {
        push 'objmemb'
	push 'obj.memb'
	getvariable
        setvariable
};
xcheck_equals(objmemb, 3);

//-----------------------------------------------------------------------
// Check 'obj/member' access using GetVariable (rather then getMember)
// (expected to fail)
//-----------------------------------------------------------------------

var obj = { memb:3 };
asm {
        push 'objmemb'
	push 'obj/memb'
	getvariable
        setvariable
};
check_equals(objmemb, undefined);

//-----------------------------------------------------------------------
// TODO: try use of 'with' stack
//-----------------------------------------------------------------------

#endif // MING_SUPPORT_ASM
