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

rcsid="$Id: XMLNode.as,v 1.5 2006/11/16 23:26:53 strk Exp $";

#include "dejagnu.as"

var node = new XMLNode;

// test the XMLNode constuctor
//dejagnu(node, "XMLNode::XMLNode()");

note("Test the existance of all the methods");

// test the XMLNode::appendchild method
if (node.appendChild) {
    pass("XMLNode::appendChild() exists");
} else {
    fail("XMLNode::appendChild() doesn't exist");
}

// test the XMLNode::clonenode method
if (node.cloneNode) {
	pass("XMLNode::cloneNode() exists");
} else {
	fail("XMLNode::cloneNode() doesn't exist");
}
// test the XMLNode::haschildnodes method
if (node.hasChildNodes) {
	pass("XMLNode::hasChildNodes() exists");
} else {
	fail("XMLNode::hasChildNodes() doesn't exist");
}
// test the XMLNode::insertbefore method
if (node.insertBefore) {
	pass("XMLNode::insertBefore() exists");
} else {
	fail("XMLNode::insertBefore() doesn't exist");
}
// test the XMLNode::removenode method
if (node.removeNode) {
	pass("XMLNode::removeNode() exists");
} else {
	fail("XMLNode::removeNode() doesn't exist");
}
// test the XMLNode::tostring method
if (node.toString) {
	pass("XMLNode::toString() exists");
} else {
	fail("XMLNode::toString() doesn't exist");
}

note("\nTest the existance of all the properties");
// Check the properties
// test the XMLNode::nodeName property
if (node.nodeName) {
	pass("XMLNode::nodeName exists");
} else {
	fail("XMLNode::nodeName doesn't exist");
}
if (node.nodeType) {
	pass("XMLNode::nodeType exists");
} else {
	fail("XMLNode::nodeType doesn't exist");
}
if (node.attributes) {
	pass("XMLNode::attributes exists");
} else {
	fail("XMLNode::attributes doesn't exist");
}
if (node.childNodes) {
	pass("XMLNode::childNodes exists");
} else {
	fail("XMLNode::childNodes doesn't exist");
}
if (node.firstChild) {
	pass("XMLNode::firstChild exists");
} else {
	fail("XMLNode::firstChild doesn't exist");
}
if (node.lastChild) {
	pass("XMLNode::lastChild exists");
} else {
	fail("XMLNode::lastChild doesn't exist");
}
if (node.nextSibling) {
	pass("XMLNode::nextSibling exists");
} else {
	fail("XMLNode::nextSibling doesn't exist");
}
if (node.parentNode) {
	pass("XMLNode::parentNode exists");
} else {
	fail("XMLNode::parentNode doesn't exist");
}
if (node.previousSibling) {
	pass("XMLNode::previousSibling exists");
} else {
	fail("XMLNode::previousSibling doesn't exist");
}

note("Now test the functionality of the properties");

node.nodeName = "foo";

if (node.nodeName == "foo") {
    pass("XMLNode::nodeName works");
} else {
    fail("XMLNode::nodeName doesn't work");
}

node.nodeValue = "bar";
if (node.nodeValue == "bar") {
    pass("XMLNode::nodeValue works");
} else {
    fail("XMLNode::nodeValue doesn't work");
}

// The read only properties. These should all return NULL, because
// there are no children.
if (node.firstChild) {
    pass("XMLNode::firstChild with no children works");
} else {
    fail("XMLNode::firstChild with no children doesn't works");
}

if (node.lastChild) {
    pass("XMLNode::lastChild with no children works");
} else {
    fail("XMLNode::lastChild with no children doesn't works");
}

if (node.parentNode) {
    pass("XMLNode::parentNode with no children works");
} else {
    fail("XMLNode::parentNode with no children doesn't works");
}

if (node.nextSibling) {
    pass("XMLNode::nextSibling with no children works");
} else {
    fail("XMLNode::nextSibling with no children doesn't works");
}

if (node.previousSibling) {
    pass("XMLNode::PreviousSibling with no children works");
} else {
    fail("XMLNode::PreviousSibling with no children doesn't works");
}

note("Now test the functionality of the methods");

var childnode = new XMLNode;
childnode.nodeName = "fu";
childnode.nodeValue = "bore";

var before = node.hasChildNodes();
node.appendChild(childnode);
var after = node.hasChildNodes();

//trace(before);
//trace(after);

if ((before == false) && (after == true)) {
    pass("XMLNode::appendChild() works");
} else {
    fail("XMLNode::appendChild() doesn't work");
}

// Because the node has no type, we don't have a legit value yet.
if (childnode.nodeType == "") {
    pass("XMLNode::nodeType property works");
} else {
    fail("XMLNode::nodeType property doesn't work");
}

// The read only properties. These should all return a valid XMLNode
// object because there is one child.
if (node.firstChild.nodeName == "fu") {
    pass("XMLNode::firstChild with child works");
} else {
    fail("XMLNode::firstChild with child doesn't works");
}

if (node.lastChild.nodeName  == "fu") {
    pass("XMLNode::lastChild with child works");
} else {
    fail("XMLNode::lastChild with children doesn't works");
}

// These should still return zero because there is only one child
if (node.nextSibling) {
    pass("XMLNode::nextSibling with child works");
} else {
    fail("XMLNode::nextSibling with child doesn't works");
}

if (node.previousSibling) {
    pass("XMLNode::PreviousSibling with child work");
} else {
    fail("XMLNode::PreviousSibling with child doesn't work");
}

var nextnode = new XMLNode;
nextnode.nodeName = "fur";
nextnode.nodeValue = "bare";

node.appendChild(nextnode);

// The read only properties. These should all return a valid XMLNode
// object because there is more than one child.
if (node.firstChild.nodeName == "fu") {
    pass("XMLNode::firstChild with children works");
} else {
    fail("XMLNode::firstChild with children doesn't works");
}

if (node.lastChild.nodeName  == "fur") {
    pass("XMLNode::lastChild with children works");
} else {
    fail("XMLNode::lastChild with children doesn't works");
}

// These should still return a valid pointer now
if (node.nextSibling) {
    pass("XMLNode::nextSibling with child works");
} else {
    fail("XMLNode::nextSibling with child doesn't works");
}

trace(node.previousSibling.nodeName);

if (node.previousSibling.nodeName == "fu") {
    xpass("XMLNode::PreviousSibling with child works");
} else {
    xfail("XMLNode::PreviousSibling with child doesn't works");
}

var out = node.toString();
trace(out);

totals();
