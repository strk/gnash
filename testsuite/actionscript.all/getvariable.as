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

rcsid="$Id: getvariable.as,v 1.2 2007/01/11 23:09:30 strk Exp $";

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
// Check '../:var' access 
// (expected to fail)
//---------------------------------------------------------------------

var variable_in_root = 5;
asm {
        push 'checkpoint'
	push '../:variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check '../invalidname' access 
// (expected to fail)
//---------------------------------------------------------------------

asm {
	push '../invalidname'
	push '8'
	setvariable
        push 'checkpoint'
	push '../invalidname'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj.member' access 
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
// Check 'obj/:member' access 
//-----------------------------------------------------------------------

var obj = { memb:3 };
asm {
        push 'objmemb'
	push 'obj/:memb'
	getvariable
        setvariable
};
check_equals(objmemb, 3);

//-----------------------------------------------------------------------
// Check 'invalid/name' access
// ('invalid/name' used as a variable name)
//-----------------------------------------------------------------------

asm {
	push 'invalid/name'
	push '7'
	setvariable
        push 'checkpoint'
	push 'invalid/name'
	getvariable
        setvariable
};
check_equals(checkpoint, 7);

//-----------------------------------------------------------------------
// TODO: try use of 'with' stack
//-----------------------------------------------------------------------

#endif // MING_SUPPORT_ASM
