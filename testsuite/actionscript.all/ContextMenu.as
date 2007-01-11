// 
//   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License

// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// Test case for ContextMenu ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: ContextMenu.as,v 1.8 2007/01/11 12:15:03 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION < 7

// there was no ContextMenu before SWF7, still
// it seems the player allows instantiating one (?)
xcheck_equals(typeof(ContextMenu), 'function');
var contextmenuObj = new ContextMenu;
xcheck_equals (typeof(contextmenuObj), 'object');

#else // OUTPUT_VERSION >= 7

// there was no ContextMenu before SWF7
check_equals(typeof(ContextMenu), 'function');

var contextmenuObj = new ContextMenu;

// test the ContextMenu constuctor
check_equals (typeof(contextmenuObj), 'object');

// test the ContextMenu::copy method
check_equals (typeof(contextmenuObj.copy), 'function');
// test the ContextMenu::hideBuiltinItems method
check_equals (typeof(contextmenuObj.hideBuiltInItems), 'function');

#endif
