// 
//   Copyright (C) 2008, 2009, 2010 Free Software Foundation, Inc.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
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
// Test case for ExternalInterface ActionScript class
// compile this test case with Ming makeswf, and then
// execute it like this gnash -1 -r 0 -v out.swf

#include "dejagnu.as"

ASSetPropFlags (_global, "flash", 0, 5248);

#if OUTPUT_VERSION < 6

if (flash.external.ExternalInterface == undefined) {
    pass("ExternalInterface class doesn't exist in older versions");
} else {
    fail("ExternalInterface class doesn't exist in older versions");
}

#endif

EI = flash.external.ExternalInterface;

// First make sure all the documented methods and properties exist
if (EI.hasOwnProperty("call")) {
    pass("ExternalInterface::call() exists");
} else {
    fail("ExternalInterface::call() doesn't exist");
}

if (EI.hasOwnProperty("addCallback")) {
    pass("ExternalInterface::addCallback() exists");
} else {
    fail("ExternalInterface::addCallback() doesn't exist");
}

if (EI.hasOwnProperty("available")) {
    pass("ExternalInterface::available() exists");
} else {
    fail("ExternalInterface::available() doesn't exist");
}

// this should always be true now that Gnash supports this class
if (EI.available == true) {
    pass("ExternalInterface::available is correct");
} else {
    fail("ExternalInterface::available property isn't correct");
}

// Create a test function for the callback
function TestEIMethod () {
    note("TestEIMethod called!");
}

if (ExternalInterface.addCallback("TestEIMethod", null, TestEIMethod)) {
    pass("ExternalInterface::addCallback(\"TestEIMethod\")");
} else {
    fail("ExternalInterface::addCallback(\"TestEIMethod\")");
}

if (ExternalInterface.call("TestEIMethod", null)) {
    pass("ExternalInterface::call(\"TestEIMethod\")");
} else {
    fail("ExternalInterface::call(\"TestEIMethod\")");
}

// The marshallExceptions and objectID are new
#if OUTPUT_VERSION > 7
if (EI.hasOwnProperty("marshallExceptions")) {
    pass("ExternalInterface::marshallExceptions() exists");
} else {
    fail("ExternalInterface::marshallExceptions() doesn't exist");
}

// The default is false, so if we can set it to true, it worked
EI.marshallExceptions = true;
if (EI.marshallExceptions == true) {
    pass("ExternalInterface::marshallExceptions()");
} else {
    fail("ExternalInterface::marshallExceptions()");
}

if (EI.hasOwnProperty("objectID")) {
    pass("ExternalInterface::objectID() exists");
} else {
    fail("ExternalInterface::objectID() doesn't exist");
}
if (EI.objectID == undefined) {
    pass("ExternalInterface::objectID() is undefined");
} else {
    fail("ExternalInterface::objectID() is undefined");
}

trace(EI.objectID);

// this should always be true now that Gnash supports this class
if (EI.objectID == true) {
    pass("ExternalInterface::objectID is correct");
} else {
    fail("ExternalInterface::objectID property isn't correct");
}

// Then make sure all the undocumented methods and properties exist
if (EI.hasOwnProperty("_argumentsToXML")) {
    pass("ExternalInterface::_argumentsToXML() exists");
} else {
    fail("ExternalInterface::_argumentsToXML() doesn't exist");
}

if (EI.hasOwnProperty("_argumentsToAS")) {
    pass("ExternalInterface::_argumentsToAS() exists");
} else {
    fail("ExternalInterface::_argumentsToAS() doesn't exist");
}

if (EI.hasOwnProperty("_unescapeXML")) {
    pass("ExternalInterface::_unescapeXML() exists");
} else {
    fail("ExternalInterface::_unescapeXML() doesn't exist");
}

if (EI.hasOwnProperty("_toXML")) {
    pass("ExternalInterface::_toXML() exists");
} else {
    fail("ExternalInterface::_toXML() doesn't exist");
}

if (EI.hasOwnProperty("_toJS")) {
    pass("ExternalInterface::_toJS() exists");
} else {
    fail("ExternalInterface::_toJS() doesn't exist");
}


if (EI.hasOwnProperty("_toAS")) {
    pass("ExternalInterface::_toAS() exists");
} else {
    fail("ExternalInterface::_toAS() doesn't exist");
}

if (EI.hasOwnProperty("_objectToXML")) {
    pass("ExternalInterface::_objectToXML() exists");
} else {
    fail("ExternalInterface::_objectToXML() doesn't exist");
}

if (EI.hasOwnProperty("_objectToJS")) {
    pass("ExternalInterface::_objectToJS() exists");
} else {
    fail("ExternalInterface::_objectToJS() doesn't exist");
}

if (EI.hasOwnProperty("_objectToAS")) {
    pass("ExternalInterface::_objectToAS() exists");
} else {
    fail("ExternalInterface::_objectToAS() doesn't exist");
}

if (EI.hasOwnProperty("_objectID")) {
    pass("ExternalInterface::_objectID() exists");
} else {
    fail("ExternalInterface::_objectID() doesn't exist");
}

if (EI.hasOwnProperty("_jsQuoteString")) {
    pass("ExternalInterface::_jsQuoteString() exists");
} else {
    fail("ExternalInterface::_jsQuoteString() doesn't exist");
}

if (EI.hasOwnProperty("_initJS")) {
    pass("ExternalInterface::_initJS() exists");
} else {
    fail("ExternalInterface::_initJS() doesn't exist");
}

if (EI.hasOwnProperty("_evalJS")) {
    pass("ExternalInterface::_evalJS() exists");
} else {
    fail("ExternalInterface::_evalJS() doesn't exist");
}

if (EI.hasOwnProperty("_escapeXML")) {
    pass("ExternalInterface::_escapeXML() exists");
} else {
    fail("ExternalInterface::_escapeXML() doesn't exist");
}

if (EI.hasOwnProperty("_callOut")) {
    pass("ExternalInterface::_callOut() exists");
} else {
    fail("ExternalInterface::_callOut() doesn't exist");
}

if (EI.hasOwnProperty("_callIn")) {
    pass("ExternalInterface::_callIn() exists");
} else {
    fail("ExternalInterface::_callIn() doesn't exist");
}

if (EI.hasOwnProperty("_arrayToXML")) {
    pass("ExternalInterface::_arrayToXML() exists");
} else {
    fail("ExternalInterface::_arrayToXML() doesn't exist");
}

if (EI.hasOwnProperty("_arrayToJS")) {
    pass("ExternalInterface::_arrayToJS() exists");
} else {
    fail("ExternalInterface::_arrayToJS() doesn't exist");
}

if (EI.hasOwnProperty("_arrayToAS")) {
    pass("ExternalInterface::_arrayToAS() exists");
} else {
    fail("ExternalInterface::_arrayToAS() doesn't exist");
}

// An object
o = { a: 1, b: "string" };

// An object with circular references
oc = {};
oc.a = {};
oc.a.a = oc.b;
oc.b = oc.a;

// A native class
nc = Mouse;

// A native object
no = new XML;

// An Array
a = [ 12, 34, "tr", 1, 2, 3, 4 ];

// Try instantiating.
r = new EI;
// You get an object
check_equals(typeof(r), "object");
check(r instanceOf EI);
// But it doesn't do much.
check_equals(r._toXML(o), undefined);

// All methods are static.

// _objectTo*
xcheck_equals(EI._objectToXML(o), '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');
xcheck_equals(EI._objectToXML(nc), '<object></object>');
xcheck_equals(EI._objectToXML(no), '<object><property id="namespaceURI"><null/></property><property id="localName"><null/></property><property id="prefix"><null/></property><property id="previousSibling"><null/></property><property id="parentNode"><null/></property><property id="nodeValue"><null/></property><property id="nodeType"><number>1</number></property><property id="nodeName"><null/></property><property id="nextSibling"><null/></property><property id="lastChild"><null/></property><property id="firstChild"><null/></property><property id="childNodes"><array></array></property><property id="attributes"><null/></property><property id="getPrefixForNamespace"><null/></property><property id="getNamespaceForPrefix"><null/></property><property id="toString"><null/></property><property id="hasChildNodes"><null/></property><property id="appendChild"><null/></property><property id="insertBefore"><null/></property><property id="removeNode"><null/></property><property id="cloneNode"><null/></property><property id="xmlDecl"><undefined/></property><property id="status"><number>0</number></property><property id="loaded"><undefined/></property><property id="ignoreWhite"><false/></property><property id="docTypeDecl"><undefined/></property><property id="contentType"><string>application/x-www-form-urlencoded</string></property><property id="addRequestHeader"><null/></property><property id="getBytesTotal"><null/></property><property id="getBytesLoaded"><null/></property><property id="onData"><null/></property><property id="onLoad"><null/></property><property id="sendAndLoad"><null/></property><property id="send"><null/></property><property id="load"><null/></property><property id="parseXML"><null/></property><property id="createTextNode"><null/></property><property id="createElement"><null/></property></object>');
xcheck_equals(EI._objectToXML(undefined), '<object></object>');
xcheck_equals(EI._objectToXML(6), '<object></object>');
xcheck_equals(EI._objectToXML(oc), '<object><property id="b"><object><property id="a"><undefined/></property></object></property><property id="a"><object><property id="a"><undefined/></property></object></property></object>');

xcheck_equals(EI._objectToJS(o), '({a:1,b:"string"})');

xcheck_equals(EI._objectToAS(no).toString(), '[object Object]');

check_equals(EI._objectID(o), null);

// _arrayTo*
xcheck_equals(EI._arrayToXML(a), '<array><property id="0"><number>12</number></property><property id="1"><number>34</number></property><property id="2"><string>tr</string></property><property id="3"><number>1</number></property><property id="4"><number>2</number></property><property id="5"><number>3</number></property><property id="6"><number>4</number></property></array>');
xcheck_equals(EI._arrayToJS(a), '[12,34,"tr",1,2,3,4]');

// _to*

xcheck_equals(EI._toXML("string"), '<string>string</string>');
xcheck_equals(EI._toXML(4), '<number>4</number>');

xcheck_equals(EI._toXML(o), '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>');

// escape / unescape
r = "& ß+ü < << <>''\"";
xcheck_equals(EI._escapeXML(r), "&amp; ß+ü &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;");

// It doesn't escape html entities.
r = "&amp; ß+ü &nbsp; &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";
xcheck_equals(EI._unescapeXML(r), "& ß+ü &nbsp; < << <>''\"");

#endif  // version > 7

totals();

