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

var tmp = new XML;

#include "dejagnu.as"

// test the XML constuctor
if (tmp) {
	pass("XML::XML() constructor");
} else {
	fail("XML::XML()");		
}

// test the XML::addrequestheader method
if (tmp.addRequestHeader) {
	pass("XML::addRequestHeader() exists");
} else {
	fail("XML::addRequestHeader() doesn't exist");
}
// test the XML::appendchild method
if (tmp.appendChild) {
	pass("XML::appendChild() exists");
} else {
	fail("XML::appendChild() doesn't exist");
}
// test the XML::clonenode method
if (tmp.cloneNode) {
	pass("XML::cloneNode() exists");
} else {
	fail("XML::cloneNode() doesn't exist");
}
// test the XML::createelement method
if (tmp.createElement) {
	pass("XML::createElement() exists");
} else {
	fail("XML::createElement() doesn't exist");
}
// test the XML::createtextnode method
if (tmp.createTextNode) {
	pass("XML::createTextNode() exists");
} else {
	fail("XML::createTextNode() doesn't exist");
}
// test the XML::getbytesloaded method
if (tmp.getBytesLoaded) {
	pass("XML::getBytesLoaded() exists");
} else {
	fail("XML::getBytesLoaded() doesn't exist");
}
// test the XML::getbytestotal method
if (tmp.getBytesTotal) {
	pass("XML::getBytesTotal() exists");
} else {
	fail("XML::getBytesTotal() doesn't exist");
}
// test the XML::haschildnodes method
if (tmp.hasChildNodes) {
	pass("XML::hasChildNodes() exists");
} else {
	fail("XML::hasChildNodes() doesn't exist");
}
// test the XML::insertbefore method
if (tmp.insertBefore) {
	pass("XML::insertBefore() exists");
} else {
	fail("XML::insertBefore() doesn't exist");
}
// test the XML::load method
if (tmp.load) {
	pass("XML::load() exists");
} else {
	fail("XML::load() doesn't exist");
}
// test the XML::loaded method
if (tmp.loaded) {
	pass("XML::loaded() exists");
} else {
	fail("XML::loaded() doesn't exist");
}
// test the XML::parse method
if (tmp.parseXML) {
	pass("XML::parseXML() exists");
} else {
	fail("XML::parseXML() doesn't exist");
}
// test the XML::removenode method
if (tmp.removeNode) {
	pass("XML::removeNode() exists");
} else {
	fail("XML::removeNode() doesn't exist");
}
// test the XML::send method
if (tmp.send) {
	pass("XML::send() exists");
} else {
	fail("XML::send() doesn't exist");
}
// test the XML::sendandload method
if (tmp.sendAndLoad) {
	pass("XML::sendAndLoad() exists");
} else {
	fail("XML::sendAndLoad() doesn't exist");
}
// test the XML::tostring method
if (tmp.toString) {
	pass("XML::toString() exists");
} else {
	fail("XML::toString() doesn't exist");
}

// Load
if (tmp.load("testin.xml")) {
	pass("XML::load() works");
} else {
	fail("XML::load() doesn't work");
}
//
if (tmp.hasChildNodes() == true) {
	pass("XML::hasChildNodes() works");
} else {
	fail("XML::hasChildNodes() doesn't work");
}

if (tmp.getBytesLoaded() > 1) {
	pass("XML::getBytesLoaded() works");
} else {
	fail("XML::getBytesLoaded() doesn't work");
}

if (tmp.getBytesTotal() > 1) {
	pass("XML::getBytesTotal() works");
} else {
	fail("XML::getBytesTotal() doesn't work");
}

if (tmp.getBytesLoaded() == tmp.getBytesTotal()) {
	pass("bytes count are the same");
} else {
	fail("bytes counts are not the same");
}

myXML = new XML();
var before = myXML.hasChildNodes();
//trace(before);

getCourseElement = myXML.createElement("module");
if (getCourseElement.nodename == "module") {
    pass("XML::createElementNode() works");
} else {
    fail("XML::createElementNode() doesn't work");
}

textElement = myXML.createTextNode("Hello World");
if (textElement.nodevalue == "Hello World") {
    pass("XML::createTextNode() works");
} else {
    fail("XML::createTextNode() doesn't work");
}

getCourseElement.appendChild(textElement);
nodename = getCourseElement.nodeName;
//trace(nodename);
nodevalue = getCourseElement.nodeValue;
//trace(nodevalue);
if ((nodename == "module") && (nodevalue == "")) {
	pass("XML::createTextNode() works");
} else {
	fail("XML::createTextNode() doesn't work");
}

nodename = getCourseElement.nodeName;
myXML.appendChild(getCourseElement);
var after = myXML.hasChildNodes();

//trace(after);

if ((before == false) && (after == true) && (nodename == "module")) {
	pass("XML::appendChild() works");
} else {
	fail("XML::appendChild() doesn't work");
}

// trace(myXML.toString());

newnode = myXML.cloneNode(false);

trace(myXML.nodeName);
trace(newnode.nodeValue);


// for (var i=0; i < valueArray.length; i++) {
//     getitemElement = myXML.createElement("activity");
//     getCourseElement.appendChild(getitemElement);
    
//     getnameElement = myXML.createElement("name");
//     getitemElement.appendChild(getnameElement);
//     if (typeof(itemArray) == "object") {
//         textElement = myXML.createTextNode(itemArray[i].questions);
//     } else {
//         textElement = myXML.createTextNode(itemArray[i]);
//     }
//     getnameElement.appendChild(textElement);
    
//     getvalueElement = myXML.createElement("value");
//     getitemElement.appendChild(getvalueElement);
    
//     textElement = myXML.createTextNode(valueArray[i]);
//     getvalueElement.appendChild(textElement);
// }

totals();
