// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
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

// Test case for XML ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

rcsid="$Id: XML.as,v 1.23 2007/04/04 14:33:32 strk Exp $";

#include "dejagnu.as"
#include "utils.as"

var existtests = true;

check(XML);

#if OUTPUT_VERSION >= 6
check(! XML.prototype.hasOwnProperty("appendChild") );
check(! XML.prototype.hasOwnProperty("cloneNode") );
check(! XML.prototype.hasOwnProperty("hasChildNodes") );
check(! XML.prototype.hasOwnProperty("insertBefore") );
check(! XML.prototype.hasOwnProperty("removeNode") );
check(! XML.prototype.hasOwnProperty("cloneNode") );
check(! XML.prototype.hasOwnProperty("toString") );
check(! XML.prototype.hasOwnProperty("length") );
check(! XML.prototype.hasOwnProperty("status"));
check(! XML.prototype.hasOwnProperty("loaded"));
check(XML.prototype.hasOwnProperty("createElement") );
check(XML.prototype.hasOwnProperty("addRequestHeader") );
check(XML.prototype.hasOwnProperty("createTextNode") );
check(XML.prototype.hasOwnProperty("getBytesLoaded") );
check(XML.prototype.hasOwnProperty("getBytesTotal") );
check(XML.prototype.hasOwnProperty("load") );
check(XML.prototype.hasOwnProperty("parseXML") );
check(XML.prototype.hasOwnProperty("send") );
check(XML.prototype.hasOwnProperty("sendAndLoad") );

check(!XML.hasOwnProperty("createElement") );
check(!XML.hasOwnProperty("addRequestHeader") );
check(!XML.hasOwnProperty("createTextNode") );
check(!XML.hasOwnProperty("getBytesLoaded") );
check(!XML.hasOwnProperty("getBytesTotal") );
check(!XML.hasOwnProperty("load") );
check(!XML.hasOwnProperty("parseXML") );
check(!XML.hasOwnProperty("send") );
check(!XML.hasOwnProperty("sendAndLoad") );

check(XMLNode.prototype.hasOwnProperty("appendChild") );
check(XMLNode.prototype.hasOwnProperty("cloneNode") );
check(XMLNode.prototype.hasOwnProperty("hasChildNodes") );
check(XMLNode.prototype.hasOwnProperty("insertBefore") );
check(XMLNode.prototype.hasOwnProperty("removeNode") );
check(XMLNode.prototype.hasOwnProperty("toString") );
check(XMLNode.prototype.hasOwnProperty("cloneNode") );
check(! XMLNode.prototype.hasOwnProperty("length") );
check(! XMLNode.prototype.hasOwnProperty("createElement") );
check(! XMLNode.prototype.hasOwnProperty("addRequestHeader") );
check(! XMLNode.prototype.hasOwnProperty("createTextNode") );
check(! XMLNode.prototype.hasOwnProperty("getBytesLoaded") );
check(! XMLNode.prototype.hasOwnProperty("getBytesTotal") );
check(! XMLNode.prototype.hasOwnProperty("load") );
check(! XMLNode.prototype.hasOwnProperty("parseXML") );
check(! XMLNode.prototype.hasOwnProperty("send") );
check(! XMLNode.prototype.hasOwnProperty("sendAndLoad") );
check(! XMLNode.prototype.hasOwnProperty("status"));
check(! XMLNode.prototype.hasOwnProperty("loaded"));

check(! XMLNode.hasOwnProperty("appendChild") );
check(! XMLNode.hasOwnProperty("cloneNode") );
check(! XMLNode.hasOwnProperty("hasChildNodes") );
check(! XMLNode.hasOwnProperty("insertBefore") );
check(! XMLNode.hasOwnProperty("removeNode") );
check(! XMLNode.hasOwnProperty("toString") );
check(! XMLNode.hasOwnProperty("cloneNode") );
#endif

var tmp = new XML();
check_equals(typeof(tmp.length), 'undefined');
check(! tmp.hasOwnProperty("length"));

check_equals(typeof(tmp.status), 'number');
check(! tmp.hasOwnProperty("status"));

check_equals(typeof(tmp.loaded), 'undefined');
check(! tmp.hasOwnProperty("loaded"));

tmp.loaded = 5;
check_equals(typeof(tmp.loaded), 'boolean');
check(tmp.loaded);
tmp.loaded = 0;
check_equals(typeof(tmp.loaded), 'boolean');
check(!tmp.loaded);
check(! tmp.hasOwnProperty("loaded"));


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
tmp.checkParsed = function ()
{
	note("tmp.checkParsed called");
	check(this.hasChildNodes());
	check(this.childNodes instanceof Array);
	check_equals(this.firstChild, this.lastChild);
	check_equals(typeof(this.childNodes.length), 'number');
	check_equals(this.childNodes.length, 1);
	check_equals(this.childNodes[0], this.firstChild);
	check_equals(this.childNodes[0], this.lastChild);
#if OUTPUT_VERSION > 5
	check(this.childNodes.hasOwnProperty('length'));
	check(this.childNodes[0] === this.firstChild);
	check(this.childNodes[0] === this.lastChild);
#endif

    // childNodes is a read-only property !
    this.childNodes = 5;
	check(this.childNodes instanceof Array);

        with (this.firstChild) {
            //trace("FIXME: firstChild found: " + nodeName);
            if (nodeName == 'TOPNODE') {
                //trace("FIXME: topnode found: "+ childNodes.length);
                childa = 0;
                while (childa < childNodes.length) {
                    //trace("FIXME: children found");
                    check(childNodes[childa] != undefined);
                    with (childNodes[childa]) {
                        if (nodeName == 'SUBNODE1') {
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
};

check_equals( typeof(tmp.parseXML), 'function');
check(tmp.childNodes instanceOf Array);
check_equals(tmp.childNodes.length, 0);

// parseXML doesn't return anything
ret = tmp.parseXML(xml_in);
check_equals(typeof(ret), 'undefined');

tmp.checkParsed(); // onLoad won't be called
//note("Parsed XML: "+tmp.toString());
check_equals(tmp.toString(), xml_in);
check(XML.prototype instanceof XMLNode);
check(tmp instanceof XML);
check(tmp instanceof XMLNode);
check(tmp.firstChild instanceof XMLNode);
check_equals(typeof(tmp.nodeName), 'null');
check_equals(typeof(tmp.nodeValue), 'null');
check_equals(typeof(tmp.length), 'undefined');
check_equals(tmp.firstChild.nodeName, "TOPNODE");
check_equals(typeof(tmp.firstChild.length), 'undefined');

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
check_equals(tmp.getBytesLoaded(), undefined);
check_equals(tmp.getBytesTotal(), undefined);

if (tmp.getBytesLoaded() == tmp.getBytesTotal()) {
    pass("bytes count are the same");
} else {
    fail("bytes counts are not the same");
}


check(XML);
myXML = new XML();
check(myXML != undefined);
check(myXML.createElement);

//    file.puts("function: dodo()");
// create three XML nodes using createElement()
var element1 = myXML.createElement("element1");
check(element1.nodeName == "element1");

var element2 = myXML.createElement("element2");
check_equals(element2.nodeName, "element2");

var element3 = myXML.createElement("element3");
check_equals(element3.nodeName, "element3");

check(myXML.createTextNode);

// create two XML text nodes using createTextNode()
var textNode1 = myXML.createTextNode("textNode1 String value");
check(textNode1.nodeValue == "textNode1 String value");

var textNode2 = myXML.createTextNode("textNode2 String value");
check(textNode2.nodeValue == "textNode2 String value");

// place the new nodes into the XML tree
check(!element2.hasChildNodes());
ret = element2.appendChild(textNode1);
check_equals(typeof(ret), 'undefined');
check(element2.hasChildNodes());

check_equals(element2.nodeValue, null);
check_equals(typeof(element2.lastChild), 'object');
check_equals(element2.lastChild.nodeValue, "textNode1 String value");
element2.lastChild = 4;
check_equals(typeof(element2.lastChild), 'object');

element3.appendChild(textNode2);
check_equals(element3.nodeValue, null); 
check_equals(typeof(element3.lastChild), 'object');
check_equals(element3.lastChild.nodeValue, "textNode2 String value");

// place the new nodes into the XML tree
doc.appendChild(element1);
//check(doc.firstChild.nodeName == "element1");

element1.appendChild(element2);
check(element1.hasChildNodes());
// trace(element1.nodeName);
// trace(element1.firstChild.nodeName);
check(element1.firstChild.nodeName == "element2");

element2.appendChild(element3);
check(element2.hasChildNodes());

trace(doc.toString());

// // trace(myXML.toString());

// newnode = myXML.cloneNode(false);

// //trace(myXML.nodeName);
// //trace(newnode.nodeValue);

// //trace("Child1" + _global.child1);
// //trace("Child2" + _global.child2);

// // This won't work as onLoad is not called unless you
// // actually *load* the XML, we're using parseXML that
// // does *not* trigger loading (see also getBytesLoaded
// // and getBytesTotal) and does *not* trigger onLoad 
// // event to execute.
// #if 0 
// if ((_global.child1 == "sub sub1 node data 1")
//     && (global.child2 == "sub sub1 node data 2")) {
// 	pass("XML::onLoad works");
// } else {
// 	fail("XML::onLoad doesn't work");
// }
// #endif


//--------------------------------------------------------------------
// Test loading an XML locally
//--------------------------------------------------------------------

myxml = new XML;
myxml.onLoad = function(success)
{
	note("myxml.onLoad("+success+") called");

	check_equals(typeof(myxml.status), 'number');
	check_equals(typeof(myxml.loaded), 'boolean');
#if OUTPUT_VERSION >= 6
	check(! myxml.hasOwnProperty('status'));
	check(! myxml.hasOwnProperty('loaded'));
#endif // OUTPUT_VERSION >= 6

	if ( ! success )
	{
		note("No success loading gnash.xml");
		check_equals(myxml.status, 0);
		check(! myxml.loaded);
		return;
	}
	note("gnash.xml successfully loaded");
	note("myxml status is "+myxml.status);

	check_equals(myxml.status, 0);
	check(myxml.loaded);

	// Check 'loaded' and 'status' to be "overridable"

	var loaded_backup = myxml.loaded;
	myxml.loaded = 'a string';
	check_equals(typeof(myxml.loaded), 'boolean');
	myxml.loaded = ! loaded_backup;
	check(myxml.loaded != loaded_backup);
	myxml.loaded = loaded_backup;

	var status_backup = myxml.status;
	myxml.status = 'a string';
	check_equals(typeof(myxml.status), 'number');
	xcheck(myxml.status != status_backup);
	myxml.status = status_backup;


	//note("myxml.toString(): "+myxml.toString());
	check_equals(typeof(myxml.attributes), 'object');
	xcheck(! myxml.attributes instanceof Object);
	xcheck_equals(typeof(myxml.attributes.__proto__), 'undefined');

	check(myxml.hasChildNodes());
	check_equals(myxml.nodeName, null);

	topnode = myxml.firstChild;
	check_equals(topnode.nodeName, 'XML');
	check_equals(topnode.attributes.attr1, 'attr1 value');
};
check_equals(typeof(myxml.status), 'number');
#if OUTPUT_VERSION < 7
check_equals(typeof(myxml.STATUS), 'number');
#else // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.STATUS), 'undefined');
#endif // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.__proto__.status), 'undefined');
check_equals(typeof(myxml.loaded), 'undefined');
#if OUTPUT_VERSION >= 6
check(!myxml.hasOwnProperty('status'));
check(!myxml.hasOwnProperty('loaded'));
#endif // OUTPUT_VERSION >= 6
ret = myxml.load( MEDIA(gnash.xml) );

check_equals(typeof(myxml.loaded), 'boolean');
#if OUTPUT_VERSION < 7
check_equals(typeof(myxml.LOADED), 'boolean');
#else // OUTPUT_VERSION >= 7
check_equals(typeof(myxml.LOADED), 'undefined');
#endif // OUTPUT_VERSION >= 7
xcheck(! myxml.loaded ); // is really loaded in a background thread

xcheck_equals(myxml.loaded, false ); // is really loaded in a background thread
note("myxml.loaded = "+myxml.loaded);
note("myxml.load() returned "+ret);

// We're done
//totals();
