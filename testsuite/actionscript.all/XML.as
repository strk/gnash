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

// test the XML constuctor
if (tmp) {
	trace("PASSED: XML::XML() constructor");
} else {
	trace("FAILED: XML::XML()");		
}

// test the XML::addrequestheader method
if (tmp.addRequestHeader) {
	trace("PASSED: XML::addRequestHeader() exists");
} else {
	trace("FAILED: XML::addRequestHeader() doesn't exist");
}
// test the XML::appendchild method
if (tmp.appendChild) {
	trace("PASSED: XML::appendChild() exists");
} else {
	trace("FAILED: XML::appendChild() doesn't exist");
}
// test the XML::clonenode method
if (tmp.cloneNode) {
	trace("PASSED: XML::cloneNode() exists");
} else {
	trace("FAILED: XML::cloneNode() doesn't exist");
}
// test the XML::createelement method
if (tmp.createElement) {
	trace("PASSED: XML::createElement() exists");
} else {
	trace("FAILED: XML::createElement() doesn't exist");
}
// test the XML::createtextnode method
if (tmp.createTextNode) {
	trace("PASSED: XML::createTextNode() exists");
} else {
	trace("FAILED: XML::createTextNode() doesn't exist");
}
// test the XML::getbytesloaded method
if (tmp.getBytesLoaded) {
	trace("PASSED: XML::getBytesLoaded() exists");
} else {
	trace("FAILED: XML::getBytesLoaded() doesn't exist");
}
// test the XML::getbytestotal method
if (tmp.getBytesTotal) {
	trace("PASSED: XML::getBytesTotal() exists");
} else {
	trace("FAILED: XML::getBytesTotal() doesn't exist");
}
// test the XML::haschildnodes method
if (tmp.hasChildNodes) {
	trace("PASSED: XML::hasChildNodes() exists");
} else {
	trace("FAILED: XML::hasChildNodes() doesn't exist");
}
// test the XML::insertbefore method
if (tmp.insertBefore) {
	trace("PASSED: XML::insertBefore() exists");
} else {
	trace("FAILED: XML::insertBefore() doesn't exist");
}
// test the XML::load method
if (tmp.load) {
	trace("PASSED: XML::load() exists");
} else {
	trace("FAILED: XML::load() doesn't exist");
}
// test the XML::loaded method
if (tmp.loaded) {
	trace("PASSED: XML::loaded() exists");
} else {
	trace("FAILED: XML::loaded() doesn't exist");
}
// test the XML::parse method
if (tmp.parseXML) {
	trace("PASSED: XML::parseXML() exists");
} else {
	trace("FAILED: XML::parseXML() doesn't exist");
}
// test the XML::removenode method
if (tmp.removeNode) {
	trace("PASSED: XML::removeNode() exists");
} else {
	trace("FAILED: XML::removeNode() doesn't exist");
}
// test the XML::send method
if (tmp.send) {
	trace("PASSED: XML::send() exists");
} else {
	trace("FAILED: XML::send() doesn't exist");
}
// test the XML::sendandload method
if (tmp.sendAndLoad) {
	trace("PASSED: XML::sendAndLoad() exists");
} else {
	trace("FAILED: XML::sendAndLoad() doesn't exist");
}
// test the XML::tostring method
if (tmp.toString) {
	trace("PASSED: XML::toString() exists");
} else {
	trace("FAILED: XML::toString() doesn't exist");
}

// Load
if (tmp.load("testin.xml")) {
	trace("PASSED: XML::load() works");
} else {
	trace("FAILED: XML::load() doesn't works");
}
//
if (tmp.hasChildNodes() == true) {
	trace("PASSED: XML::hasChildNodes() works");
} else {
	trace("FAILED: XML::hasChildNodes() works");
}

if (tmp.getBytesLoaded() > 1) {
	trace("PASSED: XML::getBytesLoaded() works");
} else {
	trace("FAILED: XML::getBytesLoaded() works");
}

if (tmp.getBytesTotal() > 1) {
	trace("PASSED: XML::getBytesTotal() works");
} else {
	trace("FAILED: XML::getBytesTotal() works");
}

if (tmp.getBytesLoaded() == tmp.getBytesTotal()) {
	trace("PASSED: bytes count are the same");
} else {
	trace("FAILED: bytes counts are not the same");
}
