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
if (tmp.addrequestheader) {
	trace("PASSED: XML::addrequestheader() exists");
} else {
	trace("FAILED: XML::addrequestheader() doesn't exist");
}
// test the XML::appendchild method
if (tmp.appendchild) {
	trace("PASSED: XML::appendchild() exists");
} else {
	trace("FAILED: XML::appendchild() doesn't exist");
}
// test the XML::clonenode method
if (tmp.clonenode) {
	trace("PASSED: XML::clonenode() exists");
} else {
	trace("FAILED: XML::clonenode() doesn't exist");
}
// test the XML::createelement method
if (tmp.createelement) {
	trace("PASSED: XML::createelement() exists");
} else {
	trace("FAILED: XML::createelement() doesn't exist");
}
// test the XML::createtextnode method
if (tmp.createtextnode) {
	trace("PASSED: XML::createtextnode() exists");
} else {
	trace("FAILED: XML::createtextnode() doesn't exist");
}
// test the XML::getbytesloaded method
if (tmp.getbytesloaded) {
	trace("PASSED: XML::getbytesloaded() exists");
} else {
	trace("FAILED: XML::getbytesloaded() doesn't exist");
}
// test the XML::getbytestotal method
if (tmp.getbytestotal) {
	trace("PASSED: XML::getbytestotal() exists");
} else {
	trace("FAILED: XML::getbytestotal() doesn't exist");
}
// test the XML::haschildnodes method
if (tmp.haschildnodes) {
	trace("PASSED: XML::haschildnodes() exists");
} else {
	trace("FAILED: XML::haschildnodes() doesn't exist");
}
// test the XML::insertbefore method
if (tmp.insertbefore) {
	trace("PASSED: XML::insertbefore() exists");
} else {
	trace("FAILED: XML::insertbefore() doesn't exist");
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
if (tmp.removenode) {
	trace("PASSED: XML::removenode() exists");
} else {
	trace("FAILED: XML::removenode() doesn't exist");
}
// test the XML::send method
if (tmp.send) {
	trace("PASSED: XML::send() exists");
} else {
	trace("FAILED: XML::send() doesn't exist");
}
// test the XML::sendandload method
if (tmp.sendandload) {
	trace("PASSED: XML::sendandload() exists");
} else {
	trace("FAILED: XML::sendandload() doesn't exist");
}
// test the XML::tostring method
if (tmp.tostring) {
	trace("PASSED: XML::tostring() exists");
} else {
	trace("FAILED: XML::tostring() doesn't exist");
}
