// 
//   Copyright (C) 2005, 2006, 2007, 2009, 2010 Free Software
//   Foundation, Inc
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


rcsid="XMLNode.as";
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

check(XMLNode.prototype.hasOwnProperty("getPrefixForNamespace"));
check(XMLNode.prototype.hasOwnProperty("getNamespaceForPrefix"));

check(XMLNode.prototype.hasOwnProperty("firstChild"));
check(XMLNode.prototype.hasOwnProperty("lastChild"));
check(XMLNode.prototype.hasOwnProperty("namespaceURI"));
check(XMLNode.prototype.hasOwnProperty("localName"));
check(XMLNode.prototype.hasOwnProperty("attributes"));
check(XMLNode.prototype.hasOwnProperty("childNodes"));
check(XMLNode.prototype.hasOwnProperty("nextSibling"));
check(XMLNode.prototype.hasOwnProperty("nodeName"));
check(XMLNode.prototype.hasOwnProperty("nodeType"));
check(XMLNode.prototype.hasOwnProperty("nodeValue"));
check(XMLNode.prototype.hasOwnProperty("parentNode"));
check(XMLNode.prototype.hasOwnProperty("prefix"));
check(XMLNode.prototype.hasOwnProperty("previousSibling"));

check_equals(typeof(XMLNode.prototype.attributes), "undefined");

var doc = new XML();

check(doc);
var textnode = doc.createTextNode("text content");
check_equals(typeOf(textnode), 'object');

check_equals(typeof(textnode.attributes), "object");
check_equals(textnode.attributes.toString(), undefined);
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
check_equals(node1.childNodes.length, 4);
check(!node1.childNodes[3] instanceOf XMLNode);

// Now 5. The latest element is an XMLNode, but it does not become
// lastChild
node1.childNodes.push(new XMLNode (1, "an XMLNode"));
check_equals(node1.childNodes.length, 5);
check(node1.childNodes[4] instanceOf XMLNode);
check_equals(node1.lastChild.toString(), "<node3>third text node</node3>");

// childNodes really is just an array: it can be sorted
check_equals(node1.childNodes[0].toString(), "first text node");
check_equals(node1.childNodes[2].toString(), "<node3>third text node</node3>");
check_equals(node1.childNodes[3].toString(), "not a node");
node1.childNodes.sort();
xcheck_equals(node1.childNodes[0].toString(), "<an XMLNode />");
check_equals(node1.childNodes[2].toString(), "<node3>third text node</node3>");
check_equals(node1.childNodes[3].toString(), "first text node");

// It can store anything
node1.childNodes.push(new Date());
check_equals(node1.childNodes.length, 6);
check(node1.childNodes[5] instanceOf Date);
check_equals(node1.childNodes.childNodes[5].hasChildNodes(), undefined);

node1.childNodes.lastChild.appendChild(new XMLNode(1, "datenode"));
check_equals(node1.childNodes.childNodes[5].hasChildNodes(), undefined);

o = {};
o.toString = function() {
    return "o.toString()";
};

check_equals(node1.childNodes.length, 6);
node1.childNodes.push(o);
check_equals(node1.childNodes.length, 7);
check_equals(node1.childNodes[6].toString(), "o.toString()");

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

// Test attributes. It's just a normal object.

// FIXME: This is how it is now.
#if OUTPUT_VERSION > 5
check_equals(node2.attributes, undefined);
#else
check_equals(node2.attributes, undefined);
#endif

check_equals(typeof(node2.attributes), "object");
node2.attributes[3] = "a3";
check_equals(node2.attributes[3], "a3");
check_equals(node2.attributes["3"], "a3");
node2.attributes.a = "aa";
check_equals(node2.attributes.a, "aa");
check_equals(node2.attributes["a"], "aa");
check_equals(node2.toString(), '<node2 a="aa" 3="a3">second text node</node2>');

// Seems not to be overwritable
node2.attributes = 3;
check_equals(node2.toString(), '<node2 a="aa" 3="a3">second text node</node2>');

ASSetPropFlags(XMLNode.prototype, "attributes", 0, 1);
node77 = doc.createElement("tag");
node77.attributes.a1 = "at1";
check_equals(node77.toString(), '<tag a1="at1" />');
node77.attributes = 5;
check_equals(node77.toString(), '<tag a1="at1" />');

// Check order of attributes:

node77.attributes.z = "z";
node77.attributes.x = "x";
node77.attributes.c = "c";
node77.attributes.y = "y";
node77.attributes.f = "f";
node77.attributes[5] = "5";
node77.attributes["$"] = "$";
node77.attributes.x = "x2";
check_equals(node77.toString(), '<tag $="$" 5="5" f="f" y="y" c="c" x="x2" z="z" a1="at1" />');

// Check namespace functions.

// Standard namespace
x = new XML('<tag xmlns="standard" att="u">text</tag>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag");
check_equals(ns.attributes["att"], "u");
check_equals(ns.attributes["xmlns"], "standard");
check_equals(ns.namespaceURI, "standard");
check_equals(ns.getNamespaceForPrefix(), undefined);
check_equals(ns.getNamespaceForPrefix(""), "standard");
check_equals(ns.getPrefixForNamespace("standard"), "");

ns.attributes["xmlns"] = "standard2";
check_equals(ns.namespaceURI, "standard");
check_equals(ns.getNamespaceForPrefix(""), "standard2");

ns = ns.firstChild;
check_equals(ns.nodeName, null);
check_equals(ns.nodeValue, "text");
check_equals(ns.namespaceURI, null);
check_equals(ns.prefix, null);

x = new XML('<tag xmlns:t="standard"></tag>');
ns = x.firstChild;
check_equals(ns.namespaceURI, "");
check_equals(ns.getNamespaceForPrefix(), undefined);
check_equals(ns.getNamespaceForPrefix("t"), "standard");
check_equals(ns.getPrefixForNamespace("standard"), "t");

x = new XML('<tag xmlns:t="nst"><tag2 xmlns="nss"><tag3 xmlns:r="nsr"></tag3></tag2></tag>');

n = x.firstChild;
check_equals(n.nodeName, "tag");
check_equals(n.namespaceURI, "");
check_equals(n.getNamespaceForPrefix("r"), undefined);
check_equals(n.getPrefixForNamespace("nsr"), undefined);
check_equals(n.getNamespaceForPrefix(), undefined);
check_equals(n.getNamespaceForPrefix("t"), "nst");
check_equals(n.getPrefixForNamespace("nst"), "t");

n = n.firstChild;
check_equals(n.nodeName, "tag2");
check_equals(n.namespaceURI, "nss");
check_equals(n.getNamespaceForPrefix(), undefined);
check_equals(n.getNamespaceForPrefix("r"), undefined);
check_equals(n.getPrefixForNamespace("nsr"), undefined);
check_equals(n.getNamespaceForPrefix("t"), "nst");
check_equals(n.getPrefixForNamespace("nst"), "t");

n = n.firstChild;
check_equals(n.nodeName, "tag3");
check_equals(n.namespaceURI, "nss");
check_equals(n.getNamespaceForPrefix(), undefined);
check_equals(n.getNamespaceForPrefix("r"), "nsr");
check_equals(n.getPrefixForNamespace("nsr"), "r");
check_equals(n.getNamespaceForPrefix("t"), "nst");
check_equals(n.getPrefixForNamespace("nst"), "t");

// Poorly formed prefix namespaces: become standard namespaces
x = new XML('<tag xmlns:="nst"><tag2 xmlns="nss"><tag3 xmlns:="nsr"></tag3></tag2></tag>');

n = x.firstChild.firstChild.firstChild;
check_equals(n.nodeName, "tag3");
check_equals(n.namespaceURI, "nsr");
check_equals(n.getPrefixForNamespace("nsr"), "");
check_equals(n.getNamespaceForPrefix(), undefined);
check_equals(n.getPrefixForNamespace("nst"), "");


// Multiple definition of standard namespace (first one counts, second never
// defined).
x = new XML('<tag xmlns="standard" xmlns="standard2"></tag>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag");
check_equals(ns.attributes["xmlns"], "standard");
check_equals(ns.namespaceURI, "standard");
check_equals(ns.getNamespaceForPrefix(""), "standard");

check_equals(ns.getPrefixForNamespace("standard"), "");
check_equals(ns.getPrefixForNamespace("standard2"), undefined);

// Multiple definition of prefix during parsing (first one counts,
// second never defined). Can be changed later using attributes.
x = new XML('<tag xmlns:n1="ns1" xmlns:n1="ns2"></tag>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag");
check_equals(ns.attributes["xmlns"], undefined);
check_equals(ns.attributes["xmlns:n1"], "ns1");
check_equals(ns.namespaceURI, "");
check_equals(ns.getNamespaceForPrefix("n1"), "ns1");
check_equals(ns.getPrefixForNamespace("ns1"), "n1");
check_equals(ns.getPrefixForNamespace("ns2"), undefined);

ns.attributes["xmlns:n1"] = "ns2";
check_equals(ns.attributes["xmlns:n1"], "ns2");
check_equals(ns.getNamespaceForPrefix("n1"), "ns2");
check_equals(ns.getPrefixForNamespace("ns1"), undefined);
check_equals(ns.getPrefixForNamespace("ns2"), "n1");

// Setting via attributes
x = new XML('<tag></tag>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag");
check_equals(ns.attributes["xmlns"], undefined);
check_equals(ns.namespaceURI, "");
ns.attributes["xmlns"] = "nss";
check_equals(ns.attributes["xmlns"], "nss");
check_equals(ns.namespaceURI, "");

/// Prefix, localName
x = new XML('<fr:tag/>');
ns = x.firstChild;
check_equals(ns.nodeName, "fr:tag");
check_equals(ns.localName, "tag");
check_equals(ns.prefix, "fr");

x = new XML('<fr:pr:tag/>');
ns = x.firstChild;
check_equals(ns.nodeName, "fr:pr:tag");
check_equals(ns.localName, "pr:tag");
check_equals(ns.prefix, "fr");

x = new XML('<:fr:tag/>');
ns = x.firstChild;
check_equals(ns.nodeName, ":fr:tag");
check_equals(ns.localName, "fr:tag");
check_equals(ns.prefix, "");

x = new XML('<:tag/>');
ns = x.firstChild;
check_equals(ns.nodeName, ":tag");
check_equals(ns.localName, "tag");
check_equals(ns.prefix, "");

x = new XML('<tag:/>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag:");
check_equals(ns.localName, "tag:");
check_equals(ns.prefix, "");

x = new XML('<tag/>');
ns = x.firstChild;
check_equals(ns.nodeName, "tag");
check_equals(ns.localName, "tag");
check_equals(ns.prefix, "");

// xmlDecl and docTypeDecl don't work for XMLNode
xn = new XMLNode(1, "");
xn.xmlDecl = "hello";
xn.docTypeDecl = "dtd";
check_equals(xn.toString(), "< />");

xn = new XMLNode(2, "");
check_equals(xn.toString(), "");

xn = new XMLNode(3, "");
check_equals(xn.toString(), "");

xn = new XMLNode(4, "");
check_equals(xn.toString(), "");

xn = new XMLNode(5, "");
check_equals(xn.toString(), "");

xn = new XMLNode(6, "");
check_equals(xn.toString(), "");

xn = new XMLNode(7, "");
check_equals(xn.toString(), "");


check_totals(182);
