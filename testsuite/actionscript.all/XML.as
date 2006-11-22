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

rcsid="$Id: XML.as,v 1.11 2006/11/22 14:38:32 strk Exp $";

#include "dejagnu.as"
#include "utils.as"

var existtests = true;

check(XML);
var tmp = new XML();

// test the XML constuctor
if (tmp) {
    pass("XML::XML() constructor");
} else {
    fail("XML::XML()");		
}

check(XML);

if (existtests) {
    
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
// This doesn't seem to exist in the real player
// test the XML::loaded method
    if (tmp.loaded) {
	unresolved("XML::loaded() exists, it shouldn't!");
    } else {
	unresolved("XML::loaded() doesn't exist yet");
    }
    
//test the XML::parse method
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
    
} // end of existtests
/////////////////////////////////////////////////////

check(XML);
newXML = new XML();
check(XML);

// Load
// if (tmp.load("testin.xml")) {
// 	pass("XML::load() works");
// } else {
// 	fail("XML::load() doesn't work");
// }
check(XML);
var xml_in = "<TOPNODE><SUBNODE1><SUBSUBNODE1>sub sub1 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub1 node data 2</SUBSUBNODE2></SUBNODE1><SUBNODE2><SUBSUBNODE1>sub sub2 node data 1</SUBSUBNODE1><SUBSUBNODE2>sub sub2 node data 2</SUBSUBNODE2></SUBNODE2></TOPNODE>";
check(XML);

check(XML);
tmp.onLoad = function (success) {
    if (success) {
        with (tmp.firstChild) {
            trace("FIXME: firstChild found: " + nodeName);
            if (nodeName == 'TOPNODE') {
                trace("FIXME: topnode found: "+ childNodes.length);
                childa = 0;
                while (childa < childNodes.length) {
                    trace("FIXME: children found");
                    check(childNodes[childa] != undefined);
                    with (childNodes[childa]) {
                        if (nodeName == 'SUBNODE1') {
                            trace("FIXME: subnode1 found");
                            childb = 0;
                            while (childb < childNodes.length) {
                                with (childNodes[childb]) {
                                    if (nodeName == 'SUBSUBNODE1') {
                                        _global.child1 = firstChild.nodeValue;
note("Set _global.child1 to "+_global.child1);
                                    } else {
                                        if (nodeName == 'SUBNODE2') {
                                            _global.child2 = firstChild.nodeValue;
                                        } else {
                                            if (nodeName == 'SUBSUBNODE1') {
                                                _global.child3 = firstChild.nodeValue;
                                            }
                                        }
                                    }
                                }
                                ++childb;
                            }
                        }
                    }
                    ++childa;
                }
            }
        }
    }
};
check(XML);

// parseXML doesn't return anything
tmp.parseXML(xml_in);

if (tmp.firstChild.nodeName == "TOPNODE") {
    pass("XML::parseXML() works");
} else {
    fail("XML::parseXML() doesn't work");
}

if (tmp.hasChildNodes() == true) {
    pass("XML::hasChildNodes() works");
} else {
    fail("XML::hasChildNodes() doesn't work");
}
// note(tmp.getBytesLoaded());
// note(tmp.getBytesTotal());

// Since we didn't *load* the XML, but we
// just *parsed* it, expect getBytesLoaded 
// and getBytesTotal to return undefined
xcheck_equals(tmp.getBytesLoaded(), undefined);
xcheck_equals(tmp.getBytesTotal(), undefined);

if (tmp.getBytesLoaded() == tmp.getBytesTotal()) {
    pass("bytes count are the same");
} else {
    fail("bytes counts are not the same");
}

check(XML);

myXML = new XML();
check(myXML != undefined);

var before = myXML.hasChildNodes();
//trace(before);

check(myXML.createElement);

getElement = myXML.createElement("module");

check(getElement);
trace(typeof(getElement));
check(getElement.nodeName != undefined );
#if OUTPUT_VERSION > 6
// we're case-sensitive in version 7 !!
xcheck_equals(getElement.nodename, undefined );
#else
check(getElement.nodename != undefined );
#endif

check_equals(getElement.nodeName, "module");
if (getElement.nodeName == "module") {
    pass("XML::createElementNode() works");
} else {
    fail("XML::createElementNode() doesn't work");
}

textElement = myXML.createTextNode("Hello World");
if (textElement.nodeValue == "Hello World") {
    pass("XML::createTextNode() works");
} else {
    fail("XML::createTextNode() doesn't work");
}

xcheck_equals(textElement.nodeType, 3);
check_equals(textElement.nodeValue, "Hello World");
xcheck_equals(textElement.nodeName, null);

//note(textElement);

check(getElement);
check(getElement.appendChild);
check(textElement);

xcheck_equals(getElement.hasChildNodes(), false);

getElement.appendChild(textElement);
if ( getElement.hasChildNodes() ) {
    pass("Appending Text Node to Element Node works");
} else {
    xfail("Appending Text Node to Element Node doesn't work");
}

nodename = getElement.nodeName;
myXML.appendChild(getElement);
var after = myXML.hasChildNodes();

//trace(after);

if ((before == false) && (after == true) && (nodename == "module")) {
	pass("XML::appendChild() works");
} else {
	fail("XML::appendChild() doesn't work");
}

// trace(myXML.toString());

newnode = myXML.cloneNode(false);

//trace(myXML.nodeName);
//trace(newnode.nodeValue);

//trace("Child1" + _global.child1);
//trace("Child2" + _global.child2);

// This won't work as onLoad is not called unless you
// actually *load* the XML, we're using parseXML that
// does *not* trigger loading (see also getBytesLoaded
// and getBytesTotal) and does *not* trigger onLoad 
// event to execute.
#if 0 
if ((_global.child1 == "sub sub1 node data 1")
    && (global.child2 == "sub sub1 node data 2")) {
	pass("XML::onLoad works");
} else {
	fail("XML::onLoad doesn't work");
}
#endif

// We're done
totals();
