// 
//   Copyright (C) 2007 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
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
//
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
// Test case for TextField ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: TextField.as,v 1.3 2007/07/13 14:50:07 strk Exp $";

#include "check.as"

#if OUTPUT_VERSION > 5

check_equals(typeof(TextField), 'function');
check_equals(typeof(TextField.prototype), 'object');
check_equals(typeof(TextField.prototype.setTextFormat), 'function');
check_equals(typeof(TextField.prototype.getTextFormat), 'function');
check_equals(typeof(TextField.prototype.setNewTextFormat), 'function');
check_equals(typeof(TextField.prototype.getNewTextFormat), 'function');
check_equals(typeof(TextField.prototype.addListener), 'function');
check_equals(typeof(TextField.prototype.removeListener), 'function');
check_equals(typeof(TextField.prototype.getDepth), 'function');
check_equals(typeof(TextField.prototype.removeTextField), 'function');
check_equals(typeof(TextField.prototype.replaceSel), 'function');
check(!TextField.prototype.hasOwnProperty('background'));

// this is a static method
check_equals(typeof(TextField.getFontList), 'function');

check_equals(typeof(TextField.prototype.getFontList), 'undefined');

#if OUTPUT_VERSION > 6
check_equals(typeof(TextField.prototype.replaceText), 'function');
#else
check_equals(typeof(TextField.prototype.replaceText), 'undefined');
#endif

tfObj = new TextField();
check_equals(typeof(tfObj), 'object');
check(tfObj instanceof TextField);

check_equals(typeof(tfObj.setTextFormat), 'function');
check_equals(typeof(tfObj.getTextFormat), 'function');
check_equals(typeof(tfObj.setNewTextFormat), 'function');
check_equals(typeof(tfObj.getNewTextFormat), 'function');
check_equals(typeof(tfObj.addListener), 'function');
check_equals(typeof(tfObj.removeListener), 'function');
check_equals(typeof(tfObj.getDepth), 'function');
check_equals(typeof(tfObj.removeTextField), 'function');
check_equals(typeof(tfObj.replaceSel), 'function');
// this is a static method, it's available as TextField.getFontList
check_equals(typeof(tfObj.getFontList), 'undefined');


#endif // OUTPUT_VERSION > 5
