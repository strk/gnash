// 
//   Copyright (C) 2008 Free Software Foundation, Inc.
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
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextFormat ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: TextFormat.as,v 1.1 2008/04/09 20:34:54 strk Exp $";

#include "check.as"

Object.prototype.hasOwnProperty = ASnative(101, 5);

check_equals(typeof(TextFormat), 'function');
check_equals(typeof(TextFormat.prototype), 'object');
tfObj = new TextFormat();
check_equals(typeof(tfObj), 'object');
check(tfObj instanceof TextFormat);

// The members below would not exist before
// the construction of first TextFormat object
check(TextFormat.prototype.hasOwnProperty('display'));
check(TextFormat.prototype.hasOwnProperty('bullet'));
check(TextFormat.prototype.hasOwnProperty('tabStops'));
check(TextFormat.prototype.hasOwnProperty('blockIndent'));
check(TextFormat.prototype.hasOwnProperty('leading'));
check(TextFormat.prototype.hasOwnProperty('indent'));
check(TextFormat.prototype.hasOwnProperty('rightMargin'));
check(TextFormat.prototype.hasOwnProperty('leftMargin'));
check(TextFormat.prototype.hasOwnProperty('align'));
check(TextFormat.prototype.hasOwnProperty('underline'));
check(TextFormat.prototype.hasOwnProperty('italic'));
check(TextFormat.prototype.hasOwnProperty('bold'));
check(TextFormat.prototype.hasOwnProperty('target'));
check(TextFormat.prototype.hasOwnProperty('url'));
check(TextFormat.prototype.hasOwnProperty('color'));
check(TextFormat.prototype.hasOwnProperty('size'));
check(TextFormat.prototype.hasOwnProperty('font'));
check(!TextFormat.prototype.hasOwnProperty('getTextExtent'));
check(tfObj.hasOwnProperty('getTextExtent'));


check_totals(23);
