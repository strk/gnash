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

//

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: XMLNode.as,v 1.6 2006/11/27 09:17:28 strk Exp $";

#include "dejagnu.as"

check(XMLNode);
var textnode = new XMLNode(3, "text content");

check(textnode);

// test the XMLNode constuctor
//dejagnu(node, "XMLNode::XMLNode()");

//note("Test the existance of all the methods");

check(textnode.appendChild);
check(textnode.cloneNode);
check(textnode.hasChildNodes);
check(textnode.insertBefore);
check(textnode.removeNode);
check(textnode.toString);

//note("Now test the functionality of the properties");

textnode.nodeName = "foo";
check_equals(textnode.nodeName, "foo");

xcheck_equals (textnode.nodeValue, "text content");
textnode.nodeValue = "bar";
check_equals (textnode.nodeValue, "bar");

// The read only properties. These should all return NULL, because
// there are no children.
check_equals(textnode.hasChildNodes(), false);
xcheck_equals(textnode.firstChild, undefined);
xcheck_equals(textnode.lastChild, undefined);
xcheck_equals(textnode.parentNode, undefined);
xcheck_equals(textnode.nextSibling, undefined);
xcheck_equals(textnode.previousSibling, undefined);

//note("Now test the functionality of the methods");

var childnode1 = new XMLNode(3, "first child");
xcheck_equals(childnode1.nodeType, 3);
textnode.appendChild(childnode1);

check_equals(textnode.hasChildNodes(), true);
check_equals(textnode.firstChild, childnode1);
check_equals(textnode.lastChild, childnode1);
xcheck_equals(childnode1.nextSibling, undefined);
xcheck_equals(childnode1.previousSibling, undefined);

var nextnode = new XMLNode(3, "second child");
xcheck_equals(nextnode.nodeType, 3);
textnode.appendChild(nextnode);

check_equals(textnode.hasChildNodes(), true);
check_equals(textnode.firstChild, childnode1);
check_equals(textnode.lastChild, nextnode);
xcheck_equals(childnode1.nextSibling, nextnode);
xcheck_equals(childnode1.previousSibling, undefined);
xcheck_equals(nextnode.previousSibling, childnode1);

//var out = textnode.toString();
//trace(out);

// TODO: test removeNode, insertNode

totals();
