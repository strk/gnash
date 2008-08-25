// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf


rcsid="$Id: XMLNode.as,v 1.17 2008/03/11 19:31:48 strk Exp $";
#include "check.as"


// Test properties

#if OUTPUT_VERSION < 6
XMLNode.prototype.hasOwnProperty = ASnative(101, 5);
#endif

check(XMLNode.prototype.hasOwnProperty("appendChild"));
check(XMLNode.prototype.hasOwnProperty("cloneNode"));
check(XMLNode.prototype.hasOwnProperty("hasChildNodes"));
check(XMLNode.prototype.hasOwnProperty("insertBefore"));
check(XMLNode.prototype.hasOwnProperty("removeNode"));
check(XMLNode.prototype.hasOwnProperty("toString"));

xcheck(XMLNode.prototype.hasOwnProperty("getPrefixForNamespace"));
xcheck(XMLNode.prototype.hasOwnProperty("getNamespaceForPrefix"));

check(XMLNode.prototype.hasOwnProperty("firstChild"));
check(XMLNode.prototype.hasOwnProperty("lastChild"));
xcheck(XMLNode.prototype.hasOwnProperty("namespaceURI"));
xcheck(XMLNode.prototype.hasOwnProperty("localName"));
check(XMLNode.prototype.hasOwnProperty("attributes"));
check(XMLNode.prototype.hasOwnProperty("childNodes"));
check(XMLNode.prototype.hasOwnProperty("nextSibling"));
check(XMLNode.prototype.hasOwnProperty("nodeName"));
check(XMLNode.prototype.hasOwnProperty("nodeType"));
check(XMLNode.prototype.hasOwnProperty("nodeValue"));
check(XMLNode.prototype.hasOwnProperty("parentNode"));
xcheck(XMLNode.prototype.hasOwnProperty("prefix"));
check(XMLNode.prototype.hasOwnProperty("previousSibling"));

var doc = new XML();

check(doc);
var textnode = doc.createTextNode("text content");
check_equals(typeOf(textnode), 'object');

check(textnode.appendChild);
check(textnode.cloneNode);
check(textnode.hasChildNodes);
check(textnode.insertBefore);
check(textnode.removeNode);
check(textnode.toString);

// functionality of the properties

textnode.nodeName = "foo";
check_equals(textnode.nodeName, "foo");

check_equals (textnode.nodeValue, "text content");
textnode.nodeValue = "bar";
check_equals (textnode.nodeValue, "bar");

// The read only properties. These should all return NULL, because
// there are no children.
check_equals(textnode.hasChildNodes(), false);
check_equals(typeof(textnode.firstChild), 'null');
check_equals(typeof(textnode.lastChild), 'null');
check_equals(typeof(textnode.parentNode), 'null');
check_equals(typeof(textnode.nextSibling), 'null');
check_equals(typeof(textnode.previousSibling), 'null');

//note("Now test the functionality of the methods");


var node1 = doc.createElement("node1");
var node2 = doc.createElement("node2");
var textnode1 = doc.createTextNode("first text node");
var textnode2 = doc.createTextNode("second text node");
check_equals(textnode1.nodeType, 3);
node1.appendChild(textnode1);
node2.appendChild(textnode2);
node1.appendChild(node2);
check_equals(node1.hasChildNodes(), true);

check_equals(node1.firstChild.nodeValue, "first text node");
check_equals(typeof(node1.lastChild.nodeValue), 'null');
check_equals(node2.lastChild.toString(), "second text node"); 

var node3 = doc.createElement("node3");
var textnode3 = doc.createTextNode("third text node");
node3.appendChild(textnode3);
node1.appendChild(node3);


// Childnodes is an array.
check_equals(typeOf(node1.childNodes), "object");
check(node1.childNodes.push);
check(node1.childNodes instanceOf Array);

// node1 has three children (textnode1, node2, node 3)
check_equals(node1.childNodes.length, 3);

// Now it has 4, but the latest element is not an XMLNode
node1.childNodes.push("not a node");
xcheck_equals(node1.childNodes.length, 4);
check(!node1.childNodes[3] instanceOf XMLNode);

// Now 5. The latest element is an XMLNode, but it does not become
// lastChild
node1.childNodes.push(new XMLNode (1, "an XMLNode"));
xcheck_equals(node1.childNodes.length, 5);
xcheck(node1.childNodes[4] instanceOf XMLNode);
check_equals(node1.lastChild.toString(), "<node3>third text node</node3>");

// childNodes really is just an array: it can be sorted
check_equals(node1.childNodes[0].toString(), "first text node");
check_equals(node1.childNodes[2].toString(), "<node3>third text node</node3>");
xcheck_equals(node1.childNodes[3].toString(), "not a node");
node1.childNodes.sort();
xcheck_equals(node1.childNodes[0].toString(), "<an XMLNode />");
check_equals(node1.childNodes[2].toString(), "<node3>third text node</node3>");
xcheck_equals(node1.childNodes[3].toString(), "first text node");

// It can store anything
node1.childNodes.push(new Date());
xcheck_equals(node1.childNodes.length, 6);
xcheck(node1.childNodes[5] instanceOf Date);
check_equals(node1.childNodes.childNodes[5].hasChildNodes(), undefined);

node1.childNodes.lastChild.appendChild(new XMLNode(1, "datenode"));
check_equals(node1.childNodes.childNodes[5].hasChildNodes(), undefined);

o = {};
o.toString = function() {
    return "o.toString()";
};

xcheck_equals(node1.childNodes.length, 6);
node1.childNodes.push(o);
xcheck_equals(node1.childNodes.length, 7);
xcheck_equals(node1.childNodes[6].toString(), "o.toString()");

// The last child is still node3
check(node1.lastChild != node1.childNodes[6]);
check_equals(node1.lastChild.nodeName, "node3");
check_equals(node1.firstChild.toString(), "first text node");

// Only when a new valid element is added does the lastChild change, and the 'fake'
// items disappear.
var node4 = doc.createElement("4node");
node1.appendChild(node4);
check_equals(node1.lastChild.toString(), "<4node />");
check_equals(node1.childNodes.length, 4);
check_equals(node1.childNodes[node1.childNodes.length - 1].toString(), "<4node />")

// And sorting makes no difference to the lastChild, only to the last in the
// array of childNodes.
node1.childNodes.sort();
check_equals(node1.lastChild.toString(), "<4node />");
xcheck_equals(node1.childNodes[node1.childNodes.length - 1].toString(), "first text node")




check_equals(node1.firstChild.nodeValue, "first text node");
check_equals(typeof(node1.lastChild.nodeValue), 'null');

check_equals(typeof(node1.firstChild.nodeName), "null");
check_equals(node1.lastChild.nodeName, "4node");

check_equals(node2.previousSibling.nodeValue, "first text node");

// TODO: test removeNode, insertNode

// for (var aNode = node1.firstChild; node1 != null; aNode = node1.nextSibling) {
//     trace(aNode);
// }


check_totals(78);
