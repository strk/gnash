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

rcsid="$Id: getvariable.as,v 1.5 2007/01/15 00:06:59 strk Exp $";

#include "check.as"

// see check.as
#ifdef MING_SUPPORTS_ASM

_global.globalvar = "gv1";
_global.globalobj = { m1: 1 };

//---------------------------------------------------------------------
// Check access to root variable (simplest case)
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
// Check access to root variable trough '_root.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.1;
asm {
        push 'checkpoint'
	push '_root.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.1);

//---------------------------------------------------------------------
// Check access to root variable trough '_level0.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.2;
asm {
        push 'checkpoint'
	push '_level0.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.2);

//---------------------------------------------------------------------
// Check access to root variable trough 'this.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.3;
asm {
        push 'checkpoint'
	push 'this.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.3);

//---------------------------------------------------------------------
// Check access to root variable trough 'this._root._level0.varname'
// (insane)
//---------------------------------------------------------------------

var variable_in_root = 5.4;
asm {
        push 'checkpoint'
	push 'this._root._level0.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, 5.4);

//---------------------------------------------------------------------
// Check access to root variable trough 'THIS.varname'
//---------------------------------------------------------------------

var variable_in_root = 5.5;
asm {
        push 'checkpoint'
	push 'THIS.variable_in_root'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 6
check_equals(checkpoint, undefined);
#else
check_equals(checkpoint, 5.5);
#endif


//---------------------------------------------------------------------
// Check '../:variable_in_root' access 
// (expected to fail, but I'm not sure why)
//---------------------------------------------------------------------

var variable_in_root = 6;
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
// Check '/invalidname2' access 
// (expected to fail)
//---------------------------------------------------------------------

asm {
	push '/invalidname2'
	push '18'
	setvariable
        push 'checkpoint'
	push '/invalidname'
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
check_equals(objmemb, 3);

//---------------------------------------------------------------------
// Check 'obj._root.variable_in_root' access 
// (expect to fail)
//---------------------------------------------------------------------

var variable_in_root = 10.0;
var o = { memb:4 };
asm {
        push 'checkpoint'
	push 'o._root.variable_in_root'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj.this.memb' access 
// (expect to fail)
//---------------------------------------------------------------------

var o = { memb:10.1 };
asm {
        push 'checkpoint'
	push 'o.this.memb'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'obj1.obj2.member' access 
//---------------------------------------------------------------------

var o2 = { memb:4 };
var o = { obj2:o2 };
asm {
        push 'checkpoint'
	push 'o.obj2.memb'
	getvariable
        setvariable
};
check_equals(checkpoint, 4);

//---------------------------------------------------------------------
// Check 'obj1.globalvar'
// (expect to fail)
//---------------------------------------------------------------------

var o3 = { memb:4 };
asm {
        push 'checkpoint'
	push 'o3.globalvar'
	getvariable
        setvariable
};
check_equals(checkpoint, undefined);

//---------------------------------------------------------------------
// Check 'globalobj.m1'
//---------------------------------------------------------------------

var o3 = { memb:4 };
asm {
        push 'checkpoint'
	push 'globalobj.m1'
	getvariable
        setvariable
};
#if OUTPUT_VERSION > 5
check_equals(checkpoint, 1);
#else // OUTPUT_VERSION < 6
// _global was added in SWF6 !
xcheck_equals(checkpoint, undefined);
#endif

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
