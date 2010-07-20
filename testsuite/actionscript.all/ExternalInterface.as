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

flash.system.Security.allowDomain("localhost");

ASSetPropFlags (_global, "flash", 0, 5248);

#if OUTPUT_VERSION < 6

if (flash.external.ExternalInterface == undefined) {
    pass("ExternalInterface class doesn't exist in older versions");
} else {
    fail("ExternalInterface class doesn't exist in older versions");
}

#endif

EI = flash.external.ExternalInterface;

#if OUTPUT_VERSION > 6

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
#endif

#if OUTPUT_VERSION > 7
if (EI.hasOwnProperty("available")) {
    pass("ExternalInterface::available() exists");
} else {
    fail("ExternalInterface::available() doesn't exist");
}

// Create a test function for the callback
function TestASMethod (msg) {
    note("TestASMethod called! " + msg);
    return "I am here!";
}

// ExternalInterface::available is always false when run standalone,
// and true if running under a browser and has allowable access to the
// resource. So we can't have a test case for this property that works
// both standalone and online Instead we do depend on it being correct
// as the ExternalInterface::call() tests require this test case to
// be run under a browser.

// ::call() calls JavaScript functions in the browser, not in flash,
// so we can't test it when running standalone. So this will always
// return null unless run from a browser

// This adds a callback that the brower can call from Javascript. This is
// a bit of a bogus test case, as addCallback() doesn't return anything, but
// we need to add a method anyway to test being called by Javascript.
if (EI.addCallback("TestASMethod", null, TestASMethod) == false) {
    pass("ExternalInterface::addCallback(\"TestASMethod\")");
} else {
    fail("ExternalInterface::addCallback(\"TestASMethod\")");
}

if (EI.available == true) {
  // This test tries to invoke a Javascript method running in the browser
  if (EI.call("TestJSMethod", "test") == null) {
    pass("ExternalInterface::call(\"TestJSMethod\") == null");
  } else {
    fail("ExternalInterface::call(\"TestJSMethod\") == null");
  }
}

// The default is false, so if we can set it to true, it worked
EI.marshallExceptions = true;
if (EI.marshallExceptions == true) {
    pass("ExternalInterface::marshallExceptions()");
} else if (EI.objectID == undefined) {
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
if (typeof(r) == "object") {
    pass("ExternalInterface::ExternalInterface()");
} else {
    fail("ExternalInterface::ExternalInterface()");
}

// check(r instanceOf EI);
if (r instanceOf EI) {
    pass("ExternalInterface instanceOf");
} else {
    fail("ExternalInterface instanceOf");
}

// But it doesn't do much.
if (r._toXML(o) == undefined) {
    pass("ExternalInterface undefined");
} else {
    fail("ExternalInterface undefined");
}

xml = EI._objectToXML(nc);
if (xml == '<object></object>') {
    pass("ExternalInterface::_objectToXML(native class)");
} else {
    fail("ExternalInterface::_objectToXML(native class)");
}

xml = EI._objectToXML(o);
if (xml == '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>') {
    xpass("ExternalInterface::_objectToXML(object)");
} else {
    xfail("ExternalInterface::_objectToXML(object)");
}

xml = EI._objectToXML(undefined);
if (xml == "<object></object>") {
    pass("ExternalInterface::_objectToXML(undefined)");
} else {
    fail("ExternalInterface::_objectToXML(undefined)");
}

xml = EI._objectToXML(null);
if (xml == "<object></object>") {
    pass("ExternalInterface::_objectToXML(null)");
} else {
    fail("ExternalInterface::_objectToXML(null)");
}

xml = EI._objectToXML(6);
if (xml == "<object></object>") {
    pass("ExternalInterface::_objectToXML(number)");
} else {
    fail("ExternalInterface::_objectToXML(number)");
}

xml = EI._arrayToXML(a);
if (xml == '<array><property id="0"><number>12</number></property><property id="1"><number>34</number></property><property id="2"><string>tr</string></property><property id="3"><number>1</number></property><property id="4"><number>2</number></property><property id="5"><number>3</number></property><property id="6"><number>4</number></property></array>') {
    pass("ExternalInterface::_arrayToXML(array)");
} else {
    fail("ExternalInterface::_arrayToXML(array)");
}

xml = EI._argumentsToXML(a, 0);
if (xml == '<arguments><number>34</number><string>tr</string><number>1</number><number>2</number><number>3</number><number>4</number></arguments>') {
    pass("ExternalInterface::_argumentsToXML()");
} else {
    fail("ExternalInterface::_argumentsToXML()");
}

// xml = EI._toXML(o);
// if (xml == '<object><property id="a"><number>1</number></property><property id="b"><string>string</string></property></object>') {
//     pass("ExternalInterface::_toXML(object)");
// } else {
//     fail("ExternalInterface::_toXML(object)");
// }

// xml = EI._toXML("Hello World!");
// if (xml == "<string>Hello World!</string>") {
//     pass("ExternalInterface::_toXML(string)");
// } else {
//     fail("ExternalInterface::_toXML(string)");
// }

xml = EI._toXML(123.456);
if (xml == "<number>123.456</number>") {
    pass("ExternalInterface::_toXML(number)");
} else {
    fail("ExternalInterface::_toXML(number)");
}

xml = EI._objectToXML(no);
if (xml == '<object><property id="namespaceURI"><null/></property><property id="localName"><null/></property><property id="prefix"><null/></property><property id="previousSibling"><null/></property><property id="parentNode"><null/></property><property id="nodeValue"><null/></property><property id="nodeType"><number>1</number></property><property id="nodeName"><null/></property><property id="nextSibling"><null/></property><property id="lastChild"><null/></property><property id="firstChild"><null/></property><property id="childNodes"><array></array></property><property id="attributes"><null/></property><property id="getPrefixForNamespace"><null/></property><property id="getNamespaceForPrefix"><null/></property><property id="toString"><null/></property><property id="hasChildNodes"><null/></property><property id="appendChild"><null/></property><property id="insertBefore"><null/></property><property id="removeNode"><null/></property><property id="cloneNode"><null/></property><property id="xmlDecl"><undefined/></property><property id="status"><number>0</number></property><property id="loaded"><undefined/></property><property id="ignoreWhite"><false/></property><property id="docTypeDecl"><undefined/></property><property id="contentType"><string>application/x-www-form-urlencoded</string></property><property id="addRequestHeader"><null/></property><property id="getBytesTotal"><null/></property><property id="getBytesLoaded"><null/></property><property id="onData"><null/></property><property id="onLoad"><null/></property><property id="sendAndLoad"><null/></property><property id="send"><null/></property><property id="load"><null/></property><property id="parseXML"><null/></property><property id="createTextNode"><null/></property><property id="createElement"><null/></property></object>') {
    xpass("ExternalInterface::_objectToXML(native object)");
} else {
    xfail("ExternalInterface::_objectToXML(native object)");
}

// xcheck_equals(EI._objectToJS(o), '({a:1,b:"string"})');

// xcheck_equals(EI._objectToAS(no).toString(), '[object Object]');

// check_equals(EI._objectID(o), null);

// escape / unescape
rin = "& ß+ü < << <>''\"";
rout = "&amp; ß+ü &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";

if (EI._escapeXML(rin) == rout) {
    pass("ExternalInterface::_escapeXML()");
} else {
    fail("ExternalInterface::_escapeXML()");
}

// It doesn't escape html entities.
rin = "&amp; ß+ü &nbsp; &lt; &lt;&lt; &lt;&gt;&apos;&apos;&quot;";
rout = "& ß+ü .. < << <>''\"";
ret  = EI._unescapeXML(rin);
// This test will until fail until we can figure out the best way to
// match the converted strings. Testing in GDB show the result is correct,
// so this is mainly a test case problem.
if (ret == "& ß+ü &nbsp; < << <>''\"") {
    xpass("ExternalInterface::_unescapeXML()");
} else {
    xfail("ExternalInterface::_unescapeXML()");
}

val = EI._toAS("<number>34.56</number>");
if (val == 34.56) {
    pass("ExternalInterface::_toAS(number)");
} else {
    fail("ExternalInterface::_toAS(number)");
}

val = EI._toAS("<string>Hello World!</string>");
if (val == "Hello World!") {
    pass("ExternalInterface::_toAS(string)");
} else {
    fail("ExternalInterface::_toAS(string)");
}

val = EI._toAS("<null/>");
if (val == null) {
    pass("ExternalInterface::_toAS(null)");
} else {
    fail("ExternalInterface::_toAS(null)");
}

val = EI._toAS("<true/>");
if (val == true) {
    pass("ExternalInterface::_toAS(true)");
} else {
    fail("ExternalInterface::_toAS(true)");
}

val = EI._toAS("<false/>");
if (val == false) {
    pass("ExternalInterface::_toAS(false)");
} else {
    fail("ExternalInterface::_toAS(false)");
}

val = EI._objectToAS('<object><property id="b"><string>string</string></property><property id="a"><number>1</number></property></object>');
if (typeOf(val) == "object") {
    xpass("ExternalInterface::_objectToAS(object)");
} else {
    xfail("ExternalInterface::_objectToAS(object)");
}

#endif  // version > 7

totals();

